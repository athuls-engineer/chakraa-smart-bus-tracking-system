/*
 * =============================================================================
 * CHAAKRA: IoT Real-Time Fleet GPS Telematics Node
 * =============================================================================
 * Hardware Architecture: ESP32-WROOM-32 + u-blox Neo-6M GPS Module
 * Communication: HardwareSerial (UART2) + Wi-Fi / LTE Gateway + Firebase RTDB
 * Author: Athul S (https://github.com/athuls-engineer)
 * Repository: https://github.com/athuls-engineer/chakraa-smart-bus-tracking-system
 * =============================================================================
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <HardwareSerial.h>
#include <TinyGPSPlus.h>
#include "config.h"

// TinyGPS++ instance for NMEA decoding
TinyGPSPlus gps;

// Dedicated Hardware UART2 for GPS communication (avoids Serial0 USB conflicts)
HardwareSerial gpsSerial(2);

// Telemetry State Variables
unsigned long lastTelemetryDispatch = 0;
unsigned long lastLedToggle = 0;
bool ledState = false;
int packetSequence = 0;

// Function Prototypes
void connectToWiFi();
void checkWiFiConnection();
void processGpsStream();
bool dispatchTelemetryPacket(double lat, double lng, float speedKmh, float altitudeM, int sats, float hdop);
void handleDiagnosticLed();

void setup() {
  // Initialize USB Serial for debug monitoring
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  Serial.println(F("=================================================="));
  Serial.println(F("  CHAAKRA FLEET TELEMATICS NODE - INITIALIZING    "));
  Serial.println(F("=================================================="));

  // Initialize Diagnostic Status LED
  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, LOW);

  // Initialize Hardware UART2 for Neo-6M GPS
  // GPIO 16 = RX2 (receives from Neo-6M TX)
  // GPIO 17 = TX2 (transmits to Neo-6M RX)
  gpsSerial.begin(GPS_BAUD_RATE, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  Serial.printf("[HARDWARE] GPS UART2 initialized on RX=GPIO%d, TX=GPIO%d at %d baud\n",
                GPS_RX_PIN, GPS_TX_PIN, GPS_BAUD_RATE);

  // Connect to Wi-Fi / Mobile Telematics Gateway
  connectToWiFi();

  Serial.println(F("[STATUS] Node initialization complete. Listening for NMEA sentences..."));
}

void loop() {
  // 1. Maintain Wi-Fi connectivity with auto-reconnect
  checkWiFiConnection();

  // 2. Continuous non-blocking ingestion of NMEA stream from Neo-6M
  processGpsStream();

  // 3. Periodic telemetry dispatch to Firebase Realtime Database
  unsigned long currentMillis = millis();
  if (currentMillis - lastTelemetryDispatch >= TELEMETRY_INTERVAL_MS) {
    lastTelemetryDispatch = currentMillis;

    if (gps.location.isValid()) {
      double latitude = gps.location.lat();
      double longitude = gps.location.lng();
      float speedKmh = gps.speed.isValid() ? gps.speed.kmph() : 0.0f;
      float altitudeM = gps.altitude.isValid() ? gps.altitude.meters() : 0.0f;
      int satellites = gps.satellites.isValid() ? gps.satellites.value() : 0;
      float hdop = gps.hdop.isValid() ? gps.hdop.hdop() : 99.9f;

      Serial.printf("[GPS FIX] Lat: %.6f, Lng: %.6f | Speed: %.1f km/h | Sats: %d | HDOP: %.1f\n",
                    latitude, longitude, speedKmh, satellites, hdop);

      // Dispatch payload to Firebase REST endpoint
      bool success = dispatchTelemetryPacket(latitude, longitude, speedKmh, altitudeM, satellites, hdop);
      if (success) {
        packetSequence++;
      }
    } else {
      Serial.printf("[GPS SEARCH] Awaiting valid fix... Characters processed: %lu, Checksum failures: %lu\n",
                    gps.charsProcessed(), gps.failedChecksum());
    }
  }

  // 4. Update status LED pattern based on operational mode
  handleDiagnosticLed();
}

/**
 * Ingests incoming NMEA characters from hardware UART buffer into TinyGPS++
 */
void processGpsStream() {
  while (gpsSerial.available() > 0) {
    char c = gpsSerial.read();
    gps.encode(c);
  }
}

/**
 * Dispatches coordinates and fleet telemetry directly to Firebase Realtime Database
 */
bool dispatchTelemetryPacket(double lat, double lng, float speedKmh, float altitudeM, int sats, float hdop) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(F("[ERROR] Wi-Fi link down. Skipping telemetry dispatch."));
    return false;
  }

  HTTPClient http;
  String endpoint = String(FIREBASE_HOST) + "/buses/" + String(BUS_ID) + "/location.json";
  
  if (String(FIREBASE_AUTH).length() > 0) {
    endpoint += "?auth=" + String(FIREBASE_AUTH);
  }

  http.begin(endpoint);
  http.addHeader("Content-Type", "application/json");

  // Construct JSON payload matching React Native schema
  String payload = "{";
  payload += "\"latitude\":" + String(lat, 6) + ",";
  payload += "\"longitude\":" + String(lng, 6) + ",";
  payload += "\"speed\":" + String(speedKmh, 1) + ",";
  payload += "\"altitude\":" + String(altitudeM, 1) + ",";
  payload += "\"satellites\":" + String(sats) + ",";
  payload += "\"hdop\":" + String(hdop, 1) + ",";
  payload += "\"sequence\":" + String(packetSequence) + ",";
  payload += "\"timestamp\":{\".sv\":\"timestamp\"}"; // Firebase ServerValue.TIMESTAMP
  payload += "}";

  int httpCode = http.PUT(payload);

  if (httpCode == HTTP_CODE_OK || httpCode == 204) {
    Serial.printf("[FIREBASE] Telemetry ACK #%d delivered successfully (HTTP %d)\n", packetSequence, httpCode);
    
    // Quick pulse on status LED to confirm data transit
    digitalWrite(STATUS_LED_PIN, LOW);
    delay(40);
    digitalWrite(STATUS_LED_PIN, HIGH);
    
    http.end();
    return true;
  } else {
    Serial.printf("[FIREBASE ERROR] Dispatch failed, HTTP code: %d, Response: %s\n", 
                  httpCode, http.getString().c_str());
    http.end();
    return false;
  }
}

/**
 * Establishes initial connection to local Wi-Fi / LTE telematics gateway
 */
void connectToWiFi() {
  Serial.printf("[WIFI] Connecting to SSID: %s", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 25) {
    delay(400);
    Serial.print(F("."));
    digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN)); // Fast blink during connection
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.printf("[WIFI] Connected! Assigned IP: %s (RSSI: %d dBm)\n",
                  WiFi.localIP().toString().c_str(), WiFi.RSSI());
    digitalWrite(STATUS_LED_PIN, HIGH);
  } else {
    Serial.println();
    Serial.println(F("[WIFI WARNING] Gateway connection timed out. Will retry in background."));
  }
}

/**
 * Non-blocking reconnection watchdog
 */
void checkWiFiConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    static unsigned long lastReconnectAttempt = 0;
    if (millis() - lastReconnectAttempt > 10000) {
      lastReconnectAttempt = millis();
      Serial.println(F("[WIFI] Re-establishing dropped gateway link..."));
      WiFi.disconnect();
      WiFi.reconnect();
    }
  }
}

/**
 * Visual diagnostic LED state controller
 * - Fast Blink (200ms): Wi-Fi connecting
 * - Slow Blink (1000ms): Wi-Fi OK, awaiting GPS satellite fix
 * - Solid ON: GPS lock acquired & tracking active
 */
void handleDiagnosticLed() {
  unsigned long now = millis();

  if (WiFi.status() != WL_CONNECTED) {
    // Wi-Fi offline -> Fast blink
    if (now - lastLedToggle >= 200) {
      lastLedToggle = now;
      ledState = !ledState;
      digitalWrite(STATUS_LED_PIN, ledState);
    }
  } else if (!gps.location.isValid()) {
    // Wi-Fi connected, but searching for GPS satellites -> Slow pulse
    if (now - lastLedToggle >= 1000) {
      lastLedToggle = now;
      ledState = !ledState;
      digitalWrite(STATUS_LED_PIN, ledState);
    }
  } else {
    // Valid GPS lock & connected -> Solid active
    digitalWrite(STATUS_LED_PIN, HIGH);
  }
}
