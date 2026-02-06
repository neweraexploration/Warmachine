# War Machine - DIY Robotic Turret (ESP32 + ESP32-CAM)

This repo contains the firmware and project materials for my first robotics build: a WiFi-controlled, ESP32-based robotic defense turret. It started as a curiosity project and quickly turned into a crash course in embedded systems, DC motor control, and safety-first engineering.

Video: https://youtu.be/zob-NRF-iT8

## Safety Disclaimer
This is an educational robotics project using toy components.
It is not a weapon. Do not try to recreate this.

## Project Highlights
- WiFi control with a custom web UI (ESP32 Async WebServer + WebSocket)
- Dual camera feeds via ESP32-CAM streams
- ESP-NOW device-to-device signaling
- Guard mode using an ultrasonic distance sensor
- Map mode (record + replay movements with timing)
- Auto-home sequence with limit switches

## System Architecture
- `War Machine (ESP32)`: main controller, motors, web UI, guard mode, map mode
- `War-Eye (ESP32-CAM)`: camera feed + ESP-NOW messages
- `ESP-NOW link`: event signaling using simple message codes (`"1001"`, `"1002"`)

## Hardware (Core Parts)
- ESP32 (main controller)
- ESP32-CAM (camera stream)
- DC motors + H-bridge driver
- Ultrasonic sensor (HC-SR04 or equivalent)
- Limit switches (base/arm home)
- Servo (trigger/actuation)
- PVC mechanical frame, wiring, and power system

## Firmware Layout
- `Codes/code_warmachine_main/code_warmachine_main.ino`: main controller
- `Codes/espNOW/espnow-eye/espnow-eye.ino`: ESP32-CAM ESP-NOW listener
- `Codes/espNOW/espsnow-war/espsnow-war.ino`: ESP32 ESP-NOW listener

## Pin Mapping (Main Controller)
These are defined in `code_warmachine_main.ino`:
- Ultrasonic: `trigPin=2`, `echoPin=4`
- Home switches: `basehomebtn=25`, `armhomebtn=21`
- Base servo: `basehomeservo_PIN=26`
- Trigger/actuator servo: `gun_SERVO_PIN=27`
- Right motor: `EN=22`, `IN1=16`, `IN2=17`
- Left motor: `EN=23`, `IN1=18`, `IN2=19`

## WiFi + UI
The main controller starts a SoftAP:
- SSID: `WARMACHINE`
- Password: `warmachine1234`

Open the UI at `http://192.168.4.1/` (default ESP32 SoftAP IP).  
The UI includes two camera slots with default IPs:
- War-Eye: `192.168.4.2`
- ESP-Eye: `192.168.4.3`

You can edit these IPs directly in the UI if your camera stream address differs.

## Setup (Quick Start)
1. Install Arduino IDE and ESP32 board support.
2. Install libraries:
   - `ESPAsyncWebServer`
   - `AsyncTCP`
   - `ESP32Servo`
3. Flash firmware:
   - Main ESP32: `Codes/code_warmachine_main/code_warmachine_main.ino`
   - ESP32-CAM: `Codes/espNOW/espnow-eye/espnow-eye.ino`
   - Secondary ESP32 (optional): `Codes/espNOW/espsnow-war/espsnow-war.ino`
4. Power on the system and connect to `WARMACHINE`.
5. Open `http://192.168.4.1/` to access the control UI.

## Operating Modes
- **Control Mode**: manual directional movement and trigger control
- **Map Mode**: record steps with timing and replay them with "Engage"
- **Guard Mode**: uses distance readings and ESP-NOW alerts
- **Auto-home**: return to base using limit switches

## Media + Report
Project report and photos are in `Report And media/`.

## Credits
The `Resources(Code i used from internet)/` folder contains third-party references used during prototyping. Each resource has its own license or attribution.
