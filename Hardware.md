# 🔧 DockBot – Hardware Documentation

## 1. Overview

DockBot is an ESP32-based autonomous warehouse patrol robot. Its hardware integrates sensing, motor control, hazard detection and wireless communication to enable autonomous navigation and remote monitoring.

This folder contains the hardware-related documentation, including circuit diagrams, pin connections and component details.

## 2. Hardware Components

| Component | Purpose |
|---|---|
| ESP32 Development Board | Main controller |
| 8-Channel IR Sensor Array | Line detection and navigation |
| Motor Driver | Controls DC motors |
| DC Gear Motors | Robot movement |
| Ultrasonic Sensor | Obstacle detection |
| MQ-135 Sensor | Air-quality monitoring |
| Electrode-Based Liquid Sensor | Liquid spill detection |
| Buzzer | Audible hazard alert |
| LED | Visual hazard indication |
| Battery Supply | Powers the robot |

## 3. ESP32 Pin Configuration

### Line Following Sensors

| Sensor | GPIO |
|---|---:|
| IR Sensor 1 | 19 |
| IR Sensor 2 | 18 |
| IR Sensor 3 | 5 |
| IR Sensor 4 | 17 |
| IR Sensor 5 | 16 |
| IR Sensor 6 | 4 |
| IR Sensor 7 | 2 |
| IR Sensor 8 | 15 |

### Motor Driver Connections

| Motor Control | GPIO |
|---|---:|
| Left Motor Forward | 32 |
| Left Motor Reverse | 33 |
| Right Motor Forward | 26 |
| Right Motor Reverse | 27 |
| Left Motor PWM | 14 |
| Right Motor PWM | 13 |

### Hazard Detection Sensors

| Component | ESP32 GPIO |
|---|---:|
| Liquid Sensor Excitation | 25 |
| Liquid Sensor Analog Input | 34 |
| Ultrasonic TRIG | 21 |
| Ultrasonic ECHO | 22 |
| MQ-135 Analog Output | 35 |
| Buzzer | 23 |
| Warning LED | 12 |

## 4. Hardware Working

### IR Sensor Array
The eight IR sensors detect the track and provide digital sensor readings to the ESP32 for line-position calculation and navigation.

### Motor Driver
The motor driver receives control signals from the ESP32 and drives the left and right DC motors. PWM signals are used to adjust motor speed.

### Liquid Spill Detection
An electrode-based sensing arrangement monitors for the presence of liquid. Its analog output is read by the ESP32.

### Ultrasonic Obstacle Detection
The ultrasonic sensor measures the distance to nearby objects and helps identify obstacles in the robot's path.

### MQ-135 Air Quality Sensor
The MQ-135 provides analog readings that indicate changes in the surrounding air quality.

### Alert System
A buzzer and LED provide local alerts when configured hazard conditions are detected.

## 5. Hardware Images

Add your actual hardware photographs below.

### Complete Robot
![DockBot Robot](../Media/robot_front.jpg)

### Electronics and Wiring
![Electronics](../Media/robot_electronics.jpg)

### Top View
![Top View](../Media/robot_top.jpg)

### Track Setup
![Track Setup](../Media/robot_track.jpg)

## 6. Hardware Documentation Files

| File | Description |
|---|---|
| `circuit_diagram.png` | Complete hardware circuit diagram |
| `pin_connections.md` | ESP32 pin connection reference |
| `component_list.md` | Component and hardware list |

## 7. Important Hardware Notes

- Ensure a common ground between the ESP32 and motor driver power supply.
- Use a suitable motor power supply for the motors.
- Do not connect a 5 V ultrasonic ECHO signal directly to an ESP32 GPIO; use a suitable voltage divider or level shifter.
- GPIO 34 and GPIO 35 are input-only pins on the ESP32.
- Check boot-strapping pin connections when using GPIO 2, 5, 12 and 15.
- Calibrate the liquid sensor threshold according to the actual sensing arrangement.
- Use appropriate power regulation for the ESP32 and sensors.

---

**Project:** DockBot – Autonomous Warehouse Patrol & Hazard Monitoring Robot

**Competition:** COLOSSUS – Hardware / IoT & Robotics Track
