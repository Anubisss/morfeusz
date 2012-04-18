--
-- Table structure for table `characters`
--

DROP TABLE IF EXISTS `characters`;
CREATE TABLE `characters` (
  `guid` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Global Unique Identifier',
  `account` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Account Identifier',
  `name` varchar(12) NOT NULL DEFAULT '',
  `race` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `class` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `gender` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `level` tinyint(3) unsigned NOT NULL DEFAULT '0',
  -- `xp` int(10) unsigned NOT NULL DEFAULT '0',
  -- `money` int(10) unsigned NOT NULL DEFAULT '0',
  `playerBytes` int(10) unsigned NOT NULL DEFAULT '0',
  `playerBytes2` int(10) unsigned NOT NULL DEFAULT '0',
  -- `playerFlags` int(10) unsigned NOT NULL DEFAULT '0',
  `position_x` float NOT NULL DEFAULT '0',
  `position_y` float NOT NULL DEFAULT '0',
  `position_z` float NOT NULL DEFAULT '0',
  `map` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Map Identifier',
  -- `instance_id` int(11) unsigned NOT NULL DEFAULT '0',
  -- `dungeon_difficulty` tinyint(1) unsigned NOT NULL DEFAULT '0',
  -- `orientation` float NOT NULL DEFAULT '0',
  -- `taximask` longtext,
  -- `online` tinyint(3) unsigned NOT NULL DEFAULT '0',
  -- `cinematic` tinyint(3) unsigned NOT NULL DEFAULT '0',
  -- `totaltime` int(11) unsigned NOT NULL DEFAULT '0',
  -- `leveltime` int(11) unsigned NOT NULL DEFAULT '0',
  -- `logout_time` bigint(20) unsigned NOT NULL DEFAULT '0',
  -- `is_logout_resting` tinyint(3) unsigned NOT NULL DEFAULT '0',
  -- `rest_bonus` float NOT NULL DEFAULT '0',
  -- `resettalents_cost` int(11) unsigned NOT NULL DEFAULT '0',
  -- `resettalents_time` bigint(20) unsigned NOT NULL DEFAULT '0',
  -- `trans_x` float NOT NULL DEFAULT '0',
  -- `trans_y` float NOT NULL DEFAULT '0',
  -- `trans_z` float NOT NULL DEFAULT '0',
  -- `trans_o` float NOT NULL DEFAULT '0',
  -- `transguid` bigint(20) unsigned NOT NULL DEFAULT '0',
  -- `extra_flags` tinyint(3) unsigned NOT NULL DEFAULT '0',
  -- `stable_slots` tinyint(1) unsigned NOT NULL DEFAULT '0',
  -- `at_login` int(11) unsigned NOT NULL DEFAULT '0',
  `zone` int(11) unsigned NOT NULL DEFAULT '0',
  -- `death_expire_time` bigint(20) unsigned NOT NULL DEFAULT '0',
  -- `taxi_path` text,
  -- `arena_pending_points` int(10) unsigned NOT NULL DEFAULT '0',
  -- `arenaPoints` int(10) unsigned NOT NULL DEFAULT '0',
  -- `totalHonorPoints` int(10) unsigned NOT NULL DEFAULT '0',
  -- `todayHonorPoints` int(10) unsigned NOT NULL DEFAULT '0',
  -- `yesterdayHonorPoints` int(10) unsigned NOT NULL DEFAULT '0',
  -- `totalKills` int(10) unsigned NOT NULL DEFAULT '0',
  -- `todayKills` smallint(5) unsigned NOT NULL DEFAULT '0',
  -- `yesterdayKills` smallint(5) unsigned NOT NULL DEFAULT '0',
  -- `chosenTitle` int(10) unsigned NOT NULL DEFAULT '0',
  -- `watchedFaction` int(10) NOT NULL DEFAULT '0',
  -- `drunk` smallint(5) unsigned NOT NULL DEFAULT '0',
 --  `health` int(10) unsigned NOT NULL DEFAULT '0',
  -- `power1` int(10) unsigned NOT NULL DEFAULT '0',
 --  `power2` int(10) unsigned NOT NULL DEFAULT '0',
 --  `power3` int(10) unsigned NOT NULL DEFAULT '0',
  -- `power4` int(10) unsigned NOT NULL DEFAULT '0',
  -- `power5` int(10) unsigned NOT NULL DEFAULT '0',
 --  `latency` int(11) unsigned NOT NULL DEFAULT '0',
 --  `exploredZones` longtext,
  -- `equipmentCache` longtext,
  -- `ammoId` int(10) unsigned NOT NULL DEFAULT '0',
 --  `knownTitles` longtext,
  -- `actionBars` tinyint(3) unsigned NOT NULL DEFAULT '0',
 --  `xp_blocked` tinyint(3) unsigned NOT NULL DEFAULT '0',
  -- `lastGenderChange` bigint(11) NOT NULL DEFAULT '0',
 --  `deleted` tinyint(3) unsigned NOT NULL DEFAULT '0',
 --  `deleted_time` bigint(20) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`),
  KEY `idx_account` (`account`)
  -- KEY `idx_online` (`online`),
 --  KEY `idx_name` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Characters';