# RC MegaBlast - Dual Motor Control System with PS4 Controller (ESP32)

This is an open-source project for building a remote-controlled vehicle using an **ESP32 DevKit v4**, dual-motor driver (**TB6612FNG**), and a **PS4 controller** via Bluetooth. The system is powered by **3x 18650 Li-Ion cells** with charging and power protection modules.

## 🔧 Features

- Full-speed motor control using analog trigger (R2)
- Directional steering using analog stick (left/right)
- Brake function with R1
- LED control for headlights and position lights
- PS4 controller rumble and LED feedback
- Safe power delivery and battery charging

## 🧩 Hardware Components

| Component                                                                               | Description                                  |
| --------------------------------------------------------------------------------------- | -------------------------------------------- |
| [ESP32 DevKit v4](https://www.espressif.com/en/products/devkits/esp32-devkitc/overview) | Main microcontroller with Bluetooth and WiFi |
| [3x 18650 Li-Ion cells](https://pl.aliexpress.com/item/1005008533683033.html)           | Battery pack powering the system             |
| TP4056 (with protection) module                                                         | Battery charging and protection              |
| TB6612FNG Motor Driver                                                                  | Dual channel motor driver (2 DC motors)      |
| MP2307 DC-DC Mini 360                                                                   | Step-down voltage regulator                  |
| RUEF300 PTC Resettable Fuse                                                             | 3A 30V overcurrent protection                |
| PS4 Controller                                                                          | Remote control via Bluetooth                 |
| LEDs + 2N5551 transistors                                                               | For lights and visual feedback               |

## ⚡ Power Architecture

- Battery pack: 3x 18650 (approx. 11.1–12.6V when fully charged)
- Step-down regulator (MP2307) outputs 5V to power the ESP32
- TB6612FNG powered directly from battery voltage (up to 15V supported)
- TP4056 module ensures safe charging via USB
- RUEF300 fuse protects the system from overcurrent and shorts

## 🚗 Control Logic

- **Throttle**: R2 trigger (0–255)
- **Brake**: L2 button
- **Steering**: Left analog stick (X-axis)
- **LED Feedback**: Headlights and tail lights via GPIO
- **Haptic Feedback**: Rumble motors and LED color via PS4 controller
- **Drive Modes**: Forward, reverse, and idle (coasting)

## 💪 Project Status

- ✅ Hardware tested and working
- ✅ Basic PS4 input integrated
- ✅ Motor logic implemented
- ✅ LED and rumble feedback functional
- ⚠️ Audio module (PCM5102A / MAX98357A) in planning
- ⚠️ Unit test integration with PlatformIO ongoing

## 🛠️ Development

The firmware is written in C++ and built using **PlatformIO**.

## 🧪 Unit Testing

Using PlatformIO's Unity test framework to validate:

- Input normalization (analog stick, trigger)
- Motor speed calculations
- Safe brake and coast behavior

## 🔍 Keywords

ESP32 RC Car, PS4 Controller ESP32, TB6612FNG Motor Driver, 18650 ESP32 Power, MAX98357A I2S Audio, PlatformIO ESP32 Car, Remote Controlled Car Project

## 📄 License

MIT License

