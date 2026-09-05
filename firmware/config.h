#ifndef CONFIG_H
#define CONFIG_H

// =========================================================================
// CHAAKRA IOT BUS TRACKING SYSTEM - HARDWARE TELEMATICS CONFIGURATION
// =========================================================================

// Wi-Fi Network Credentials (Bus Cellular Gateway or Depot Access Point)
#define WIFI_SSID         "YOUR_WIFI_OR_MOBILE_HOTSPOT"
#define WIFI_PASSWORD     "YOUR_WIFI_PASSWORD"

// Firebase Realtime Database Configuration
#define FIREBASE_HOST     "https://chakraa-bus-tracker-gps-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH     "YOUR_FIREBASE_DATABASE_SECRET_OR_WEB_API_KEY"

// Bus Identifier (Matches the bus ID in the React Native / Admin dashboard)
#define BUS_ID            "BUS_AMRITA_01"

// Telemetry Timing Parameters
#define TELEMETRY_INTERVAL_MS   3000   // Telemetry push cadence (3 seconds)
#define GPS_BAUD_RATE           9600   // Standard NMEA baud rate for u-blox Neo-6M

// Hardware Pin Definitions (ESP32 NodeMCU-32S)
#define GPS_RX_PIN        16     // ESP32 GPIO 16 (UART2 RX) -> Neo-6M TX
#define GPS_TX_PIN        17     // ESP32 GPIO 17 (UART2 TX) -> Neo-6M RX
#define STATUS_LED_PIN    2      // Built-in blue LED for hardware diagnostics

#endif // CONFIG_H
