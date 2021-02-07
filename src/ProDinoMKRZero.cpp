// ProDinoMKRZero.cpp
// Company: KMP Electronics Ltd, Bulgaria
// Web: https://kmpelectronics.eu/
// Supported boards: 
//		- KMP ProDino MKR Zero V1 https://kmpelectronics.eu/products/prodino-mkr-zero-v1/
//		- KMP ProDino MKR Zero Ethernet V1 https://kmpelectronics.eu/products/prodino-mkr-zero-ethernet-v1/
//		- KMP ProDino MKR GSM V1 https://kmpelectronics.eu/products/prodino-mkr-gsm-v1/
//		- KMP ProDino MKR GSM Ethernet V1  https://kmpelectronics.eu/products/prodino-mkr-gsm-ethernet-v1/
// Description:
//		Library for supported board. It contains base methods to work with boards.
// Version: 1.1.0
// Date: 27.09.2018
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu> & Dimitar Antonov <d.antonov@kmpelectronics.eu>

#include "ProDinoMKRZero.h"

// Relay outputs pins.
#define Rel1Pin  21 // PA07
#define Rel2Pin  20 // PA06
#define Rel3Pin  19 // PA05
#define Rel4Pin  18 // PA04

#define OptoIn1Pin 16 // PB02
#define OptoIn2Pin 7  // PA21
#define OptoIn3Pin 0  // PA22
#define OptoIn4Pin 1  // PA23

// Status led pin.
#define	StatusLedPin  6 // PA20

// W5500 pins.
#define W5500ResetPin 5  // PB11
#define W5500CSPin    4  // PB10

// RS485 pins. Serial.
#define RS485Pin      3  // PA11
#define RS485Serial	  Serial1
#define RS485Transmit HIGH
#define RS485Receive  LOW

/**
 * @brief Relay pins.
 */
const uint8_t Relay_Pins[RELAY_COUNT] =
{ Rel1Pin, Rel2Pin, Rel3Pin, Rel4Pin };

/**
 * @brief Input pins.
 */
const int OPTOIN_PINS[OPTOIN_COUNT] =
{ OptoIn1Pin, OptoIn2Pin, OptoIn3Pin, OptoIn4Pin };

ProDinoMKRZeroClass ProDinoMKRZero;
BoardType _board;

void ProDinoMKRZeroClass::init(BoardType board)
{
	init(board, true, true);
}

void ProDinoMKRZeroClass::init(BoardType board, bool startEthernet, bool startGSM)
{
	_board = board;

	// Relay pins init.
	pinMode(Rel1Pin, OUTPUT);
	pinMode(Rel2Pin, OUTPUT);
	pinMode(Rel3Pin, OUTPUT);
	pinMode(Rel4Pin, OUTPUT);

	// Opto inputs pins init.
	pinMode(OptoIn1Pin, INPUT);
	pinMode(OptoIn2Pin, INPUT);
	pinMode(OptoIn3Pin, INPUT);
	pinMode(OptoIn4Pin, INPUT);

	// Set status led output pin.
	pinMode(StatusLedPin, OUTPUT);
	digitalWrite(StatusLedPin, LOW);

	// RS485 pin init.
	pinMode(RS485Pin, OUTPUT);
	digitalWrite(RS485Pin, RS485Receive);

	InitEthernet(startEthernet);
	InitGSM(startGSM);
}

void ProDinoMKRZeroClass::InitEthernet(bool startEthernet)
{
	if (_board == ProDino_MKR_Zero_Ethernet || _board == ProDino_MKR_GSM_Ethernet)
	{
		// W5500 pin init.
		pinMode(W5500ResetPin, OUTPUT);

		if (startEthernet)
		{
			RestartEthernet();
			Ethernet.init(W5500CSPin);
		}
		else
		{
			digitalWrite(W5500ResetPin, LOW);
		}
	}
}

void ProDinoMKRZeroClass::InitGSM(bool startGSM)
{
	if (_board == ProDino_MKR_GSM || _board == ProDino_MKR_GSM_Ethernet)
	{
		// Start serial communication with the GSM modem
		SerialGSM.begin(115200);

		// Turn on the GSM module by triggering GSM_RESETN pin
		pinMode(GSM_RESETN, OUTPUT);
		if (startGSM)
		{
			RestartGSM();
		}
		else
		{
			digitalWrite(GSM_RESETN, HIGH);
		}
	}
}

void ProDinoMKRZeroClass::RestartGSM()
{
	// Reset occurs when a low level is applied to the RESET_N pin, which is normally set high by an internal pull-up, for a valid time period min 10 mS
	digitalWrite(GSM_RESETN, HIGH);
	delay(20);
	digitalWrite(GSM_RESETN, LOW);
}

void ProDinoMKRZeroClass::RestartEthernet()
{
	// RSTn Pull-up Reset (Active low) RESET should be held low at least 500 us for W5500 reset.
	digitalWrite(W5500ResetPin, LOW);
	delay(600);
	digitalWrite(W5500ResetPin, HIGH);
}

bool ProDinoMKRZeroClass::GetStatusLed()
{
	return digitalRead(StatusLedPin);
}

void ProDinoMKRZeroClass::SetStatusLed(bool state)
{
	digitalWrite(StatusLedPin, state);
}

void ProDinoMKRZeroClass::OnStatusLed()
{
	SetStatusLed(true);
}

void ProDinoMKRZeroClass::OffStatusLed()
{
	SetStatusLed(false);
}

void ProDinoMKRZeroClass::NotStatusLed()
{
	SetStatusLed(!GetStatusLed());
}

/* ----------------------------------------------------------------------- */
/* Relays methods. */
/* ----------------------------------------------------------------------- */

void ProDinoMKRZeroClass::SetRelayState(uint8_t relayNumber, bool state)
{
	// Check if relayNumber is out of range - return.
	if (relayNumber > RELAY_COUNT - 1)
	{
		return;
	}

	digitalWrite(Relay_Pins[relayNumber], state);
}

void ProDinoMKRZeroClass::SetRelayState(Relay relay, bool state)
{
	SetRelayState((uint8_t)relay, state);
}

void ProDinoMKRZeroClass::SetAllRelaysState(bool state)
{
	for (uint8_t i = 0; i < RELAY_COUNT; i++)
	{
		SetRelayState(i, state);
	}
}

void ProDinoMKRZeroClass::SetAllRelaysOn()
{
	SetAllRelaysState(true);
}

void ProDinoMKRZeroClass::SetAllRelaysOff()
{
	SetAllRelaysState(false);
}

bool ProDinoMKRZeroClass::GetRelayState(uint8_t relayNumber)
{
	// Check if relayNumber is out of range - return false.
	if (relayNumber > RELAY_COUNT - 1)
	{
		return false;
	}

	return digitalRead(Relay_Pins[relayNumber]);
}

bool ProDinoMKRZeroClass::GetRelayState(Relay relay)
{
	return GetRelayState((uint8_t)relay);
}

/* ----------------------------------------------------------------------- */
/* Opto input methods. */
/* ----------------------------------------------------------------------- */

bool ProDinoMKRZeroClass::GetOptoInState(uint8_t optoInNumber)
{
	// Check if optoInNumber is out of range - return false.
	if (optoInNumber > OPTOIN_COUNT - 1)
	{
		return false;
	}

	return !digitalRead(OPTOIN_PINS[optoInNumber]);
}

bool ProDinoMKRZeroClass::GetOptoInState(OptoIn optoIn)
{
	return GetOptoInState((uint8_t)optoIn);
}

/* ----------------------------------------------------------------------- */
/* RS485 methods. */
/* ----------------------------------------------------------------------- */

void ProDinoMKRZeroClass::RS485Begin(unsigned long baud)
{
	RS485Begin(baud, SERIAL_8N1);
}

void ProDinoMKRZeroClass::RS485Begin(unsigned long baud, uint16_t config)
{
	RS485Serial.begin(baud, config);
}

void ProDinoMKRZeroClass::RS485End()
{
	RS485Serial.end();
}

/**
* @brief Begin write data to RS485.
*
* @return void
*/
void RS485BeginWrite()
{
	digitalWrite(RS485Pin, RS485Transmit);
}

/**
* @brief End write data to RS485.
*
* @return void
*/
void RS485EndWrite()
{
	RS485Serial.flush();
	digitalWrite(RS485Pin, RS485Receive);
}

size_t ProDinoMKRZeroClass::RS485Write(uint8_t data)
{
	RS485BeginWrite();

	size_t result = RS485Serial.write(data);

	RS485EndWrite();

	return result;
}

size_t ProDinoMKRZeroClass::RS485Write(char data)
{
	return RS485Write((uint8_t)data);
}

size_t ProDinoMKRZeroClass::RS485Write(const char* data)
{
	RS485BeginWrite();

	size_t len = strlen(data);
	size_t result = 0;
	while (len > 0)
	{
		result += RS485Serial.write(*data++);
		--len;
	}

	RS485EndWrite();

	return result;
}

size_t ProDinoMKRZeroClass::RS485Write(uint8_t* data, uint8_t dataLen)
{
	RS485BeginWrite();

	size_t result = 0;
	for (size_t i = 0; i < dataLen; i++)
	{
		result += RS485Serial.write(data[i]);
	}

	RS485EndWrite();

	return result;
}

int ProDinoMKRZeroClass::RS485Read()
{
	return RS485Read(10, 10);
}

int ProDinoMKRZeroClass::RS485Read(unsigned long delayWait, uint8_t repeatTime)
{
	// If the buffer is empty, wait until the data arrives.
	while (!RS485Serial.available())
	{
		delay(delayWait);
		--repeatTime;

		if (repeatTime == 0)
		{
			return -1;
		}
	}

	return RS485Serial.read();
}
