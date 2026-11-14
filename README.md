# Turbo_Crafters
# 🤖 DockBot – Autonomous Warehouse Patrol & Safety Monitoring Robot

An ESP32-based autonomous warehouse patrol robot designed to monitor predefined paths, detect liquid spills, identify obstacles, monitor surrounding air quality, and transmit real-time sensor data to a Firebase-powered command dashboard.

## 🚀 Project Overview

Warehouses and industrial storage facilities require continuous monitoring to ensure workplace safety and prevent accidents.

DockBot is designed to automate basic warehouse safety inspections using an autonomous line-following robot integrated with multiple sensors and a cloud-based monitoring system.

The robot follows a predefined black-line path using an 8-channel IR sensor array and PID control. It continuously monitors its surroundings for liquid spills and obstacles.

When a safety hazard is detected, DockBot:
- Activates an audible buzzer for liquid spill detection.
- Activates an LED for obstacle detection.
- Immediately stops its motors when a safety hazard is detected.
- Transmits real-time sensor readings and robot status to Firebase.

## 🎯 Key Features

- **Autonomous Navigation:** PID-based line following using an 8-channel IR sensor array.
- **Junction Navigation:** Detects predefined left and right turns.
- **Liquid Spill Detection:** Uses two electrodes to identify changes in electrical conductivity.
- **Obstacle Detection:** Uses an ultrasonic sensor to measure distance.
- **Air Quality Monitoring:** Uses an MQ135 sensor to collect air-quality readings.
- **Emergency Stop:** Stops robot movement whenever a spill or obstacle is detected.
- **Audible Alert:** Buzzer activates during spill detection.
- **Visual Alert:** LED activates during obstacle detection.
- **Cloud Connectivity:** Sends live telemetry to Firebase Realtime Database.
- **Remote Monitoring:** Allows users to monitor robot conditions through a web dashboard.

## 🧰 Hardware Components

| Component | Purpose |
|---|---|
| ESP32 | Main microcontroller |
| 8 IR Sensors | Line detection and navigation |
| DC Motors | Robot movement |
| Motor Driver | Motor direction and speed control |
| Ultrasonic Sensor | Obstacle detection |
| Two Metal Electrodes | Liquid spill detection |
| MQ135 Sensor | Air-quality monitoring |
| Buzzer | Audible safety alert |
| LED | Visual obstacle alert |
| Battery | Portable power supply |
| Robot Chassis | Mechanical structure |

## 🔌 Pin Configuration

### 1. Motor Connections

| Function | ESP32 GPIO |
|---|---:|
| Left Motor Forward | 32 |
| Left Motor Backward | 33 |
| Right Motor Forward | 26 |
| Right Motor Backward | 27 |
| Left Motor PWM | 14 |
| Right Motor PWM | 13 |

### 2. IR Sensor Array

| Sensor | ESP32 GPIO |
|---|---:|
| IR 1 | 19 |
| IR 2 | 18 |
| IR 3 | 5 |
| IR 4 | 17 |
| IR 5 | 16 |
| IR 6 | 4 |
| IR 7 | 2 |
| IR 8 | 15 |

### 3. Safety Sensor Connections

| Component | ESP32 GPIO |
|---|---:|
| Spill Sensor Output | 25 |
| Spill Sensor Input | 34 |
| Ultrasonic Trigger | 21 |
| Ultrasonic Echo | 22 |
| MQ135 Analog Output | 35 |
| Buzzer | 23 |
| LED | 12 |

## ⚙️ Working Principle

### 1. Autonomous Line Following

DockBot uses eight IR sensors to detect a black line on a contrasting surface.

The sensor readings are converted into weighted positions. The PID controller calculates the deviation from the center of the path.

The controller uses:

- Proportional Gain (Kp) = 2.5
- Derivative Gain (Kd) = 1.2
- Base Motor Speed = 60

The calculated correction is applied to the left and right motor speeds to maintain the robot's position on the path.

### 2. Liquid Spill Detection

The spill detection system uses two metal electrodes.

The ESP32 applies a controlled HIGH signal to one electrode and reads the analog response from the other electrode.

When liquid creates a conductive path between the electrodes, the analog reading changes.

Current code threshold:

`SPILL_THRESHOLD = 500`

If the electrode reading exceeds this threshold, the system identifies a possible liquid spill.

The buzzer activates and the robot enters emergency-stop mode.

### 3. Obstacle Detection

An ultrasonic sensor measures the distance between DockBot and nearby objects.

The distance is calculated using:

`Distance = (Echo Time × Speed of Sound) / 2`

Configured obstacle threshold:

`OBSTACLE_DISTANCE = 15 cm`

If an object is detected within 15 cm, the LED turns ON and the robot stops.

### 4. Air Quality Monitoring

The MQ135 sensor provides an analog reading representing its response to surrounding gases.

The ESP32 reads the analog value through GPIO 35.

These readings are transmitted to Firebase for monitoring.

Note: The current implementation reports raw ADC readings and does not calculate calibrated gas concentration in ppm.

## 🛑 Emergency Stop Mechanism

Safety monitoring runs before the line-following controller.

The emergency condition is:

`Safety Active = Spill Detected OR Obstacle Detected`

Whenever either condition becomes TRUE:

1. Both motor PWM outputs are set to zero.
2. Motor direction pins are switched LOW.
3. The robot stops moving.
4. The corresponding warning indicator is activated.

The robot resumes navigation after both safety conditions clear.

## ☁️ Firebase Integration

DockBot uses Firebase Realtime Database to transmit live sensor information over Wi-Fi.

The ESP32 uses:
- WiFi.h
- WiFiClientSecure.h
- HTTPClient.h

A separate FreeRTOS task handles Firebase communication so that cloud uploads do not need to run directly inside the main navigation loop.

### Data transmitted to Firebase

| Parameter | Description |
|---|---|
| status | PATROLLING / EMERGENCY_STOP |
| spillDetected | Liquid spill status |
| obstacleDetected | Obstacle status |
| electrode | Raw electrode reading |
| airQuality | Raw MQ135 reading |
| distanceCm | Measured distance |
| checkpoint | Placeholder in current code |
| zone | Placeholder in current code |
| battery | Not measured currently |
| uptimeMs | ESP32 operating time |

Data is uploaded approximately every 2 seconds to the Firebase Realtime Database.

### Firebase Database Structure

`dockbot/live`

Example telemetry:

```json
{
  "status": "PATROLLING",
  "spillDetected": false,
  "obstacleDetected": false,
  "electrode": 120,
  "airQuality": 350,
  "distanceCm": 45.2,
  "checkpoint": "NOT_CONFIGURED",
  "zone": "UNKNOWN",
  "battery": null,
  "uptimeMs": 125000
}
```

## 💻 Software Requirements

- Arduino IDE
- ESP32 Board Package
- ESP32 Arduino Core
- Firebase Realtime Database
- Wi-Fi connection

### Required Libraries

```cpp
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
```

These libraries are included with the ESP32 Arduino environment.

## 🔧 Installation & Setup

1. Install Arduino IDE.
2. Install the ESP32 board package.
3. Connect the ESP32 to your computer.
4. Open the DockBot Arduino source file.
5. Configure your Wi-Fi credentials.
6. Configure your Firebase Realtime Database URL.
7. Select the correct ESP32 board and COM port.
8. Upload the program.
9. Open Serial Monitor at 115200 baud.
10. Power the robot and verify sensor readings and motor operation.

## 📊 Serial Monitor

The system prints:
- Electrode sensor readings.
- Spill detection status.
- Ultrasonic distance.
- Obstacle detection status.
- MQ135 readings.
- Wi-Fi connection status.
- Emergency warnings.

This helps with debugging and sensor calibration.

## 🔮 Future Enhancements

- Checkpoint detection and counting.
- Warehouse zone identification.
- Battery voltage monitoring.
- Gas concentration calibration and hazard thresholds.
- Advanced obstacle avoidance.
- Historical safety event logging.
- Multiple robot monitoring.
- Improved cloud communication security.
- Automated patrol route management.

## 🏭 Applications

- Warehouse safety inspection.
- Industrial facility monitoring.
- Liquid spill identification.
- Autonomous patrol systems.
- Smart warehouse management.
- Industrial hazard monitoring.

## 👥 Project Team

**Project:** DockBot – Autonomous Warehouse Patrol & Safety Monitoring System

**Controller:** ESP32

**Cloud Platform:** Firebase Realtime Database

**Development Environment:** Arduino IDE

---

## 📌 Important Note

This repository contains the current ESP32 implementation. Features such as checkpoint tracking, zone identification, and battery monitoring are planned enhancements and are not yet implemented in the uploaded firmware.

**DockBot – Making Warehouse Monitoring Smarter, Safer, and Autonomous.**
