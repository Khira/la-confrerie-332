/*
 * Copyright (C) 2005-2010 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "Common.h"
#include "Database/DatabaseEnv.h"
#include "World.h"
#include "Player.h"
#include "Opcodes.h"
#include "Chat.h"
#include "ObjectAccessor.h"
#include "Language.h"
#include "AccountMgr.h"
#include "SystemConfig.h"
#include "revision.h"
#include "revision_nr.h"
#include "Util.h"
#include "SpellAuras.h"
#include "SpellMgr.h"
#include "ObjectMgr.h"

bool ChatHandler::HandleHelpCommand(const char* args)
{
    char* cmd = strtok((char*)args, " ");
    if(!cmd)
    {
        ShowHelpForCommand(getCommandTable(), "help");
        ShowHelpForCommand(getCommandTable(), "");
    }
    else
    {
        if(!ShowHelpForCommand(getCommandTable(), cmd))
            SendSysMessage(LANG_NO_HELP_CMD);
    }

    return true;
}

bool ChatHandler::HandleCommandsCommand(const char* /*args*/)
{
    ShowHelpForCommand(getCommandTable(), "");
    return true;
}

bool ChatHandler::HandleAccountCommand(const char* /*args*/)
{
    AccountTypes gmlevel = m_session->GetSecurity();
    PSendSysMessage(LANG_ACCOUNT_LEVEL, uint32(gmlevel));
    return true;
}

bool ChatHandler::HandleStartCommand(const char* /*args*/)
{
    Player *chr = m_session->GetPlayer();

    if(chr->isInFlight())
    {
        SendSysMessage(LANG_YOU_IN_FLIGHT);
        SetSentErrorMessage(true);
        return false;
    }

    if(chr->isInCombat())
    {
        SendSysMessage(LANG_YOU_IN_COMBAT);
        SetSentErrorMessage(true);
        return false;
    }

    // cast spell Stuck
    chr->CastSpell(chr,7355,false);
    return true;
}

bool ChatHandler::HandleServerInfoCommand(const char* /*args*/)
{
    uint32 activeClientsNum = sWorld.GetActiveSessionCount();
    uint32 queuedClientsNum = sWorld.GetQueuedSessionCount();
    uint32 maxActiveClientsNum = sWorld.GetMaxActiveSessionCount();
    uint32 maxQueuedClientsNum = sWorld.GetMaxQueuedSessionCount();
    std::string str = secsToTimeString(sWorld.GetUptime());

    char const* full;
    if(m_session)
        full = _FULLVERSION(REVISION_DATE,REVISION_TIME,REVISION_NR,"|cffffffff|Hurl:" REVISION_ID "|h" REVISION_ID "|h|r");
    else
        full = _FULLVERSION(REVISION_DATE,REVISION_TIME,REVISION_NR,REVISION_ID);

    SendSysMessage(full);
    PSendSysMessage(LANG_USING_SCRIPT_LIB,sWorld.GetScriptsVersion());
    PSendSysMessage(LANG_USING_WORLD_DB,sWorld.GetDBVersion());
    PSendSysMessage(LANG_USING_EVENT_AI,sWorld.GetCreatureEventAIVersion());
    PSendSysMessage(LANG_CONNECTED_USERS, activeClientsNum, maxActiveClientsNum, queuedClientsNum, maxQueuedClientsNum);
    PSendSysMessage(LANG_UPTIME, str.c_str());

    return true;
}

bool ChatHandler::HandleDismountCommand(const char* /*args*/)
{
    //If player is not mounted, so go out :)
    if (!m_session->GetPlayer( )->IsMounted())
    {
        SendSysMessage(LANG_CHAR_NON_MOUNTED);
        SetSentErrorMessage(true);
        return false;
    }

    if(m_session->GetPlayer( )->isInFlight())
    {
        SendSysMessage(LANG_YOU_IN_FLIGHT);
        SetSentErrorMessage(true);
        return false;
    }

    m_session->GetPlayer()->Unmount();
    m_session->GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_MOUNTED);
    return true;
}

bool ChatHandler::HandleSaveCommand(const char* /*args*/)
{
    Player *player=m_session->GetPlayer();

    // save GM account without delay and output message (testing, etc)
    if(m_session->GetSecurity() > SEC_PLAYER)
    {
        player->SaveToDB();
        SendSysMessage(LANG_PLAYER_SAVED);
        return true;
    }

    // save or plan save after 20 sec (logout delay) if current next save time more this value and _not_ output any messages to prevent cheat planning
    uint32 save_interval = sWorld.getConfig(CONFIG_UINT32_INTERVAL_SAVE);
    if (save_interval==0 || (save_interval > 20*IN_MILISECONDS && player->GetSaveTimer() <= save_interval - 20*IN_MILISECONDS))
        player->SaveToDB();

    return true;
}

bool ChatHandler::HandleGMListIngameCommand(const char* /*args*/)
{
    bool first = true;

    HashMapHolder<Player>::MapType &m = HashMapHolder<Player>::GetContainer();
    HashMapHolder<Player>::MapType::const_iterator itr = m.begin();
    for(; itr != m.end(); ++itr)
    {
        AccountTypes itr_sec = itr->second->GetSession()->GetSecurity();
        if ((itr->second->isGameMaster() || (itr_sec > SEC_PLAYER && itr_sec <= (AccountTypes)sWorld.getConfig(CONFIG_UINT32_GM_LEVEL_IN_GM_LIST))) &&
            (!m_session || itr->second->IsVisibleGloballyFor(m_session->GetPlayer())))
        {
            if(first)
            {
                SendSysMessage(LANG_GMS_ON_SRV);
                first = false;
            }

            SendSysMessage(GetNameLink(itr->second).c_str());
        }
    }

    if(first)
        SendSysMessage(LANG_GMS_NOT_LOGGED);

    return true;
}

bool ChatHandler::HandleAccountPasswordCommand(const char* args)
{
    if(!*args)
        return false;

    char *old_pass = strtok ((char*)args, " ");
    char *new_pass = strtok (NULL, " ");
    char *new_pass_c  = strtok (NULL, " ");

    if (!old_pass || !new_pass || !new_pass_c)
        return false;

    std::string password_old = old_pass;
    std::string password_new = new_pass;
    std::string password_new_c = new_pass_c;

    if (password_new != password_new_c)
    {
        SendSysMessage (LANG_NEW_PASSWORDS_NOT_MATCH);
        SetSentErrorMessage (true);
        return false;
    }

    if (!sAccountMgr.CheckPassword (m_session->GetAccountId(), password_old))
    {
        SendSysMessage (LANG_COMMAND_WRONGOLDPASSWORD);
        SetSentErrorMessage (true);
        return false;
    }

    AccountOpResult result = sAccountMgr.ChangePassword(m_session->GetAccountId(), password_new);

    switch(result)
    {
        case AOR_OK:
            SendSysMessage(LANG_COMMAND_PASSWORD);
            break;
        case AOR_PASS_TOO_LONG:
            SendSysMessage(LANG_PASSWORD_TOO_LONG);
            SetSentErrorMessage(true);
            return false;
        case AOR_NAME_NOT_EXIST:                            // not possible case, don't want get account name for output
        default:
            SendSysMessage(LANG_COMMAND_NOTCHANGEPASSWORD);
            SetSentErrorMessage(true);
            return false;
    }

    return true;
}

bool ChatHandler::HandleAccountLockCommand(const char* args)
{
    if (!*args)
    {
        SendSysMessage(LANG_USE_BOL);
        return true;
    }

    std::string argstr = (char*)args;
    if (argstr == "on")
    {
        loginDatabase.PExecute( "UPDATE account SET locked = '1' WHERE id = '%d'",m_session->GetAccountId());
        PSendSysMessage(LANG_COMMAND_ACCLOCKLOCKED);
        return true;
    }

    if (argstr == "off")
    {
        loginDatabase.PExecute( "UPDATE account SET locked = '0' WHERE id = '%d'",m_session->GetAccountId());
        PSendSysMessage(LANG_COMMAND_ACCLOCKUNLOCKED);
        return true;
    }

    SendSysMessage(LANG_USE_BOL);
    return true;
}

/// Display the 'Message of the day' for the realm
bool ChatHandler::HandleServerMotdCommand(const char* /*args*/)
{
    PSendSysMessage(LANG_MOTD_CURRENT, sWorld.GetMotd());
    return true;
}

bool ChatHandler::HandleMorphCommand(const char* args)
{
	if(!m_session->GetPlayer()->isAlive())
	{
		SetSentErrorMessage(true);
		return false;
	}

	if( !*args )
	{
		SendSysMessage(LANG_MORPH_RACE_HELP);
		SetSentErrorMessage(true);
		return false;
	}
	char* argument1_cache = strtok((char*)args, " ");
	std::string argument1 = argument1_cache;
	Player* character = m_session->GetPlayer();

	if (argument1 == "on")
	{
		QueryResult *result = ConfrerieDatabase.PQuery("SELECT displayid_m, displayid_w, scale, speed, aura1, aura2, aura3, spell1, spell2, spell3 FROM player_race WHERE entry=(SELECT morph FROM player_race_relation WHERE guid='%u')", character->GetGUID());
		if (!result) 
		{
			SendSysMessage(LANG_MORPH_RACE_NOT_POSSIBLE);
			SetSentErrorMessage(true);
			return false; 
		}
		Field* fields = result->Fetch();
		float Scale = fields[2].GetFloat();
		float Speed = fields[3].GetFloat();
		uint32 auraID1 = fields[4].GetUInt32();
		SpellEntry const *spellInfo1 = sSpellStore.LookupEntry( auraID1 );
		if(spellInfo1)
		{
			for(uint32 i = 0;i<3;++i)
			{
				uint8 eff1 = spellInfo1->Effect[i];
				if (eff1>=TOTAL_SPELL_EFFECTS)
					continue;
				if( IsAreaAuraEffect(eff1)           ||
				eff1 == SPELL_EFFECT_APPLY_AURA  ||
				eff1 == SPELL_EFFECT_PERSISTENT_AREA_AURA )
				{
					Aura *Aur1 = CreateAura(spellInfo1, i, NULL, character);
					character->AddAura(Aur1);
				}
			}
		}
		uint32 auraID2 = fields[5].GetUInt32();
		SpellEntry const *spellInfo2 = sSpellStore.LookupEntry( auraID2 );
		if(spellInfo2)
		{
			for(uint32 i = 0;i<3;++i)
			{
				uint8 eff2 = spellInfo2->Effect[i];
				if (eff2>=TOTAL_SPELL_EFFECTS)
					continue;
				if( IsAreaAuraEffect(eff2)           ||
				eff2 == SPELL_EFFECT_APPLY_AURA  ||
				eff2 == SPELL_EFFECT_PERSISTENT_AREA_AURA )
				{
					Aura *Aur2 = CreateAura(spellInfo2, i, NULL, character);
					character->AddAura(Aur2);
				}
			}
		}
		uint32 auraID3 = fields[6].GetUInt32();
		SpellEntry const *spellInfo3 = sSpellStore.LookupEntry( auraID3 );
		if(spellInfo3)
		{
			for(uint32 i = 0;i<3;++i)
			{
				uint8 eff3 = spellInfo3->Effect[i];
				if (eff3>=TOTAL_SPELL_EFFECTS)
					continue;
				if( IsAreaAuraEffect(eff3)           ||
				eff3 == SPELL_EFFECT_APPLY_AURA  ||
				eff3 == SPELL_EFFECT_PERSISTENT_AREA_AURA )
				{
					Aura *Aur3 = CreateAura(spellInfo3, i, NULL, character);
					character->AddAura(Aur3);
				}
			}
		}
		if (!(character->HasSpell(fields[7].GetUInt32())) && fields[7].GetUInt32()!=0)
			character->learnSpell(fields[7].GetUInt32(),false);

		if (!(character->HasSpell(fields[8].GetUInt32())) && fields[8].GetUInt32()!=0)
			character->learnSpell(fields[8].GetUInt32(),false);

		if (!(character->HasSpell(fields[9].GetUInt32())) && fields[9].GetUInt32()!=0)
			character->learnSpell(fields[9].GetUInt32(),false); 

		uint16 displayid;
		if (character->getGender() == GENDER_MALE)
		{
			displayid = fields[0].GetUInt32();
		}
		else
		{
			displayid = fields[1].GetUInt32();
		}
		character->SetSpeed(MOVE_RUN,Speed,true);
		character->SetSpeed(MOVE_FLIGHT,Speed,true);
		character->SetFloatValue(OBJECT_FIELD_SCALE_X, Scale);
		character->SetDisplayId(displayid);
		delete result;
		return true;
	}

	if (argument1 == "off")
	{
		QueryResult *resultRace = ConfrerieDatabase.PQuery("SELECT aura1, aura2, aura3, spell1, spell2, spell3 FROM player_race WHERE entry=(SELECT morph FROM player_race_relation WHERE guid='%u')", character->GetGUID());
		if (resultRace)
		{
			Field* fieldsRace = resultRace->Fetch();

			if (character->HasSpell(fieldsRace[3].GetUInt32()) && fieldsRace[3].GetUInt32()!=0)
				character->removeSpell(fieldsRace[3].GetUInt32(),false,false);

			if (character->HasSpell(fieldsRace[4].GetUInt32()) && fieldsRace[3].GetUInt32()!=0)
				character->removeSpell(fieldsRace[4].GetUInt32(),false,false);

			if (character->HasSpell(fieldsRace[5].GetUInt32()) && fieldsRace[3].GetUInt32()!=0)
				character->removeSpell(fieldsRace[5].GetUInt32(),false,false);

			if(fieldsRace[0].GetUInt32())
				character->RemoveAurasDueToSpell(fieldsRace[0].GetUInt32());

			if(fieldsRace[1].GetUInt32())
				character->RemoveAurasDueToSpell(fieldsRace[1].GetUInt32());

			if(fieldsRace[2].GetUInt32())
				character->RemoveAurasDueToSpell(fieldsRace[2].GetUInt32());
		}
		delete resultRace;

		character->DeMorph();
		character->SetSpeed(MOVE_RUN,1,true);
		character->SetSpeed(MOVE_FLIGHT,1,true);
		character->SetFloatValue(OBJECT_FIELD_SCALE_X, 1);
		m_session->SendNotification(LANG_MORPH_RACE_HUMAN);
		return true;
	}

	if (argument1 == "set" && m_session->GetSecurity()>0)
	{
		Player *target = getSelectedPlayer();
		if (!target)
		{
			target = m_session->GetPlayer();
			if (!target)
			{
				return false;
			}
		}
		char* argument2 = strtok((char*)NULL, " ");
		PSendSysMessage(LANG_MORPH_RACE_SET, target->GetName());
		ConfrerieDatabase.PExecute("DELETE FROM player_race_relation WHERE guid = '%u'", target->GetGUID());
		ConfrerieDatabase.PExecute("INSERT INTO player_race_relation (morph, guid) VALUES ('%s', '%u')", argument2, target->GetGUID());
		return true;
	}

	SendSysMessage(LANG_MORPH_RACE_HELP);
	SetSentErrorMessage(true);
	return false;
}

bool ChatHandler::HandleItemnumberCommand(const char* args)
{
	if (!*args)
        return false;
    uint32 itemId = 0;
    if(args[0]=='[')
    {
        char* citemName = strtok((char*)args, "]");

        if(citemName && citemName[0])
        {
            std::string itemName = citemName+1;
            WorldDatabase.escape_string(itemName);
            QueryResult *result1 = WorldDatabase.PQuery("SELECT entry FROM item_template WHERE name = '%s'", itemName.c_str());
            if (!result1)
            {
                PSendSysMessage(LANG_COMMAND_COULDNOTFIND, citemName+1);
                SetSentErrorMessage(true);
                return false;
            }
            itemId = result1->Fetch()->GetUInt16();
            delete result1;
        }
        else
            return false;
    }
    else
    {
        char* cId = extractKeyFromLink((char*)args,"Hitem");
        if(!cId)
            return false;
        itemId = atol(cId);
    }
	Player* target = getSelectedPlayer();
	if(!target)
		target = m_session->GetPlayer();
	// target->SaveInventoryAndGoldToDB();
	QueryResult *result = CharacterDatabase.PQuery("SELECT CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(`data`, ' ', 15), ' ', -1) AS UNSIGNED) FROM `item_instance` WHERE guid IN (SELECT item FROM character_inventory WHERE item_template='%u' AND guid='%u')",itemId,target->GetGUID());
	int ItemNumber=0;
	if(!result)
    {
		PSendSysMessage(LANG_ITEM_NUMBER_ERROR, target->GetName(), itemId);
		return true;
    }
	else
	{
		do
		{
			Field *fields = result->Fetch();
			ItemNumber += fields[0].GetInt32();
		}while( result->NextRow() );
		PSendSysMessage(LANG_ITEM_NUMBER_DISP, target->GetName(), ItemNumber, itemId);
		delete result;
		return true;
	}
}

bool ChatHandler::HandleSellerAddItemCommand(const char* args)
{
	if(!m_session->GetPlayer()->isAlive())
	{
		SetSentErrorMessage(true);
		return false;
	}

    if (!*args)
        return false;

    uint32 itemId = 0;

    if(args[0]=='[')
    {
        char* citemName = strtok((char*)args, "]");

        if(citemName && citemName[0])
        {
            std::string itemName = citemName+1;
            WorldDatabase.escape_string(itemName);
            QueryResult *result = WorldDatabase.PQuery("SELECT entry FROM item_template WHERE name = '%s'", itemName.c_str());
            if (!result)
            {
                PSendSysMessage(LANG_COMMAND_COULDNOTFIND, citemName+1);
                SetSentErrorMessage(true);
                return false;
            }
            itemId = result->Fetch()->GetUInt16();
            delete result;
        }
        else
            return false;
    }
    else                                                    // item_id or [name] Shift-click form |color|Hitem:item_id:0:0:0|h[name]|h|r
    {
        char* cId = extractKeyFromLink((char*)args,"Hitem");
        if(!cId)
            return false;
        itemId = atol(cId);
    }
    int32 count = 1;
    Player* pl = m_session->GetPlayer();
    Player* plTarget = getSelectedPlayer();
    if(!plTarget)
        plTarget = pl;
		
	// Selection des permissions du vendeur
	QueryResult *resultSeller = ConfrerieDatabase.PQuery("SELECT vendeur_armure, vendeur_arme, vendeur_bijoux, vendeur_autre, level_item_max, qualitee_item_max, item_requierd, guild_vendor FROM player_seller WHERE pguid='%u'", pl->GetGUID());
		if (!resultSeller) {
			SendSysMessage(LANG_COMMAND_SELLER_ADD_NOSELLER);
			SetSentErrorMessage(true);
			return false; }
		Field* fieldsSeller = resultSeller->Fetch();
		uint32 vendeur_armure = fieldsSeller[0].GetUInt32();
		uint32 vendeur_arme = fieldsSeller[1].GetUInt32();
		uint32 vendeur_bijoux = fieldsSeller[2].GetUInt32();
		uint32 vendeur_autre = fieldsSeller[3].GetUInt32();
		uint32 level_item_max = fieldsSeller[4].GetUInt32();
		uint32 qualitee_item_max = fieldsSeller[5].GetUInt32();
		uint32 idItemRequierd = fieldsSeller[6].GetUInt32();
		uint32 guildAllowToSell = fieldsSeller[7].GetUInt32();
		delete resultSeller;

    ItemPrototype const *pProto = sObjectMgr.GetItemPrototype(itemId);
    if(!pProto)
    {
        PSendSysMessage(LANG_COMMAND_ITEMIDINVALID, itemId);
        SetSentErrorMessage(true);
        return false;
    }
	uint32 money = pl->GetMoney();

	if (pProto->Quality > qualitee_item_max)
	{
		PSendSysMessage(LANG_COMMAND_SELLER_ADD_NOQUALITY);
		SetSentErrorMessage(true);
		return false;
	}
	if (pProto->ItemLevel > level_item_max)
	{
		PSendSysMessage(LANG_COMMAND_SELLER_ADD_NOLEVEL);
		SetSentErrorMessage(true);
		return false;
	}
	// Selection du nombre de 'jetons'
	QueryResult *resultJeton = CharacterDatabase.PQuery("SELECT CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(`data`, ' ', 15), ' ', -1) AS UNSIGNED) FROM `item_instance` WHERE guid IN (SELECT item FROM character_inventory WHERE item_template='%u' AND guid='%u')", idItemRequierd, pl->GetGUID());
	int ItemNumber=0;
	if(!resultJeton)
    {
		pl->SaveToDB();
		PSendSysMessage(LANG_COMMAND_SELLER_ADD_NOJETON);
		SetSentErrorMessage(true);
		return false;
    }
	delete resultJeton;
	
	uint32 itemPrix = 0;

	// Si vendeur d'armures : tete(1), epaules(3), torse(5),  ceinture(6),  jambes(7), pieds(8),  chemise(4),  mains(10), dos(16), robe(20)
	if (pProto->InventoryType == 1 || pProto->InventoryType == 3 || pProto->InventoryType == 5 || pProto->InventoryType == 6 || pProto->InventoryType == 4 || pProto->InventoryType == 10 || pProto->InventoryType == 16 || pProto->InventoryType == 7 || pProto->InventoryType == 20 || pProto->InventoryType == 8) {
		if (vendeur_armure == 1) {
			itemPrix = 10000 * pProto->Quality * pProto->ItemLevel / 25; }
		else {
			PSendSysMessage(LANG_COMMAND_SELLER_ADD_DENIED, itemId);
			SetSentErrorMessage(true);
			return false; }
	}
	// Si vendeur d'amres : armes(13), bouclier(14),  deux-mains(17), une main(21)
	else if (pProto->InventoryType == 13 || pProto->InventoryType == 14 || pProto->InventoryType == 17 || pProto->InventoryType == 21) {
		if (vendeur_arme == 1) {
			itemPrix = 10000 * pProto->Quality * pProto->ItemLevel / 25; }
		else {
			PSendSysMessage(LANG_COMMAND_SELLER_ADD_DENIED, itemId);
			SetSentErrorMessage(true);
			return false; }
	}
	//Si vendeur de bijoux : cou(2), bracelets(9), bagues(11), bijoux(12),  main gauche(22), tome(23),  relique(28)
	else if (pProto->InventoryType == 2 || pProto->InventoryType == 9 || pProto->InventoryType == 11 || pProto->InventoryType == 12 ||
			 pProto->InventoryType == 22 || pProto->InventoryType == 23 || pProto->InventoryType == 28)  {
		if (vendeur_bijoux == 1) {
			itemPrix = 10000 * pProto->Quality * pProto->ItemLevel / 50; }
		else {
			PSendSysMessage(LANG_COMMAND_SELLER_ADD_DENIED, itemId);
			SetSentErrorMessage(true);
			return false; }
	}
	// Si vendeur autre : Montures (C15, SC5), familiers (C15, SC2)
	else if ((pProto->Class == 15 && pProto->SubClass == 5) || (pProto->Class == 15 && pProto->SubClass == 2)) {
		if (vendeur_autre == 1) {
			itemPrix = 10000 * pProto->Quality * pProto->ItemLevel; }
		else {
			PSendSysMessage(LANG_COMMAND_SELLER_ADD_DENIED, itemId);
			SetSentErrorMessage(true);
			return false; }
	}
	else {
		PSendSysMessage(LANG_COMMAND_SELLER_ADD_DENIED, itemId);
		SetSentErrorMessage(true);
		return false;
	}

	if (plTarget == pl)
	{
        int loc_idx = GetSessionDbLocaleIndex();
        if ( loc_idx >= 0 )
        {
            ItemLocale const *il = sObjectMgr.GetItemLocale(pProto->ItemId);
            if (il)
            {
                if (il->Name.size() > loc_idx && !il->Name[loc_idx].empty())
                {
                    std::string name = il->Name[loc_idx];
					uint32 gold = itemPrix /GOLD;
					uint32 silv = (itemPrix % GOLD) / SILVER;
					uint32 copp = (itemPrix % GOLD) % SILVER;
                    PSendSysMessage(LANG_COMMAND_SELLER_ADD_PRICE, itemId, itemId, name.c_str(), gold, silv, copp);
					// %d - |cffffffff|Hitem:%d:0:0:0:0:0:0:0:0|h[%s]|h|r - Prix : %uPO %uPA %uPC.
					SetSentErrorMessage(true);
                }
            }
        }
		return false;
	}

	if(!pl->GetGuildId()) { }
	else
	{
		if(pl->GetGuildId()==plTarget->GetGuildId() && guildAllowToSell==0) // Non autorise a vendre a sa guilde.
		{
			PSendSysMessage(LANG_COMMAND_SELLER_ADD_NOGUILD);
			SetSentErrorMessage(true);
			return false;
		}
	}
		
	if (money < itemPrix) {
		PSendSysMessage(LANG_COMMAND_SELLER_ADD_NOMONEY, itemPrix);
		SetSentErrorMessage(true);
		return false;
	}
		

    //Adding items
    uint32 noSpaceForCount = 0;

    // check space and find places
    ItemPosCountVec dest;
    uint8 msg = plTarget->CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, itemId, count, &noSpaceForCount );
    if( msg != EQUIP_ERR_OK )                               // convert to possible store amount
        count -= noSpaceForCount;

    if( count == 0 || dest.empty())                         // can't add any
    {
        PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itemId, noSpaceForCount );
        SetSentErrorMessage(true);
        return false;
    }

    Item* item = plTarget->StoreNewItem( dest, itemId, true, Item::GenerateItemRandomPropertyId(itemId));

    // remove binding (let GM give it to another player later)
    if(pl==plTarget)
        for(ItemPosCountVec::const_iterator itr = dest.begin(); itr != dest.end(); ++itr)
            if(Item* item1 = pl->GetItemByPos(itr->pos))
                item1->SetBinding( false );

    if(count > 0 && item)
    {
		pl->DestroyItemCount(idItemRequierd, 1, true, false);
        int32 newmoney = int32(money) - int32(itemPrix);
        pl->SetMoney( newmoney );
        pl->SendNewItem(item,count,false,true);
        if(pl!=plTarget)
            plTarget->SendNewItem(item,count,true,false);
    }

    if(noSpaceForCount > 0)
        PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itemId, noSpaceForCount);

	pl->SaveToDB();
    return true;
}