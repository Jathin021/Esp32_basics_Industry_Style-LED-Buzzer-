ESP32 Basics – Industry Style (LED & Buzzer Projects)
📌 Overview

This repository contains a collection of beginner-to-intermediate embedded systems projects implemented using the ESP32 microcontroller and the ESP-IDF framework, following industry-style coding practices.

The projects focus on LED and buzzer outputs only, making them ideal for:

Embedded systems learning labs

ESP-IDF beginners

Understanding timing, PWM, and state machines

Transitioning from Arduino-style coding to professional firmware development

🎯 Objectives

Learn ESP-IDF project structure and workflow

Understand GPIO, PWM (LEDC), timers, and FreeRTOS basics

Implement time-based and state-based logic

Write clean, readable, and maintainable embedded C code

🧠 What You Will Learn

GPIO configuration and control

LED blinking and patterns

Buzzer tone generation using PWM

Non-blocking timing using system timers

Arrays and data-driven programming

Finite State Machine (FSM) design

Audio–visual synchronization

Debugging using ESP_LOG

🛠️ Tools & Technologies

ESP32 Development Board

ESP-IDF (Espressif IoT Development Framework)

Embedded C

FreeRTOS

VS Code + ESP-IDF Extension

Git & GitHub

📁 Repository Structure
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


Each project follows the standard ESP-IDF structure:

project/
├── main/
│   └── main.c
├── CMakeLists.txt
└── sdkconfig

🚀 Projects Included
🔹 Project 1: Blink LED

Objective:
To understand basic GPIO configuration by blinking an LED using ESP-IDF.

Learning Outcome:
Ability to configure ESP32 GPIO pins and control digital outputs.

🔹 Project 2: Police Siren

Objective:
To simulate a police warning system using two LEDs and a buzzer.

Learning Outcome:
Understanding non-blocking timing and concurrent LED–buzzer control.

🔹 Project 3: Digital Melody Player (Jukebox)

Objective:
To play predefined melodies using a buzzer with LED indication for notes.

Learning Outcome:
Learning array-based data handling and PWM-based sound generation.

🔹 Project 4: SOS Morse Code Beacon

Objective:
To transmit the SOS Morse code pattern using LED flashes and buzzer beeps.

Learning Outcome:
Understanding Morse code encoding and time-based signaling.

🔹 Project 5: Ticking Time Bomb Countdown

Objective:
To design a countdown timer with accelerating buzzer ticks and explosion effect.

Learning Outcome:
Learning array-driven LED control and phase-based timing logic.

🔹 Project 6: Automated Traffic Light System (Blind-Friendly)

Objective:
To implement a traffic signal system with audio cues for visually impaired users.

Learning Outcome:
Understanding finite state machines, timing control, and accessibility-focused embedded design.

⚠️ Important Notes

ESP32 GPIO pins are 3.3V only (not 5V tolerant)

LEDs must be connected with 220Ω resistors

Only safe GPIO pins are used (boot/strap pins avoided)

All projects are implemented without external input devices

👨‍🎓 Who This Repository Is For

EEE / ECE / CSE students

Embedded systems beginners

ESP-IDF learners

Anyone preparing for embedded interviews or labs

📜 License

This repository is intended for educational and learning purposes.

✨ Author

Jathin Pusuluri
Embedded Systems | ESP32 | ESP-IDF
🔗 GitHub: https://github.com/Jathin021
