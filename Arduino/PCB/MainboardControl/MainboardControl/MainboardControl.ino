// HCServer Mainboard 
/*
Write I2C data from Raspberry to board:
i2cset -y 1 0x8 48 255 24 i

*/

#include <Wire.h>
const int I2C_SLAVE_ADR = 0x08;
unsigned char counters[9];
byte receiveBuffer[3];
bool commandReceived;

const char REL0_CMD = '0';
const char REL1_CMD = '1';
const char REL2_CMD = '2';
const char REL3_CMD = '3';
const char REL4_CMD = '4';
const char REL5_CMD = '5';
const char REL6_CMD = '6';
const char REL7_CMD = '7';

const char WD_CMD = 'w';
const char WD_ENABLE_CMD = 'W';

const int REL0 = 0;
const int REL1 = 1;
const int REL2 = 2;
const int REL3 = 3;
const int REL4 = 4;
const int REL5 = 5;
const int REL6 = 6;
const int REL7 = 7;
const int WD = 8;

const int PIN_REL0 = 14;
const int PIN_REL1 = 15;
const int PIN_REL2 = 16;
const int PIN_REL3 = 17;
const int PIN_REL4 = 12;
const int PIN_REL5 = 13;
const int PIN_REL6 = 2;
const int PIN_REL7 = 3;

const int PIN_WD = 8;

bool watchdogEnabled;

void setup() {
	watchdogEnabled = false;
	commandReceived = false;
	Wire.begin(I2C_SLAVE_ADR);                // join i2c bus with address #8
	Wire.onReceive(receiveEvent); // register event
	Serial.begin(19200);           // start serial for output

	pinMode(PIN_REL0, OUTPUT);
	pinMode(PIN_REL1, OUTPUT);
	pinMode(PIN_REL2, OUTPUT);
	pinMode(PIN_REL3, OUTPUT);
	pinMode(PIN_REL4, OUTPUT);
	pinMode(PIN_REL5, OUTPUT);
	pinMode(PIN_REL6, OUTPUT);
	pinMode(PIN_REL7, OUTPUT);
	pinMode(PIN_WD, OUTPUT);

	memset(counters, 0, sizeof(counters));
}

void processCounters()
{
	for (int pos = REL0; pos <= WD; pos++)
	{
		switch (pos)
		{
			case REL0:
			{
				digitalWrite(PIN_REL0, (counters[REL0] > 0));
				break;
			}
			case REL1:
			{
				digitalWrite(PIN_REL1, (counters[REL1] > 0));
				break;
			}
			case REL2:
			{
				digitalWrite(PIN_REL2, (counters[REL2] > 0));
				break;
			}
			case REL3:
			{
				digitalWrite(PIN_REL3, (counters[REL3] > 0));
				break;
			}
			case REL4:
			{
				digitalWrite(PIN_REL4, (counters[REL4] > 0));
				break;
			}
			case REL5:
			{
				digitalWrite(PIN_REL5, (counters[REL5] > 0));
				break;
			}
			case REL6:
			{
				digitalWrite(PIN_REL6, (counters[REL6] > 0));
				break;
			}
			case REL7:
			{
				digitalWrite(PIN_REL7, (counters[REL7] > 0));
				break;
			}

			case WD:
			{
				if ((counters[WD] == 0) && watchdogEnabled)
				{
					//Reset Raspberry
					digitalWrite(PIN_WD, 1);
					delay(100);
					digitalWrite(PIN_WD, 0);

					watchdogEnabled = false;
				}
				break;
			}
		}
	}
}

void processReceived(byte cmd, byte value)
{
	switch (cmd)
		{
		case REL0_CMD:
		{
			counters[REL0] = value;
			break;
		}
		case REL1_CMD:
		{
			counters[REL1] = value;
			break;
		}
		case REL2_CMD:
		{
			counters[REL2] = value;
			break;
		}
		case REL3_CMD:
		{
			counters[REL3] = value;
			break;
		}
		case REL4_CMD:
		{
			counters[REL4] = value;
			break;
		}
		case REL5_CMD:
		{
			counters[REL5] = value;
			break;
		}
		case REL6_CMD:
		{
			counters[REL6] = value;
			break;
		}
		case REL7_CMD:
		{
			counters[REL7] = value;
			break;
		}
		case WD_CMD:
		{
			counters[WD] = value;
			break;
		}
		case WD_ENABLE_CMD:
		{
			watchdogEnabled = value > 0;
		}
	}
}

void loop()
{
	delay(1000);
	for (int pos = REL0; pos <= WD; pos++)
	{
		if (counters[pos] > 0)
		{
			counters[pos] -= 1;
		}
	}

	//Check if new data need processing
	byte cmd;
	byte value;
	bool process = false;

	noInterrupts();
	if (commandReceived)
	{
		process = true;
		cmd = receiveBuffer[0];
		value = receiveBuffer[1];
		commandReceived = false;
	}
	interrupts();

	if (process)
	{
		processReceived(cmd, value);
	}

	printCounters();
	processCounters();
}

void printCounters()
{
	Serial.print("Counters: ");
	bool first = true;
	for (int pos = REL0; pos <= WD; pos++)
	{
		if (!first)
		{
			Serial.print(", ");
		}
		Serial.print(pos);
		Serial.print(':');
		Serial.print(counters[pos]);
		first = false;
	}

	Serial.print(", wd: ");
	Serial.print(watchdogEnabled);

	Serial.println();
}

byte CRC8(const byte *data, byte len)
{
	byte crc = 0x00;
	while (len--)
	{
		byte extract = *data++;
		for (byte tempI = 8; tempI; tempI--)
		{
			byte sum = (crc ^ extract) & 0x01;
			crc >>= 1;
			if (sum)
			{
				crc ^= 0x8C;
			}
			extract >>= 1;
		}
	}
	return crc;
}

void receiveEvent(int bytesReceived)
{
	// 3 bytes = counter set (cmd, value, crc )
	if (bytesReceived == 3)
	{
		int pos = 0;
		while (Wire.available() > 0 & pos < 3)
		{
			receiveBuffer[pos] = Wire.read();
			pos++;
		}

		byte crc = CRC8(receiveBuffer, 2);
		if (receiveBuffer[2] == crc)
		{
			commandReceived = true;
		}
		else
		{
			Serial.print("Invalid crc, must be: ");
			Serial.println((int) crc);
		}
	}
	else
	{
		Serial.print("Unknown receive: ");
		while (Wire.available())
		{
			Serial.print(Wire.read());
			Serial.print(" ");
		}
		Serial.println();
	}
}