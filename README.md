# Aquaponics System

## Overview
This project is an IoT-based aquaponics system that monitors and controls environmental conditions (e.g., water temperature, pH, soil moisture, etc.) using a combination of Raspberry Pi, Arduino, and various sensors. The collected data is stored in a MariaDB/MySQL database, and you can monitor the system through a mobile app.

### Features:
- Real-time monitoring of temperature, humidity, water pH, TDS, and soil moisture
- Automated water pump control based on soil moisture sensor readings
- LED lights for plant growth, controlled by a time schedule
- Mobile app interface for logging in, monitoring data, and receiving security alerts via WhatsApp

---

## Materials Used

### Hardware:
1. **Raspberry Pi 5**
   - Acts as the central server and database host.
2. **Arduino Mega 2560**
   - Interacts with sensors and relays for real-time data collection.
3. **DHT22 Sensor**
   - Measures air temperature and humidity.
4. **DS18B20 Sensor**
   - Measures water temperature.
5. **TDS Sensor**
   - Monitors total dissolved solids in the water.
6. **pH Sensor**
   - Measures water pH levels.
7. **Soil Moisture Sensor**
   - Monitors soil moisture levels to control the water pump.
8. **5V Relay Module**
   - Controls the water pump (DC or AC pump depending on your setup).
9. **Water Pump**
   - For watering the plants based on soil moisture readings.
10. **LED Strip (WS2812B)**
    - LED lighting for plant growth, with timed control.

### Software:
1. **Raspberry Pi OS (64-bit)**
2. **Arduino IDE** (for programming the Arduino)
3. **MariaDB/MySQL**
   - Database to store sensor data.
4. **Node.js**
   - Backend server running on Raspberry Pi for data handling.
5. **Flask (Python)**
   - Provides a simple API to interact with the mobile app.
6. **Swift (for iOS app development)**
   - Mobile app for monitoring sensor data and login security.

---

## How It Works

### 1. **System Architecture**
   - The **Arduino** collects real-time data from the sensors.
   - Data is sent to the **Raspberry Pi** via serial communication.
   - The Raspberry Pi runs a **Node.js server** to process the sensor data and stores it in a **MySQL/MariaDB** database.
   - A **Flask-based API** allows data retrieval and WhatsApp alerts through Twilio.
   - The **iOS app** allows users to log in, monitor sensor data, and receive alerts.

### 2. **Automation and Control**
   - **Water Pump Control**: Based on soil moisture readings, the Raspberry Pi instructs the pump to turn on or off using a relay.
   - **LED Control**: The LED strip is controlled by a timed schedule (turns off from 12 AM to 6 AM).
   - **Security Code via WhatsApp**: If too many incorrect login attempts occur, a security code is sent to the user's phone via WhatsApp.

---

## Setup Instructions

### 1. **Hardware Setup**
   - Connect the sensors (DHT22, DS18B20, pH sensor, TDS sensor, soil moisture sensor) to the Arduino Mega.
   - Connect the Arduino to the Raspberry Pi via USB.
   - Set up the relay module to control the water pump.

### 2. **Raspberry Pi Setup**
   - Install **Node.js**:
     ```bash
     sudo apt update
     sudo apt install nodejs
     ```
   - Install **MariaDB** and configure the database:
     ```bash
     sudo apt install mariadb-server
     sudo mysql_secure_installation
     ```
   - Create the database and tables:
     ```sql
     CREATE DATABASE aquaponics;
     USE aquaponics;
     CREATE TABLE sensor_data (
         id INT AUTO_INCREMENT PRIMARY KEY,
         temperature FLOAT,
         water_temperature FLOAT,
         humidity FLOAT,
         pH FLOAT,
         soil_moisture FLOAT,
         TDS FLOAT,
         timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP
     );
     ```

### 3. **Arduino Setup**
   - Program the **Arduino Mega 2560** using the Arduino IDE with the sensors and pump control logic.
   - Include the necessary libraries like `DHT`, `OneWire`, and `FastLED`.

### 4. **Backend Setup (Raspberry Pi)**
   - Install necessary Node.js modules:
     ```bash
     npm install express mysql serialport
     ```
   - Set up the **Flask API** for the mobile app:
     ```bash
     pip install Flask twilio python-dotenv
     ```

### 5. **Mobile App (iOS)**
   - The mobile app is built using SwiftUI, allowing users to log in, view sensor data, and receive security codes via WhatsApp.

### 6. **WhatsApp Security Code Setup**
   - Use Twilio's WhatsApp API to send a security code to users when they attempt to log in after multiple failed login attempts.

---

## Usage

1. **Run the Node.js server**:
  ```bash
   node server.js
2. **Run the Flask API**:
  ```bash
   python app.py
3. **Monitor the data through the mobile app or check directly on the Raspberry Pi using**:
   ```bash
   curl http://localhost:3000/sensor-data

Future Enhancements
Implement remote access to the system.
Add support for multiple users with different access levels.
Expand the mobile app to include additional control features (e.g., manual control of the pump and lights).

## Contact
Munghoon Cho
Email: mcho36@asu.edu

Rina Kawamura
Email: rkawamu1@asu.edu
