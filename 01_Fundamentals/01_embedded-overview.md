# Embedded Systems Overview

Computers like laptops and desktops are designed to perform many different tasks. We can browse the internet, watch videos, write code, play games, and edit documents. These are general-purpose systems.

Embedded systems are different.

An embedded system is a computer system built for a specific purpose inside a larger device. It is designed to perform one dedicated function, usually with constrained processing power, memory, and input/output.


Examples:

- Washing machine → controls wash cycles
- Microwave → manages heating and timing
- Smart watch → reads sensors and displays information
- Printer → controls printing operations
- Traffic light → manages signal timing
- Car braking system → monitors and controls braking

---

## Typical basic embedded system structure

A simple embedded system usually includes:

- Sensors to read the environment
- A processor to run the control logic
- Memory to store the program and temporary data
- Actuators or outputs to control the world

This combination of hardware and software is what makes the system embedded.

---

## Hardware and software roles

### Hardware

Hardware is the physical part of the system.

Examples:

- Microcontroller
- Sensors
- Memory
- Input devices
- Output devices

### Software

Software is the code that tells the hardware what to do.

Examples:

- Firmware
- Device drivers
- Application logic

### How they work together

When a button is pressed, the hardware sends a signal to the processor. The software reads the input, decides what to do, and then turns an LED on or activates a motor. This shows how hardware and software collaborate to complete a task.

---

## Why C is common in embedded systems

Embedded devices often have limited memory and processing power. C is common because it is efficient and gives direct control over hardware.

Benefits of C for embedded systems:

- Fast execution
- Low memory usage
- Direct access to hardware registers
- Good support for bit operations
- Minimal runtime overhead

---

## Key idea

Embedded systems are specialized computers designed for a dedicated task. They combine hardware and software to solve a specific problem reliably and efficiently.