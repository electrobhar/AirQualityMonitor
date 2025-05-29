#include <SoftwareSerial.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Pin Definitions
const int mq8Pin = A0;
const int mq135Pin = A2;
const int mq7Pin = A1;
const int dhtPin = 7;
const int pmsRX = 8;
const int pmsTX = 9;
const int btRX = 10;
const int btTX = 11;

// Calibration constants
#define MQ7_R0    10.0
#define MQ8_R0    10.0
#define MQ135_R0  10.0
#define RL        10.0

float calculatePPM(int analogValue, float R0, float a, float b) {
  float Vout = (analogValue / 1023.0) * 5.0;
  float Rsensor = (5.0 - Vout) / Vout * RL;
  float ratio = Rsensor / R0;
  return pow(10, ((log10(ratio) - b) / a));
}

// DHT Sensor
#define DHTTYPE DHT11
DHT dht(dhtPin, DHTTYPE);

// Serial interfaces
SoftwareSerial bluetooth(btRX, btTX);
SoftwareSerial pmsSerial(pmsRX, pmsTX);

// PMS5003 data buffer
uint8_t pmsData[32];

// LCD: 20 columns, 4 rows
LiquidCrystal_I2C lcd(0x27, 20, 4);

unsigned long previousMillis = 0;
const long interval = 4000;
int screen = 0;

void setup() {
  Serial.begin(9600);
  bluetooth.begin(9600);
  pmsSerial.begin(9600);
  dht.begin();

  pinMode(mq8Pin, INPUT);
  pinMode(mq135Pin, INPUT);
  pinMode(mq7Pin, INPUT);

  lcd.init();
  lcd.backlight();

  Serial.println("System Initialized.");
  bluetooth.println("System Initialized.");
  lcd.setCursor(2, 1);
  lcd.print("System Initialized");
  delay(2000);
  lcd.clear();
}

void loop() {
  int mq8Raw = analogRead(mq8Pin);
  int mq135Raw = analogRead(mq135Pin);
  int mq7Raw = analogRead(mq7Pin);

  float mq7PPM = calculatePPM(mq7Raw, MQ7_R0, -0.77, 0.82);      // CO
  float mq8PPM = calculatePPM(mq8Raw, MQ8_R0, -0.48, 0.38);      // H2
  float mq135PPM = calculatePPM(mq135Raw, MQ135_R0, -0.42, 0.36); // CO2/others

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  String pmData = readPMS();

  String dataToSend = "MQ8: " + String(mq8PPM, 1) + "ppm" +
                      " | MQ135: " + String(mq135PPM, 1) + "ppm" +
                      " | MQ7: " + String(mq7PPM, 1) + "ppm" +
                      " | Temp: " + String(temperature) + "C" +
                      " | Humidity: " + String(humidity) + "%" +
                      " | " + pmData;

  Serial.println(dataToSend);
  bluetooth.println(dataToSend);

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    lcd.clear();

    switch (screen) {
      case 0:
        // Temp & Humidity
        lcd.setCursor(0, 0);
        lcd.print("Temperature: ");
        lcd.print(temperature, 1);
        lcd.print("C");
        lcd.setCursor(0, 1);
        lcd.print("Humidity: ");
        lcd.print(humidity, 1);
        lcd.print("%");
        break;

      case 1:
        // Gas sensor PPMs
        lcd.setCursor(0, 0);
        lcd.print("MQ7 (CO): ");
        lcd.print(mq7PPM, 1);
        lcd.print(" ppm");

        lcd.setCursor(0, 1);
        lcd.print("MQ8 (H2): ");
        lcd.print(mq8PPM, 1);
        lcd.print(" ppm");

        lcd.setCursor(0, 2);
        lcd.print("MQ135: ");
        lcd.print(mq135PPM, 1);
        lcd.print(" ppm");
        break;

      case 2:
        // PM sensor data
        lcd.setCursor(0, 0);
        lcd.print("PMS5003 Readings:");
        lcd.setCursor(0, 1);
        lcd.print(pmData.substring(0, 20));
        if (pmData.length() > 20) {
          lcd.setCursor(0, 2);
          lcd.print(pmData.substring(20, 40));
        }
        break;
    }

    screen++;
    if (screen > 2) screen = 0;
  }

  delay(200);
}

// PMS5003 reading function
String readPMS() {
  if (pmsSerial.available() >= 32) {
    if (pmsSerial.read() == 0x42 && pmsSerial.read() == 0x4D) {
      pmsData[0] = 0x42;
      pmsData[1] = 0x4D;
      for (int i = 2; i < 32; i++) {
        pmsData[i] = pmsSerial.read();
      }

      uint16_t pm10_standard = (pmsData[10] << 8) | pmsData[11];
      uint16_t pm25_standard = (pmsData[12] << 8) | pmsData[13];
      uint16_t pm100_standard = (pmsData[14] << 8) | pmsData[15];

      return "PM1.0:" + String(pm10_standard) + " PM2.5:" + String(pm25_standard) + " PM10:" + String(pm100_standard);
    }
  }
  return "PM: N/A";
}
