# 🛰️ Chaakra Embedded Hardware Telematics Node (ESP32)

Production-ready firmware for the physical transit vehicle tracking node powering the **Chaakra Smart Bus Tracking System**. The node samples high-precision GPS NMEA telemetry via Hardware UART and dispatches live coordinate packets to Firebase Realtime Database over Wi-Fi / LTE gateway every 3–5 seconds.

---

## 📋 Hardware Bill of Materials (BOM)

| Component | Specification | Quantity | Purpose |
| :--- | :--- | :---: | :--- |
| **Microcontroller** | ESP32-WROOM-32 (NodeMCU 30/38 Pin) | 1 | Dual-core processing, hardware UART2, Wi-Fi stack |
| **GPS Module** | u-blox Neo-6M GPS Receiver | 1 | 50-channel positioning engine, 5Hz refresh |
| **Antenna** | 28 dB Active Ceramic Patch Antenna | 1 | High-gain GNSS signal reception |
| **Power Supply** | AMS1117 3.3V LDO / 5V Step-Down USB | 1 | Stable regulated supply for GPS & ESP32 |
| **Passives** | 100µF decoupling capacitor, jumper leads | - | Power stabilization and signal integrity |

---

## ⚡ Circuit Pinout & Interconnect Diagram

```
                 +-------------------+
                 | ESP32-WROOM-32    |
                 |                   |
                 |        GPIO 16/RX2|<------ (TX) Neo-6M GPS
                 |        GPIO 17/TX2|------> (RX) Neo-6M GPS (Optional cfg)
                 |                   |
                 |               3.3V|------> (VCC) Neo-6M GPS (3.3V Logic)
                 |                GND|------> (GND) Neo-6M GPS Common Ground
                 |                   |
                 |      GPIO 2 (D2)  |------> [Internal Status Diagnostic LED]
                 +-------------------+
```

### Signal Map

| ESP32 Pin | u-blox Neo-6M Pin | Function / Description |
| :--- | :--- | :--- |
| **GPIO 16 (RX2)** | **TXD** | Hardware UART2 Receive (reads `$GPRMC` & `$GPGGA` sentences) |
| **GPIO 17 (TX2)** | **RXD** | Hardware UART2 Transmit (configuration & baud negotiation) |
| **3V3** | **VCC** | 3.3V DC Regulated Power |
| **GND** | **GND** | System Common Ground |
| **GPIO 2** | Built-in LED | Visual telemetry diagnostic indicator |

---

## 🔄 Firmware Architecture & Execution Flow

```
   [ u-blox Neo-6M GPS ]
             │ (NMEA 9600 Baud UART Stream)
             ▼
   [ ESP32 HardwareSerial2 Buffer ]
             │
             ▼
   [ TinyGPS++ NMEA Sentence Decoder ]
             │ (Valid Fix: Lat, Lng, Speed, Sats, HDOP)
             ▼
   [ JSON Telemetry Packet Formatter ]
             │
             ▼
   [ HTTPClient REST Dispatcher ]
             │ (HTTPS PUT payload every 3s)
             ▼
   [ Firebase Realtime Database: /buses/{busId}/location ]
             │ (Dynamic WebSocket Listeners <5s latency)
             ▼
   [ React Native Mobile Client (Driver & Passenger Maps) ]
```

---

## 🛠️ Flashing & Setup Guide

### 1. Arduino IDE Setup
1. Install **Arduino IDE 2.x** or **PlatformIO**.
2. Open **Tools > Board > Boards Manager**, search for `esp32` by Espressif Systems and click **Install**.
3. Install required library dependencies:
   * **TinyGPSPlus** by Mikal Hart (via Library Manager: `Sketch > Include Library > Manage Libraries...`)
4. Copy `config.h.example` to `config.h`:
   ```bash
   cp config.h.example config.h
   ```
5. Edit `config.h` with your Wi-Fi credentials and Firebase endpoint:
   ```c
   #define WIFI_SSID       "Your_Bus_Gateway_SSID"
   #define WIFI_PASSWORD   "Your_Password"
   #define FIREBASE_HOST   "https://chakraa-bus-tracker-gps-default-rtdb.asia-southeast1.firebasedatabase.app"
   #define BUS_ID          "BUS_AMRITA_01"
   ```
6. Select Board: **ESP32 Dev Module** (or **DOIT ESP32 DEVKIT V1**).
7. Select your COM Port and click **Upload**.

---

## 💡 Diagnostic LED Indicators

* **Rapid Flash (200ms)**: Searching for Wi-Fi / Gateway network.
* **Slow Pulse (1000ms)**: Wi-Fi connected, acquiring satellite lock (ensure antenna has outdoor sky visibility).
* **Solid Blue**: GPS fix acquired, live telemetry actively dispatching to cloud.
* **Brief Blink Off**: ACK received from Firebase Realtime Database.
