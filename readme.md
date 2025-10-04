# 🚗 RC MegaBlast - Dual Motor RC Car with PS4 Controller (ESP32)

The project was created to bring back to life a toy found by the trash; the original electronics were removed, a 3D-printed enclosure with component mounts was made, and all parts were soldered using the cheapest approach with single-sided universal PCBs.

![s-l1200](https://github.com/user-attachments/assets/40be38f9-3fd7-4220-8dcf-e6e5ef741ffc)

Remote-controlled car project using **ESP32**, **dual TB6612FNG motors**, and **PS4 controller** via Bluetooth. Includes LEDs, audio feedback, and safe battery management.

---
## 🔧 Key Features

- 🎮 **Analog throttle & brake** – smooth control with R2 / L2
- 🕹️ **Steering** – precise with left analog stick
- 💡 **Full LED system** – headlights, tail lights, brake, indicators, auxiliary lights
- 🔊 **Audio feedback** – horn, engine start/stop, blinkers
- ✨ **PS4 controller feedback** – rumble & LED effects
- 🔋 **Battery management** – 3S Li-Ion pack charging & monitoring
- 🛑 **Safe motor control** – coasting and braking handled reliably
- 🎮 **Universal controller support** – works with pads from any console, simple pairing
---

## 🧩 Hardware Components

| Component | Description |
|-----------|-------------|
| ESP32 DevKit v4 | MCU with WiFi and Bluetooth |
| 3x 18650 Li-Ion cells (3S) | Main battery pack |
| BMS 3S 40A | Battery protection and balancing |
| USB-C PD Trigger Board | Optional 12V input for charging |
| TB6612FNG Motor Driver | Dual DC motor control |
| LEDs + 2N5551 transistors | Lights and indicators |
| MAX98357A / PCM5102A | I2S audio module (engine sounds, horn) |
| Optional Battery Monitor | E.g. 2S–8S Li-Ion/LiFePO4 voltage indicator |

---

## ⚡ Power Architecture

- 3S Li-Ion battery pack (~11.1–12.6V)
- BMS 3S 40A for protection and balancing
- Optional USB-C PD trigger for 12V input
- Step-down regulators (if needed for ESP32 or peripherals)
- 🔜 Battery monitoring module planned to track voltage and charge state

---

## 🚦 Control Logic

- **Throttle**: R2 trigger (0–255)
- **Brake**: L2 button
- **Steering**: Left stick X-axis
- **LED Feedback**: GPIO-controlled headlights, tail lights, brake, and blinkers
- **Audio Feedback**: Horn, engine start/stop, blinker sounds
- **Haptic Feedback**: PS4 controller rumble
- **Drive Modes**: Forward, reverse, idle/coast  

---

## 🛠️ Development

- Written in C++ with **PlatformIO**
- Motor logic in `MotorController` with safe running state
- Light control in `LightLedController` (FreeRTOS task for blinkers)
- PS4 input via `BluePad32Controller`
- Audio feedback simplified via `AudioClipController`  

---

## 💪 Project Status

- ✅ Hardware tested and running
- ✅ Motor and LED logic functional
- ✅ PS4 controller input and rumble working
- ✅ Audio tracks refinement working
- ⚠️ Battery monitoring and full PD integration in progress  

---

## 🔍 Keywords

ESP32 RC Car, PS4 Controller, TB6612FNG Motor Driver, 18650 Li-Ion Battery, I2S Audio, PlatformIO ESP32 RC  

---

## 📄 License

MIT License
