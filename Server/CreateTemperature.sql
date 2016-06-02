CREATE TABLE `Temperature` (
  `idTemperature` int(11) NOT NULL AUTO_INCREMENT,
  `idSensor` int(11) NOT NULL,
  `temperature` decimal(4,2) NOT NULL,
  `date` datetime NOT NULL,
  PRIMARY KEY (`idTemperature`)
) ENGINE=InnoDB AUTO_INCREMENT=11 DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

