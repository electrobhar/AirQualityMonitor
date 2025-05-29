Air Quality Monitoring System using Arduino

This project is a compact and low-cost air quality monitoring system built with Arduino. It integrates multiple gas and environmental sensors to measure CO, H2, CO2, particulate matter (PM1.0, PM2.5, PM10), temperature, and humidity in real-time. The data is displayed on a 20x4 I2C LCD and sent via Bluetooth for remote monitoring.

Features

**Gas Monitoring**
  - CO (MQ-7)
  - Hydrogen (H2) via MQ-8
  - CO2 / Air Quality via MQ-135
**Particulate Matter**
  - PM1.0, PM2.5, and PM10 (PMS5003 Sensor)
**Environmental Readings**
  - Temperature and Humidity (DHT11)
**LCD Display**
  - 20x4 I2C LCD cycles through 3 different display screens
**Bluetooth Transmission**
  - Sends real-time data to any Bluetooth-enabled device (via HC-05/06)

##Components Used

  Component              Quantity 

Arduino Uno/Nano     ----- 1        
MQ-7 Gas Sensor      ----- 1        |
MQ-8 Gas Sensor      ----- 1      
MQ-135 Sensor        ----- 1      
PMS5003 Sensor       ----- 1      
DHT11 Sensor          -----1        
I2C 20x4 LCD         ----- 1        
Bluetooth Module (HC-05/06)  1 
Jumper Wires            XX
Breadboard           ----- 1        



