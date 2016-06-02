CREATE TABLE `TemperatureSensor` (
  `idTemperatureSensor` int(11) NOT NULL AUTO_INCREMENT,
  `idRoom` int(11) NOT NULL,
  `sensorAddress` varchar(45) COLLATE utf8_bin NOT NULL,
  PRIMARY KEY (`idTemperatureSensor`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

