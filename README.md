# Smart Car Wipers & Parking Sensor Emulator

## Project Overview
This project is an embedded systems simulation of modern car features, specifically smart windshield wipers and parking sensors. It uses a **dsPIC30F4013** microcontroller to process environmental data via hardware interrupts and analog-to-digital (ADC) conversions. 

The system autonomously controls an SG90 servo motor (acting as windshield wipers) whose speed adapts dynamically based on the ambient light simulated by a photoresistor. It also integrates a PIR motion sensor to emulate parking alerts, utilizing a piezoelectric buzzer.

This project was developed as part of the *Applied Electronics* course at the Faculty of Technical Sciences, Novi Sad (Defended: February 2023).

## Hardware Components
*   **Microcontroller:** Microchip dsPIC30Fxxxx (10MHz crystal, `XT_PLL4`)
*   **Sensors:**
    *   **LDR Photoresistor** (Rain/Light detection)
    *   **LM35** (Temperature sensor)
    *   **PIR Sensor** (Motion detection for parking)
*   **Actuators & Indicators:**
    *   **SG90 Servo Motor** (Wiper emulation)
    *   **Piezoelectric Buzzer** (Parking alert audio)
    *   **LEDs for temperature threshold indication
*   **Interface:** GLCD (Graphical LCD) with Touch Panel

## Software & Peripherals Used
The firmware is written entirely in **C** and utilizes the following dsPIC peripherals:
*   **10-bit ADC Module:** Multi-channel conversions for the LDR, LM35, and PIR sensors, utilizing the `_ADCInterrupt`.
*   **UART:** Configured at 9600 baud rate for serial communication. The system is activated remotely by sending an `"ON"` command via the terminal.
*   **Timers (TMR2 & TMR3):** Hardware timers used for precise, non-blocking delays and interrupt generation (`_T2Interrupt`, `_T3Interrupt`).
*   **GPIOs:** Configured to drive the GLCD interface, servo motor PWM logic, and LED/Buzzer outputs.
*   **Development board:** MikroElektronika EasyPIC v7 (used for development and testing)


## How It Works
1.  **Booting:** The system waits for the `"ON"` command via UART to initialize.
2.  **Weather Simulation:** 
    *   *Sunny (Room lighting):* Servo is idle. All sensors operate normally.
    *   *Heavy Rain (Flashlight on LDR):* Wipers run at maximum speed.
    *   *Light Rain (LDR covered):* Wipers run at a moderate speed.
3.  **Resource Management:** To ensure system stability, when the servo is under constant load (continuous wiping) and heavy ADC conversions are taking place, secondary sensors are temporarily disabled.
4.  **Parking Alert:** If the PIR sensor detects motion, the piezo buzzer is triggered to simulate a proximity warning.


