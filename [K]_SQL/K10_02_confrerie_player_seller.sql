CREATE TABLE `player_seller` (
	`pguid` int(11) NOT NULL,
	`vendeur_armure` int(1) default '0',
	`vendeur_arme` int(1) default '0',
	`vendeur_bijoux` int(1) default '0',
	`vendeur_autre` int(1) default '0',
	`level_item_max` int(4) NOT NULL default '0',
	`qualitee_item_max` int(1) NOT NULL default '0',
	`item_requierd` int(8) default '0',
	`guild_vendor` int(1) default '0'
	) ENGINE=MyISAM DEFAULT CHARSET=utf8;
