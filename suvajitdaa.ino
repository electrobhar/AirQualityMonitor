#include <SoftwareSerial.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Pin Definitions
const int mq8Pin = A0;
const int mq135Pin = A2;
const int mq7Pin = A1;
const int dhtPin = 7;            // DHT11 data pin
const int pmsRX = 8;             // PMS5003 TX -> Arduino RX
const int pmsTX = 9;             // PMS5003 RX -> Arduino TX
const int btRX = 10;             // HC-05 TX -> Arduino RX
const int btTX = 11;             // HC-05 RX -> Arduino TX

// DHT Sensor setup
#define DHTTYPE DHT11
DHT dht(dhtPin, DHTTYPE);

// Bluetooth and PMS5003 Serial
SoftwareSerial bluetooth(btRX, btTX);
SoftwareSerial pmsSerial(pmsRX, pmsTX); // RX, TX

// PMS5003 Data Buffer
uint8_t pmsData[32];

// Initialize LCD, common I2C address is 0x27, 16 columns, 2 rows
LiquidCrystal_I2C lcd(0x27, 16, 2);

unsigned long previousMillis = 0;
const long interval = 4000; // 4 seconds between screen switches
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
  lcd.print("System Initialized");
  delay(2000);
  lcd.clear();
}

void loop() {
  // Read gas sensors
  int mq8Value = analogRead(mq8Pin);
  int mq135Value = analogRead(mq135Pin);
  int mq7Value = analogRead(mq7Pin);

  // Read DHT11
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Read PMS5003 data
  String pmData = readPMS();

  // Format all data
  String dataToSend = "MQ8: " + String(mq8Value) +
                      " | MQ135: " + String(mq135Value) +
                      " | MQ7: " + String(mq7Value) +
                      " | Temp: " + String(temperature) + "C" +
                      " | Humidity: " + String(humidity) + "%" +
                      " | " + pmData;

  // Send data via Serial and Bluetooth
  Serial.println(dataToSend);
  bluetooth.println(dataToSend);

  // Update LCD display with rotating info every 'interval' milliseconds
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    lcd.clear();

    switch(screen) {
      case 0:
        // Show temperature & humidity
        lcd.setCursor(0, 0);
        lcd.print("Temp: ");
        lcd.print(temperature, 1);
        lcd.print(" C");
        lcd.setCursor(0, 1);
        lcd.print("Humidity: ");
        lcd.print(humidity, 1);
        lcd.print("%");
        break;

      case 1:
        // Show gas sensor values
        lcd.setCursor(0, 0);
        lcd.print("MQ8:");
        lcd.print(mq8Value);
        lcd.print(" MQ135:");
        // To fit in line, abbreviate MQ135 as M135
        lcd.setCursor(0, 1);
        lcd.print("MQ7:");
        lcd.print(mq7Value);
        break;

      case 2:
        // Show particulate matter data
        // Example: PM1.0: 12 PM2.5: 20
        lcd.setCursor(0, 0);
        lcd.print(pmData.substring(0,16)); // first 16 chars
        lcd.setCursor(0, 1);
        if (pmData.length() > 16) {
          lcd.print(pmData.substring(16,32)); // next 16 chars if available
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
 