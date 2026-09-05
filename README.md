![GitHub repo size](https://img.shields.io/github/repo-size/athuls-engineer/chakraa-smart-bus-tracking-system)

# 🚌 Chakraa - Smart Bus Tracking System

A **full-stack real-time bus tracking solution** that integrates IoT, cloud backend, and mobile applications to provide live bus location tracking and enhanced user experience.

---

## 📌 Project Overview

**Chakraa** is a smart transportation system that enables real-time tracking of buses using an **ESP32-based GPS module**, a **Firebase-powered backend**, and **mobile applications for users and drivers**.

It enhances public transport usability by combining **live tracking, route visibility, and smart features like payments and dashboards**.

---

## 🧰 Tech Stack

* **Frontend:** React Native
* **Backend:** Node.js / Express
* **Database:** Firebase Realtime Database
* **Hardware:** ESP32 + GPS Module
* **Payments:** Razorpay

---

## ⚙️ Features

* 📍 Real-time bus location tracking
* 🧑‍✈️ Driver dashboard for ride management
* 📱 User app with search and tracking interface
* 🔍 Smart search functionality for routes
* 💳 Secure payment integration (Razorpay)
* 🔥 Firebase real-time database updates
* 🌐 Cloud-based backend system
* 🔔 Notification support (if implemented)

---

## 🏗️ System Architecture

* **Hardware Layer:** ESP32 + GPS Module (data collection)
* **Backend Layer:** Node.js / Express (API + processing)
* **Database Layer:** Firebase Realtime Database
* **Frontend Layer:** React Native apps (User + Driver)

---

## 📂 Project Structure

```
chakraa-smart-bus-tracking-system/
│── firmware/      # ESP32 C++ firmware, hardware pinout & circuit schematics
│── user/          # User passenger mobile application (React Native)
│── driver/        # Driver transit mobile application (React Native)
│── server/        # Backend server (Node.js/Express, Prisma ORM, Razorpay)
│── socket/        # Socket.io real-time broadcast engine
│── README.md
```

---

## 🛰️ Embedded Hardware Node (ESP32 + u-blox Neo-6M)

The onboard telematics unit deployed on each fleet vehicle runs autonomous bare-metal C++ firmware on an **ESP32-WROOM-32**:

* **Complete Firmware & Guide**: See [**`firmware/`**](firmware/) for full source code (`esp32_gps_tracker.ino`), pinout connection tables, and flashing instructions.
* **Hardware UART Stream**: Decodes live NMEA sentences via `TinyGPSPlus` on dedicated HardwareSerial2 (GPIO 16 RX / GPIO 17 TX) at 9600 baud.
* **Sub-5s Cloud Sync**: Directly formats and transmits JSON coordinate telemetry (`latitude`, `longitude`, `speed_kmh`, `altitude`, `satellites`, `hdop`) to Firebase Realtime Database (`/buses/{busId}/location`) every 3 seconds.
* **Fault-Tolerant Link**: Includes non-blocking auto-reconnect logic with exponential backoff for fluctuating cellular / Wi-Fi signals.
* **Hardware Diagnostics**: Onboard LED provides visual states for network acquisition, GPS satellite fix, and telemetry packet ACKs.

---

## 🚀 How It Works

1. 📡 GPS module collects real-time coordinates
2. 📶 ESP32 sends data to Firebase
3. ⚙️ Backend processes and updates the database
4. 📱 User app fetches and displays live bus location
5. 💳 Payment system handles secure transactions

---

## 🛠️ Installation & Setup

### 1️⃣ Clone the repository

```
git clone https://github.com/athuls-engineer/chakraa-smart-bus-tracking-system.git
```

### 2️⃣ Install dependencies

```
cd server
npm install
```

### 3️⃣ Run the server

```
npm start
```

---

## 🔐 Environment Variables

Create a `.env` file in the required directories and add:

```
FIREBASE_API_KEY=your_api_key_here
DATABASE_URL=your_database_url_here
PAYMENT_API_KEY=your_payment_key_here
```

> ⚠️ Never commit `.env` files to GitHub. Always keep API keys secure.

---

## 📸 Screenshots

<img width="401" height="857" alt="User App Working" src="https://github.com/user-attachments/assets/da313809-f36a-4de4-9fa7-cd67faf09932" />

<img width="467" height="861" alt="Driver App Working" src="https://github.com/user-attachments/assets/ff0b9082-d8fb-432d-bc62-82cc1682b7e8" />

---

## 🎯 Future Improvements

* 🤖 AI-based route prediction
* ⏱️ Traffic-based ETA estimation
* 🎨 Enhanced UI/UX
* 📡 Offline tracking capabilities
* 📊 Advanced analytics dashboard

---

## 👨‍💻 Contributors

* [Athul S](https://github.com/athuls-engineer)
* M. Varshith Reddy
* Rithin Ranjith
* Noaf N
* Nived S

---

## 📜 License

This project is developed for **academic purposes**.

---

## 💡 Acknowledgements

* Firebase
* ESP32 Community
* Razorpay
* Open-source libraries

---
