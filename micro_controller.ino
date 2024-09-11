#include <DHT.h>
#include <DHT_U.h>
#include <FastLED.h>
#include <SPI.h>
#include <OneWire.h>
#include <TimeLib.h>


#define LED_PIN 6             // LED
#define NUM_LEDS 120          // Set this to the number of LEDs in your strip
#define BRIGHTNESS 64         // Set brightness level (0-255)
#define LED_TYPE WS2812B      // LED light for plant growth
#define COLOR_ORDER GRB       // LED light
#define Relay_Pin 8           // for the pump
#define Moisture_Pin A0       // soil moisture sensor
#define DHT_Pin 2             // DHT22 
#define DHTTYPE DHT22         // humidity and temperature of air sensor
#define DS18B20_Pin 3         // DS18B20 water temperature sensor
#define TdsSensorPin A7       // TDS sensor (total dissolved solids)
#define VREF 5.0              // analog reference voltage(Volt) of the ADC
#define SCOUNT  30            // sum of sample point
#define pH_Pin A2             // pH sensor

const int csPin = 10;

CRGB leds[NUM_LEDS];
DHT dht(DHT_Pin, DHTTYPE);
OneWire ds(DS18B20_Pin);

const int dryThreshold = 365; // Soil moisture threshold for dry soil
const unsigned long pumpOffInterval = 40000; // Pump off duration in milliseconds
const unsigned long pumpOnDuration = 40000; // Pump on duration in milliseconds
unsigned long pumpOffMillis = 0;
unsigned long pumpStartMillis = 0;
bool isPumpOn = false;
bool isPumpOffDelay = false;

unsigned long previousMillis = 0;
bool isOn = true;

// TDS
int analogBuffer[SCOUNT];     // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0;
int copyIndex = 0;

float averageVoltage = 0;
float tdsValue = 0;
float temperature = 16;       // current temperature for compensation

void pHsetup();
float readpH();

void DSsetup();
float readWaterTemperature();
void DHTsetup();
float readAirHumid();
float readAirTemp();
float readTDS();
int getMedianNum(int bArray[], int iFilterLen);
void TDSsetup();
void LEDsetup();
void Relaysetup();
void MoistureSensorsetup();
void PumpON();
void PumpOFF();
int readADC(int channel);
int readMoisture();

void setup() {
  Serial.begin(115200); // Combine all Serial.begin calls into one
  pHsetup();
  DSsetup();
  DHTsetup();
  TDSsetup();
  LEDsetup();
  Relaysetup();
  MoistureSensorsetup();

  setTime(12, 0, 0, 1, 1, 2023); // Set initial time (12:00:00, 1 Jan 2023)
}

void pHsetup() {
  pinMode(pH_Pin, INPUT);
}

float readpH() {
  int pHValue = analogRead(pH_Pin);
  float voltage = pHValue * (5.0 / 1024.0);
  float pH = 7 + ((2.5 - voltage) / 0.18);
  pH = pH * -1;
  delay(1000);
  return pH;
}

// DS18B20
void DSsetup() {}

float readWaterTemperature() {
  byte data[12];
  byte address[8];

  if (!ds.search(address)) {
    ds.reset_search();
    return -1000;
  }
  if (OneWire::crc8(address, 7) != address[7]) {
    return -1000;
  }
  if (address[0] != 0x10 && address[0] != 0x28) {
    return -1000;
  }
  ds.reset();
  ds.select(address);
  ds.write(0x44, 1);

  byte present = ds.reset();
  ds.select(address);
  ds.write(0xBE);

  for (int i = 0; i < 9; i++) {
    data[i] = ds.read();
  }
  ds.reset_search();

  byte MSB = data[1];
  byte LSB = data[0];

  float tempRead = ((MSB << 8) | LSB);
  float TemperatureSUM = tempRead / 16;
  return TemperatureSUM;
}

// DHT22
void DHTsetup() {
  dht.begin();
}

// TDS
float readTDS() {
  static unsigned long analogSampleTimepoint = millis();
  if (millis() - analogSampleTimepoint > 40U) {     //every 40 milliseconds,read the analog value from the ADC
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);    //read the analog value and store into the buffer
    analogBufferIndex++;
    if (analogBufferIndex == SCOUNT) {
      analogBufferIndex = 0;
    }
  }

  static unsigned long printTimepoint = millis();
  if (millis() - printTimepoint > 800U) {
    printTimepoint = millis();
    for (copyIndex = 0; copyIndex < SCOUNT; copyIndex++) {
      analogBufferTemp[copyIndex] = analogBuffer[copyIndex];

      // read the analog value more stable by the median filtering algorithm, and convert to voltage value
      averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * (float)VREF / 1024.0;

      //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0)); 
      float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0);
      //temperature compensation
      float compensationVoltage = averageVoltage / compensationCoefficient;

      //convert voltage value to tds value
      tdsValue = (133.42 * compensationVoltage * compensationVoltage * compensationVoltage - 255.86 * compensationVoltage * compensationVoltage + 857.39 * compensationVoltage) * 0.5;
    }
    return tdsValue;
  }
}

// TDS
void TDSsetup() {
  pinMode(TdsSensorPin, INPUT);
}

// LED
void LEDsetup() {
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);

  // Turn on the LEDs with custom purple color initially
  for (int i = 0; i < NUM_LEDS/2; i++) {
    leds[i] = CRGB(128, 0, 128);  // Custom purple color (R, G, B)
  }

  // Turn on the LEDs with custom purple color initially
  for (int i = NUM_LEDS/2; i < NUM_LEDS; i++) {
    leds[i] = CRGB(128, 128, 128);  // Custom white color (R, G, B)
  }
  
  FastLED.show();

  previousMillis = millis();  // Initialize the timer
}

// Water Pump 
void Relaysetup() {
  // Initialize the relay pin as an output
  pinMode(Relay_Pin, OUTPUT);
}

// Soil Moisture Sensor
void MoistureSensorsetup() {
  pinMode(Moisture_Pin, INPUT);
}

// Function to turn on the water pump (since relay module is active low)
void PumpON() {
  digitalWrite(Relay_Pin, HIGH);
}

// Function to turn off the water pump
void PumpOFF() {
  digitalWrite(Relay_Pin, LOW);
}

// Function to read from the MCP3008
int readADC(int channel) {
  byte command = 0b11000000 | ((channel & 0x07) << 3);
  digitalWrite(csPin, LOW);
  SPI.transfer(command);
  int adcValue = (SPI.transfer(0) & 0x1F) << 8;
  adcValue |= SPI.transfer(0);
  digitalWrite(csPin, HIGH);
  return adcValue;
}

// Air Humidity and Temperature sensor DHT22 
int readMoisture() {
  int moistureValue = analogRead(Moisture_Pin);
  return moistureValue;
}

// Air Humidity
float readAirHumid() {
  float humidity = dht.readHumidity();
  if (isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
  }
  return humidity;
}

// Air Temperature
float readAirTemp() {
  float temperature = dht.readTemperature();
  if (isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
  }
  return temperature;
}

// median filtering algorithm
int getMedianNum(int bArray[], int iFilterLen) {
  int bTab[iFilterLen];
  for (byte i = 0; i < iFilterLen; i++)
    bTab[i] = bArray[i];
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++) {
    for (i = 0; i < iFilterLen - j - 1; i++) {
      if (bTab[i] > bTab[i + 1]) {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  if ((iFilterLen & 1) > 0) {
    bTemp = bTab[(iFilterLen - 1) / 2];
  }
  else {
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  }
  return bTemp;
}

void loop() {
  unsigned long currentMillis = millis();

  // Calculate current time in hours and minutes
  int currentHour = hour();
  int currentMinute = minute();

  // Manage the LED on/off timing
  if (isOn && (currentHour >= 18 || currentHour < 6)) {
    // Turn off the LEDs
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Black;  // Set color to black to turn off
    }
    FastLED.show();
    isOn = false;
  } else if (!isOn && (currentHour >= 6 && currentHour < 18)) {
    // Turn on the LEDs with custom purple color
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB(128, 0, 128);  // Custom purple color (R, G, B)
    }
    FastLED.show();
    isOn = true;
  }

  // Collect sensor data
  int soilMoisture = analogRead(Moisture_Pin);
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  float waterTemperature = readWaterTemperature();
  float pH = readpH();
  float tds = readTDS();

  // Pump control based on soil moisture
  if (soilMoisture > dryThreshold && !isPumpOn) {
    PumpON();
    isPumpOn = true;
    pumpStartMillis = currentMillis;
  }

  if (isPumpOn && (currentMillis - pumpStartMillis >= pumpOnDuration)) {
    PumpOFF();
    isPumpOn = false;
  }

  // Print data in JSON format
  Serial.print("{\"soil_moisture\":");
  Serial.print(soilMoisture);
  Serial.print(",\"humidity\":");
  Serial.print(humidity);
  Serial.print(",\"temperature\":");
  Serial.print(temperature);
  Serial.print(",\"water_temperature\":");
  Serial.print(waterTemperature);
  Serial.print(",\"pH\":");
  Serial.print(pH);
  Serial.print(",\"TDS\":");
  Serial.print(tds);
  Serial.println("}");

  delay(2000);  // Delay to avoid spamming serial output
}

