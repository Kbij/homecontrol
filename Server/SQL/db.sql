CREATE TABLE `Client` (
  `idClient` int(11) NOT NULL AUTO_INCREMENT,
  `clientName` varchar(45) COLLATE utf8_bin DEFAULT NULL,
  `lastMessage` datetime DEFAULT NULL,
  PRIMARY KEY (`idClient`),
  UNIQUE KEY `clientName_UNIQUE` (`clientName`)
);
CREATE TABLE `Location` (
  `idLocation` int(11) NOT NULL AUTO_INCREMENT,
  `idClient` int(11) NOT NULL,
  `latitude` decimal(8,5) NOT NULL,
  `longitude` decimal(8,5) NOT NULL,
  `accuracy` int(11) NOT NULL,
  `timestamp` datetime NOT NULL,
  PRIMARY KEY (`idLocation`)
)

