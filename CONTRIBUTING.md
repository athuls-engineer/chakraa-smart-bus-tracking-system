# Contributing to Chakraa

Thank you for your interest in contributing to the **Chakraa Smart Bus Tracking System**!

## Development Guidelines
1. Fork the repository and create a descriptive feature branch (`git checkout -b feature/telemetry-optimization`).
2. Verify TypeScript compilation on backend changes before committing:
   ```bash
   cd server
   npm run build
   ```
3. Ensure ESP32 firmware adheres to non-blocking loop conventions (millis-based timers instead of `delay()`).
4. Submit a Pull Request targeting the `main` branch with a clear description of your changes.
