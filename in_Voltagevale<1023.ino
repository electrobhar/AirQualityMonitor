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

// DHT Sensor
#define DHTTYPE DHT11
DHT dht(dhtPin, DHTTYPE);

// Serial Interfaces
SoftwareSerial bluetooth(btRX, btTX);
SoftwareSerial pmsSerial(pmsRX, pmsTX);

// PMS5003 Data Buffer
uint8_t pmsData[32];

// Initialize LCD: 20 columns, 4 rows
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
  int mq8Value = analogRead(mq8Pin);
  int mq135Value = analogRead(mq135Pin);
  int mq7Value = analogRead(mq7Pin);

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  String pmData = readPMS();

  String dataToSend = "MQ8: " + String(mq8Value) +
                      " | MQ135: " + String(mq135Value) +
                      " | MQ7: " + String(mq7Value) +
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
        // Temperature and Humidity
        lcd.setCursor(0, 0);
        lcd.print("Temp: ");
        lcd.print(temperature, 1);
        lcd.print((char)223); // Degree symbol
        lcd.print("C");

        lcd.setCursor(0, 1);
        lcd.print("Humidity: ");
        lcd.print(humidity, 1);
        lcd.print("%");

        lcd.setCursor(0, 2);
        lcd.print("MQ8: ");
        lcd.print(mq8Value);
        lcd.setCursor(10, 2);
        lcd.print("MQ7: ");
        lcd.print(mq7Value);

        lcd.setCursor(0, 3);
        lcd.print("MQ135: ");
        lcd.print(mq135Value);
        break;

      case 1:
        // PM data
        lcd.setCursor(0, 0);
        lcd.print("PMS5003 Readings:");

        lcd.setCursor(0, 1);
        lcd.print(pmData.substring(0, 20)); // PM1.0 and PM2.5

        if (pmData.length() > 20) {
          lcd.setCursor(0, 2);
          lcd.print(pmData.substring(20, 40)); // PM10 or rest
        }

        lcd.setCursor(0, 3);
        lcd.print("Reading updated...");
        break;
    }

    screen++;
    if (screen > 1) screen = 0;
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
      delay(4000); //make some delay to getting enough time to accurately work the sensor

      return "PM1.0:" + String(pm10_standard) + " PM2.5:" + String(pm25_standard) + " PM10:" + String(pm100_standard);
    }
  }
  return "PM: N/A";
}
