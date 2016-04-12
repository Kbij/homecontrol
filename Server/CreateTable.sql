CREATE TABLE `Locations` (
  `idLocations` int(11) NOT NULL AUTO_INCREMENT,
  `client` varchar(45) COLLATE utf8_bin NOT NULL,
  `latitude` decimal(8,5) DEFAULT NULL,
  `longitude` decimal(8,5) DEFAULT NULL,
  `accuracy` int(11) DEFAULT NULL,
  `message` varchar(45) COLLATE utf8_bin DEFAULT NULL,
  `timestamp` datetime NOT NULL,
  PRIMARY KEY (`idLocations`))
 
