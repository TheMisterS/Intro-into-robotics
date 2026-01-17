# Introduction to Robotics Course

This repository contains lab projects from an Introduction to Robotics course, focusing on Arduino-based embedded systems development. Each lab explores different aspects of Arduino programming.

## Labs Overview

### [Lab 1: Pseudo 2FA Authentication System](Lab_1/)
A two-factor authentication system for greenhouse access control combining a keypad for password entry and a photoresistor for daylight detection. Successfully authenticates users only during daylight hours with the correct password, then controls a servo-based door lock.

### [Lab 2: Do-Re-Mi Tone Player](Lab_2/)
An asynchronous, event-driven musical note player that demonstrates timer interrupts and EEPROM memory usage. Plays a Do-Re-Mi sequence using precise timer-based tone generation, with an 8x8 LED matrix for visual feedback and EEPROM tracking of note play counts.

### [Lab 3: Light-Controlled Gate](Lab_3/)
An automatic servo gate that opens based on ambient light levels detected by a photoresistor. Features a manual override button, 7-segment display showing light intensity (0-9 scale), and EEPROM persistence for gate statistics and maximum light readings.

### [Lab 4: Radar-Based Gate System](Lab_4/)
An advanced three-state servo gate system with ultrasonic distance sensing and light override. The radar continuously sweeps 180° to detect nearby objects, automatically opening when objects approach within threshold distance. Includes light-based override mode and animated countdown alerts on an 8x8 LED matrix.
