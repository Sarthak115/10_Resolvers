🧭 Silent Spatial Awareness System for Visually Impaired Users

Track: The UNSEEN Challenge – Intelligent Navigation & Perception

📌 Overview

This project presents a wearable, real-time spatial awareness system designed to assist visually impaired users in navigating complex environments without relying on audio, internet, GPS, or cloud services.

The system interprets environmental patterns using non-visual sensors and communicates risk silently through haptic feedback, enabling safe and intuitive navigation in everyday scenarios such as:

Streets and footpaths

Public transport

Crowded indoor environments

🎯 Problem Statement Alignment (PS Compliance)

The solution fully satisfies the mandatory requirements of Track A – The UNSEEN Challenge.

✅ Multi-Level Hazard Detection

Floor-level: potholes, stair drops (downward ultrasonic)

Mid-level: walls, poles, obstacles (front, left, right)

High-level: head and upper-body obstacles (front sensor interpretation)

✅ Human Behaviour Awareness

Detects approaching entities using distance change rate

Infers collision risk from relative motion

Identifies following behavior via rear sensor + user motion

✅ Silent Environment Understanding

❌ No audio output

✅ Entirely vibration-based feedback

Designed for noisy and crowded environments

✅ Dynamic Environment Adaptation

Works reliably in:

Total darkness

Crowded public transport

High-noise conditions

Indoor and outdoor lighting variations

✅ Zero-Internet Autonomous Operation

No GPS

No Bluetooth dependency

No cloud inference

Fully offline, local, real-time processing

🧠 System Architecture

The system is divided into two independent wearable modules.

1️⃣ Cap Module (Sensing & Interpretation)

ESP32

5× Ultrasonic Sensors (Front, Left, Right, Back, Down)

Interprets raw sensor data into intent

Transmits compact navigation packets via ESP-NOW

2️⃣ Belt Module (Haptic Feedback)

ESP32

8× Vibration motors (direction-mapped)

Converts intent into intuitive spatial vibration

Includes manual test and debug interface

Separation of sensing and feedback improves reliability, modularity, and explainability.

🔁 Data Flow Summary
Ultrasonic Sensors
        ↓
Spatial Interpretation (Cap ESP32)
        ↓
Intent Packet (Direction, Risk, Height, Motion)
        ↓  ESP-NOW
Haptic Mapping (Belt ESP32)
        ↓
Directional Vibration Feedback

📦 Communication Protocol
ESP-NOW Packet Structure
typedef struct {
  uint8_t direction;  // 0–7, body-mapped direction
  uint8_t risk;       // 0=low, 1=medium, 2=high
  uint8_t height;     // 0=floor, 1=mid, 2=high
  uint8_t motion;     // 0=static, 1=approaching
} NavPacket;


Only interpreted intent is transmitted — not raw sensor data — reducing:

Latency

Bandwidth

System complexity

🧭 Direction Mapping (Belt Motors)

Motor indices are fixed and spatially mapped around the body:

            [0] Front
      [7] Front-Left     Front-Right [1]

      [6] Left               Right [2]

      [5] Back-Left     Back-Right [3]
            [4] Back


This ensures intuitive, body-referenced feedback without requiring pattern memorization.

📳 Vibration Logic
Risk Level	Vibration Pattern	Meaning
Low	Short pulse	Distant awareness
Medium	Repeated pulse	Caution
High	Continuous	Immediate danger

Direction → motor location

Urgency → vibration pattern

This separation significantly reduces cognitive load.

🧪 Debug & Testing Interfaces
Cap Module (HTTP)

/cap → Interpreted navigation packet

/debug → Raw ultrasonic values + packet

Belt Module (HTTP)

/belt → Last received packet

/test?m=INDEX&r=RISK → Manual motor testing

These endpoints enable transparent validation without Serial debugging.

🔋 Power Architecture
Cap Module

3.7V Li-ion battery

Boosted to regulated 5V

Powers:

ESP32 via VIN

All ultrasonic sensors

Common ground

Decoupling capacitor on 5V rail

Belt Module

Independent battery for motors

ESP32 ground shared with motor drivers

This prevents brownouts and ensures stable wireless operation.

🧠 False Positive Handling

Multi-frame confirmation (temporal filtering)

Distance sanity checks

Motion-aware suppression

Priority-based hazard selection

Safe fallback behavior under low confidence

The system avoids alert overload and builds long-term user trust.

🛠️ Hardware Used

ESP32 Dev Modules ×2

HC-SR04 Ultrasonic Sensors ×5

DC Vibration Motors ×8

NPN Transistors (2N2222 / BC547) ×8

Flyback Diodes ×8

Li-ion battery + boost converter

Elastic belt + mounting foam

💰 Prototype Cost (Approx.)
Module	Cost (₹)
Cap module	~900
Belt module	~1500
Power & enclosure	~400
Total	~3000
🔒 Key Design Decisions

8 motors instead of 4 to reduce ambiguity

No camera, audio, or AI-heavy models

Rule-based interpretation over black-box inference

Offline-first architecture

Human-centered mapping over geometric precision

🚀 Future Enhancements

mmWave radar for finer human motion sensing

Compact custom PCB integration

Adaptive vibration calibration per user

Water-resistant enclosure

Long-term user trials with NGOs
