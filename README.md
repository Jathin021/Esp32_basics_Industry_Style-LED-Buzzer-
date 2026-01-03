# ESP32 Basics – Industry Style (LED & Buzzer Projects)

## 📌 Overview
This repository contains a collection of **beginner-to-intermediate embedded systems projects** implemented using the **ESP32 microcontroller** and the **ESP-IDF framework**, following **industry-style coding practices**.

The projects focus on **LED and buzzer outputs only**, making them ideal for:
- Embedded systems learning labs  
- ESP-IDF beginners  
- Understanding timing, PWM, and state machines  
- Transitioning from Arduino-style coding to professional firmware development  

---

## 🎯 Objectives
- Learn ESP-IDF project structure and workflow  
- Understand GPIO, PWM (LEDC), timers, and FreeRTOS basics  
- Implement time-based and state-based logic  
- Write clean, readable, and maintainable embedded C code  

---

## 🧠 What You Will Learn
- GPIO configuration and control  
- LED blinking and pattern generation  
- Buzzer tone generation using PWM  
- Non-blocking timing using system timers  
- Arrays and data-driven programming  
- Finite State Machine (FSM) design  
- Audio–visual synchronization  
- Debugging using ESP_LOG  

---

## 🛠️ Tools & Technologies
- ESP32 Development Board  
- ESP-IDF (Espressif IoT Development Framework)  
- Embedded C  
- FreeRTOS  
- VS Code + ESP-IDF Extension  
- Git & GitHub  

---

## 📁 Repository Structure
```text
Esp32_basics_Industry_Style-LED-Buzzer-/
│
├── Project-1_Blink_LED/
├── Project-2_Police_Siren/
├── Project-3_Digital_Melody_Player/
├── Project-4_SOS_Morse_Code/
├── Project-5_Ticking_Time_Bomb/
├── Project-6_Traffic_Light_System/
│
└── README.md
Projects Included:

1. Blink LED
   - Basic GPIO control using ESP-IDF
   - Introduction to digital output

2. Police Siren
   - Alternating LED pattern
   - Buzzer frequency sweep (siren effect)
   - Non-blocking timing logic

3. Digital Melody Player (Jukebox)
   - Predefined melodies using buzzer
   - Arrays and data-driven note control
   - LED indication for notes

4. SOS Morse Code Beacon
   - Morse code (... --- ...)
   - LED and buzzer synchronized signaling
   - Time-based communication encoding

5. Ticking Time Bomb Countdown
   - LED countdown visualization
   - Accelerating buzzer ticks
   - Final explosion audio-visual effect

6. Automated Traffic Light System (Blind-Friendly)
   - Red–Yellow–Green traffic control
   - Finite State Machine (FSM) design
   - Audio cues for visually impaired pedestrians
Author:
Jathin Pusuluri

Domain:
Embedded Systems | ESP32 | ESP-IDF

GitHub:
https://github.com/Jathin021
