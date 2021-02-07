// BlynkGSMTiny.ino
// Company: KMP Electronics Ltd, Bulgaria
// Web: https://kmpelectronics.eu/
// Supported boards:
//		- KMP ProDino MKR GSM V1 https://kmpelectronics.eu/products/prodino-mkr-gsm-v1/
//		- KMP ProDino MKR GSM Ethernet V1  https://kmpelectronics.eu/products/prodino-mkr-gsm-ethernet-v1/
// Description:
//		This Blynk example communicate through GSM module.
// Example link: https://kmpelectronics.eu/tutorials-examples/prodino-mkr-versions-examples/
// Version: 1.0.0
// Date: 27.09.2018
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>

// --------------------------------------------------------------------------------
// Prerequisites:
//	Before start this example you need to install:
//		Install Blynk library: Sketch\Include library\Menage Libraries... find ... and click Install.
//         - Blynk
//         - TinyGSM
//         - DHT sensor library by Adafruit
//		Connect DHT22 sensor(s) to GROVE connector. Only one we use in this example. Use pins: 
//			- sensor GROVE_D0, Vcc+, Gnd(-);
//  ProDino MKR series -> Blynk pins map:
//		Relay1 -> V1 {Type: "Button", Name: "Relay 1", Color: "Green", Output: "V1", Mode: "Switch" }
//		Relay2 -> V2 {Type: "Button", Name: "Relay 2", Color: "Blue", Output: "V2", Mode: "Switch" }
//		Relay3 -> V3 {Type: "Button", Name: "Relay 3", Color: "LightBlue", Output: "V3", Mode: "Switch" }
//		Relay4 -> V4 {Type: "Button", Name: "Relay 4", Color: "Orange", Output: "V4", Mode: "Switch" }
//		OptoIn1 -> V5 {Type: "LED", Name: "In 1", Color: "Green", Input: "V5" }
//		OptoIn2 -> V6 {Type: "LED", Name: "In 2", Color: "Blue", Input: "V6" }
//		OptoIn3 -> V7 {Type: "LED", Name: "In 3", Color: "LightBlue", Input: "V7" }
//		OptoIn4 -> V8 {Type: "LED", Name: "In 4", Color: "Orange", Input: "V8" }
//		DHT1T -> V9 {Type: "Value Display", Name: "Temperature", Color: "Green", Input: "V9", Min:"-40", Max:"80", ReadingFrecuency: "5sec" }
//		DHT1H -> V10 {Type: "Value Display", Name: "Humidity", Color: "Green", Input: "V10", Min:"0", Max:"100", ReadingFrecuency: "5sec" }

#include <KMPProDinoMKRZero.h>
#include <KMPCommon.h>
#include <DHT.h>

#define DEBUG
#define BLYNK_DEBUG
#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space

#define TINY_GSM_MODEM_UBLOX

#include <TinyGsmClient.h>
#include <BlynkSimpleSIM800.h>

// You have to get your Authentication Token through Blynk Application.
const char AUTH_TOKEN[] = "123456789012345678901234567890123";

const char PINNUMBER[] = "";
// replace your GPRS APN
const char GPRS_APN[] = "xxxx"; 
// replace with your GPRS login
const char GPRS_LOGIN[] = "xxxx";
// replace with your GPRS password
const char GPRS_PASSWORD[] = "xxxx";

// Define sensors structure.
struct MeasureHT_t
{
	// Enable sensor - true, disable - false.
	bool IsEnable;
	// Name of sensor. Example: "First sensor".
	String Name;
	// DHT object with settings. Example: DHT(GROVE_D0 /* connected pin */, DHT22 /* sensor type */, 11 /* Constant */)
	DHT dht;
	// Store, read humidity from sensor.
	float Humidity;
	// Store, read temperature from sensor.
	float Temperature;
};

// Sensors count. 
#define SENSOR_COUNT 1

// Define array of 2 sensors.
MeasureHT_t _measureHT[SENSOR_COUNT] =
{
	{ true, "Sensor 1", DHT(GROVE_D0, DHT22, 11), NAN, NAN }
};

// Check sensor data, interval in milliseconds.
const long CHECK_HT_INTERVAL_MS = 10000;
// Store last measure time.
unsigned long _mesureTimeout;				

// Opto input structure.
struct OptoIn_t
{
	OptoIn Input;
	WidgetLED Widget;
	bool Status;
};

// Store opto input data, settings and processing objects.
OptoIn_t _optoInputs[OPTOIN_COUNT] =
{
	{ OptoIn1, WidgetLED(V5), false },
	{ OptoIn2, WidgetLED(V6), false },
	{ OptoIn3, WidgetLED(V7), false },
	{ OptoIn4, WidgetLED(V8), false }
};

// It supports work with GSM Modem.
TinyGsm modem(SerialGSM);

/**
* @brief Setup void. Ii is Arduino executed first. Initialize DiNo board.
*
*
* @return void
*/
void setup()
{
	delay(5000);
#ifdef DEBUG
	Serial.begin(115200);
	Serial.println("The example BlynkGSMTiny is starting...");
#endif

	// Init Dino board. Set pins, start GSM.
	KMPProDinoMKRZero.init(ProDino_MKR_GSM);

#ifdef DEBUG
	Serial.println("Initializing modem...");
#endif
	modem.init();

	String modemInfo = modem.getModemInfo();
	if (modemInfo == "")
	{
#ifdef DEBUG
		Serial.println("Modem is not started!!!");
#endif
		while (true) {}
	}

	// Unlock your SIM card if it locked with a PIN code. 
	// If PIN is not valid don't try more than 3 time because the SIM card locked and need unlock with a PUK code.
	if (strlen(PINNUMBER) > 0 && !modem.simUnlock(PINNUMBER))
	{
#ifdef DEBUG
		Serial.println("PIN code is not valid! STOP!!!");
#endif
		while (true) {}
	}

	// Starts DHT sensors.
	for (uint8_t i = 0; i < SENSOR_COUNT; i++)
	{
		MeasureHT_t* measureHT = &_measureHT[i];
		if (measureHT->IsEnable)
		{
			measureHT->dht.begin();
		}
	}

	_mesureTimeout = 0;

#ifdef DEBUG
	Serial.println("The example BlynkGSMTiny is started.");
#endif

	Blynk.begin(AUTH_TOKEN, modem, GPRS_APN, GPRS_LOGIN, GPRS_PASSWORD);
}

/**
* @brief Loop void. Arduino executed second.
*
*
* @return void
*/
void loop(void)
{
	ProcessDHTSensors(false);
	ProcessOptoInputs(false);

	Blynk.run();
}

/**
 * @brief Reading temperature and humidity from DHT sensors every X seconds and if data is changed send it to Blynk.
 *
 * @return void
 */
void ProcessDHTSensors(bool force)
{
	// Checking if time to measure is occurred
	if (millis() > _mesureTimeout)
	{
		int firstFreeVirtualPin = V9;

		for (uint8_t i = 0; i < SENSOR_COUNT; i++)
		{
			// Get sensor structure.
			MeasureHT_t* measureHT = &_measureHT[i];
			// Is enable - read data from sensor.
			if (measureHT->IsEnable)
			{
				measureHT->dht.read(true);
				float humidity = measureHT->dht.readHumidity();
				float temperature = measureHT->dht.readTemperature();

				if (measureHT->Humidity != humidity || measureHT->Temperature != temperature || force)
				{
					measureHT->Humidity = humidity;
					measureHT->Temperature = temperature;

					// Write pair of data in pins V9, V10. If have second write V11, V12.
					Blynk.virtualWrite(firstFreeVirtualPin++, measureHT->Temperature);
					Blynk.virtualWrite(firstFreeVirtualPin++, measureHT->Humidity);
				}
			}
		}

		// Set next time to read data.
		_mesureTimeout = millis() + CHECK_HT_INTERVAL_MS;
	}
}

/**
* @brief Processing data from isolated inputs. It will send data to Blynk if only the input statuses were changed.
*
* @return void
*/
void ProcessOptoInputs(bool force)
{
	for (int i = 0; i < OPTOIN_COUNT; i++)
	{
		OptoIn_t* optoInput = &_optoInputs[i];
		bool currentStatus = KMPProDinoMKRZero.GetOptoInState(optoInput->Input);
		if (optoInput->Status != currentStatus || ((bool)optoInput->Widget.getValue()) != currentStatus || force)
		{
			Serial.println("Opto input " + String(i + 1) + " status changed to -> \"" + currentStatus + "\". WidgetLED value: " + optoInput->Widget.getValue());

			currentStatus ? optoInput->Widget.on() : optoInput->Widget.off();
			optoInput->Status = currentStatus;
		}
	}
}

/**
 * @brief Set relay state.
 *
 * @return void
 */
void SetRelay(Relay relay, int status)
{
	KMPProDinoMKRZero.SetRelayState(relay, status == 1);
}

/*****************************
* Blynk methods.
*****************************/
/**
 * @brief This function will be run every time when Blynk connection is established.
 *
 */
BLYNK_CONNECTED() {
	// Request Blynk server to re-send latest values for all pins.
	Blynk.syncAll();

	ProcessDHTSensors(true);
	ProcessOptoInputs(true);
}

/**
 * @brief Set Relay 1 state.
 *			On virtual pin 1.
 */
BLYNK_WRITE(V1)
{
	SetRelay(Relay1, param.asInt());
}

/**
* @brief Set Relay 1 state.
*			On virtual pin 1.
*/
BLYNK_READ(V1)
{
	Blynk.virtualWrite(V1, KMPProDinoMKRZero.GetRelayState(Relay1));
}

/**
 * @brief Set Relay 2 state.
 *			On virtual pin 2.
 */
BLYNK_WRITE(V2)
{
	SetRelay(Relay2, param.asInt());
}

/**
 * @brief Set Relay 3 state.
 *			On virtual pin 3.
 */
BLYNK_WRITE(V3)
{
	SetRelay(Relay3, param.asInt());
}

/**
 * @brief Set Relay 4 state.
 *			On virtual pin 4.
 */
BLYNK_WRITE(V4)
{
	SetRelay(Relay4, param.asInt());
}