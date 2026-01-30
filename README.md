# ESP32 Humidity Monitor with AWS + Telegram Alerts

An IoT humidity/temperature monitoring system built with an ESP32 and an AHT20 sensor.
The device indicates comfort level using onboard LEDs (Green/Yellow/Red) and sends
status updates to a Flask server hosted on AWS, which forwards notifications to Telegram.

---

## Features
- Reads temperature and humidity using **AHT20**
- Computes a **comfort level** (Good/Fair/Bad) based on threshold rules
- Displays status via **RGB-style LED indicators**
- Sends HTTP updates to an **AWS-hosted Flask server**
- Server pushes notifications to **Telegram** (only on state changes)

---

## System Architecture
1. **ESP32** reads sensor data every few seconds
2. ESP32 determines comfort level and updates LEDs
3. When the comfort level changes, ESP32 sends an HTTP GET request to the server
4. **Flask server** receives the update and sends a Telegram message

---

## Comfort Level Rules
- **Bad** if temperature is outside `10–30°C` or humidity is outside `30–70%`
- **Good** if temperature is within `20–26°C` and humidity within `40–60%`
- Otherwise **Fair**

---

## Repository Structure
```text
.
├──  main.cpp            # ESP32 firmware (sensor + LEDs + HTTP client)
├──  server.py           # Flask server (receives updates, sends Telegram)
├──  docs                # photos
├──  .gitignore            
└──  README.md
