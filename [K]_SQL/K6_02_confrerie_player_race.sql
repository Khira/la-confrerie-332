CREATE TABLE `player_race` (
	`entry` int(11) default '0',
	`displayid_m` int(11) default '0',
	`displayid_w` int(11) default '0',
	`scale` float default '1',
	`speed` float default '1',
	`aura1` int(11) default '0',
	`aura2` int(11) default '0',
	`aura3` int(11) default '0',
	`comments` varchar(255) default NULL
	) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- Exemples de races
INSERT INTO `player_race` (`entry`, `displayid_m`, `displayid_w`, `scale`, `speed`, `aura1`, `aura2`, `aura3`, `comments`) VALUES ('1','25835','25835','1','2','34873','0','0','Drake - Vol Rouge');
INSERT INTO `player_race` (`entry`, `displayid_m`, `displayid_w`, `scale`, `speed`, `aura1`, `aura2`, `aura3`, `comments`) VALUES ('2','27398','27912','1','1','0','0','0','Gobelin - Mage');
INSERT INTO `player_race` (`entry`, `displayid_m`, `displayid_w`, `scale`, `speed`, `aura1`, `aura2`, `aura3`, `comments`) VALUES ('3','7108','10744','1','1','0','0','0','Gobelin - Voleur');
INSERT INTO `player_race` (`entry`, `displayid_m`, `displayid_w`, `scale`, `speed`, `aura1`, `aura2`, `aura3`, `comments`) VALUES ('4','21289','22393','1','1','0','0','0','Gobelin - Guerrier');
INSERT INTO `player_race` (`entry`, `displayid_m`, `displayid_w`, `scale`, `speed`, `aura1`, `aura2`, `aura3`, `comments`) VALUES ('5','6374','6374','1','2','34873','0','0','Drake - Vol Noir');
INSERT INTO `player_race` (`entry`, `displayid_m`, `displayid_w`, `scale`, `speed`, `aura1`, `aura2`, `aura3`, `comments`) VALUES ('6','22289','25393','1','1','0','0','0','Vrykul - Mage');
INSERT INTO `player_race` (`entry`, `displayid_m`, `displayid_w`, `scale`, `speed`, `aura1`, `aura2`, `aura3`, `comments`) VALUES ('7','21917','25829','1','1','0','0','0','Vrykul simple');
INSERT INTO `player_race` (`entry`, `displayid_m`, `displayid_w`, `scale`, `speed`, `aura1`, `aura2`, `aura3`, `comments`) VALUES ('8','22059','25658','1','1','0','0','0','Vrykul - Guerrier');
INSERT INTO `player_race` (`entry`, `displayid_m`, `displayid_w`, `scale`, `speed`, `aura1`, `aura2`, `aura3`, `comments`) VALUES ('9','27814','27814','1','3','34873','0','0','Drake - Vol Infini');
INSERT INTO `player_race` (`entry`, `displayid_m`, `displayid_w`, `scale`, `speed`, `aura1`, `aura2`, `aura3`, `comments`) VALUES ('10','25852','25852','1','2','34873','0','0','Drake - Vol de Bronze');
INSERT INTO `player_race` (`entry`, `displayid_m`, `displayid_w`, `scale`, `speed`, `aura1`, `aura2`, `aura3`, `comments`) VALUES ('11','25832','25832','1','2','34873','0','0','Drake - Vol Bleu');
INSERT INTO `player_race` (`entry`, `displayid_m`, `displayid_w`, `scale`, `speed`, `aura1`, `aura2`, `aura3`, `comments`) VALUES ('13','25533','25533','1','1','0','0','0','Rohart - Pêcheur');
INSERT INTO `player_race` (`entry`, `displayid_m`, `displayid_w`, `scale`, `speed`, `aura1`, `aura2`, `aura3`, `comments`) VALUES ('14','7993','9132','1','1','0','0','0','Gobelin - Marchand');
INSERT INTO `player_race` (`entry`, `displayid_m`, `displayid_w`, `scale`, `speed`, `aura1`, `aura2`, `aura3`, `comments`) VALUES ('16','25835','25835','2.5','2.3','34873','37825','0','Dragon - Vol Rouge');
INSERT INTO `player_race` (`entry`, `displayid_m`, `displayid_w`, `scale`, `speed`, `aura1`, `aura2`, `aura3`, `comments`) VALUES ('17','26477','26477','1','1','0','0','0','Gnome robot');
INSERT INTO `player_race` (`entry`, `displayid_m`, `displayid_w`, `scale`, `speed`, `aura1`, `aura2`, `aura3`, `comments`) VALUES ('18','11570','11570','0.9','1','0','0','0','Ogre - normal');
INSERT INTO `player_race` (`entry`, `displayid_m`, `displayid_w`, `scale`, `speed`, `aura1`, `aura2`, `aura3`, `comments`) VALUES ('18','23398','23398','1','1','0','0','0','Dryade');
INSERT INTO `player_race` (`entry`, `displayid_m`, `displayid_w`, `scale`, `speed`, `aura1`, `aura2`, `aura3`, `comments`) VALUES ('19','21927','25812','1','1','0','0','0','Valkyr - chasseur');
INSERT INTO `player_race` (`entry`, `displayid_m`, `displayid_w`, `scale`, `speed`, `aura1`, `aura2`, `aura3`, `comments`) VALUES ('20','657','657','1.5','1','0','0','0','Worgen noir classique');
INSERT INTO `player_race` (`entry`, `displayid_m`, `displayid_w`, `scale`, `speed`, `aura1`, `aura2`, `aura3`, `comments`) VALUES ('21','18435','18436','1','1','0','0','0','Forestier haut-elfe');
INSERT INTO `player_race` (`entry`, `displayid_m`, `displayid_w`, `scale`, `speed`, `aura1`, `aura2`, `aura3`, `comments`) VALUES ('22','27263','27263','1','1','0','0','0','Elfe de la nuit mage');
INSERT INTO `player_race` (`entry`, `displayid_m`, `displayid_w`, `scale`, `speed`, `aura1`, `aura2`, `aura3`, `comments`) VALUES ('23','8575','8575','1','1','0','0','0','Satyre rouge');
INSERT INTO `player_race` (`entry`, `displayid_m`, `displayid_w`, `scale`, `speed`, `aura1`, `aura2`, `aura3`, `comments`) VALUES ('24','613','613','1.3','1.1','0','0','0','Félin noir');
INSERT INTO `player_race` (`entry`, `displayid_m`, `displayid_w`, `scale`, `speed`, `aura1`, `aura2`, `aura3`, `comments`) VALUES ('28','11180','11180','1.5','1','0','0','0','Worgen blanc classique');
INSERT INTO `player_race` (`entry`, `displayid_m`, `displayid_w`, `scale`, `speed`, `aura1`, `aura2`, `aura3`, `comments`) VALUES ('26','7110','26250','1','1','0','0','0','Gobelin - Artisan');
INSERT INTO `player_race` (`entry`, `displayid_m`, `displayid_w`, `scale`, `speed`, `aura1`, `aura2`, `aura3`, `comments`) VALUES ('27','11387','11387','1','1','0','0','0','Gobelin - Voleur 2');
INSERT INTO `player_race` (`entry`, `displayid_m`, `displayid_w`, `scale`, `speed`, `aura1`, `aura2`, `aura3`, `comments`) VALUES ('29','0','0','2.5','2.3','34873','0','0','Dragon - Vol Noire');
INSERT INTO `player_race` (`entry`, `displayid_m`, `displayid_w`, `scale`, `speed`, `aura1`, `aura2`, `aura3`, `comments`) VALUES ('31','25852','25852','2.5','2.3','34873','0','0','Dragon - Vol de Bronze');
INSERT INTO `player_race` (`entry`, `displayid_m`, `displayid_w`, `scale`, `speed`, `aura1`, `aura2`, `aura3`, `comments`) VALUES ('32','0','25452','1','1','0','0','0','Val kyr - noire');
INSERT INTO `player_race` (`entry`, `displayid_m`, `displayid_w`, `scale`, `speed`, `aura1`, `aura2`, `aura3`, `comments`) VALUES ('33','0','26101','1','1','0','0','0','Val kyr - blanche');
INSERT INTO `player_race` (`entry`, `displayid_m`, `displayid_w`, `scale`, `speed`, `aura1`, `aura2`, `aura3`, `comments`) VALUES ('34','11539','11539','1','1','0','0','0','Ogre à deux têtes');

ALTER TABLE `player_race`
	ADD COLUMN `spell1` int(8) UNSIGNED DEFAULT '0' NOT NULL AFTER `aura3`,
	ADD COLUMN `spell2` int(8) UNSIGNED DEFAULT '0' NOT NULL AFTER `spell1`,
	ADD COLUMN `spell3` int(8) UNSIGNED DEFAULT '0' NOT NULL AFTER `spell2`;