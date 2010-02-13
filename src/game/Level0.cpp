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
    uint32 save_interval = sWorld.getConfig(CONFIG_INTERVAL_SAVE);
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
        if ((itr->second->isGameMaster() || (itr_sec > SEC_PLAYER && itr_sec <= (AccountTypes)sWorld.getConfig(CONFIG_GM_LEVEL_IN_GM_LIST))) &&
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
