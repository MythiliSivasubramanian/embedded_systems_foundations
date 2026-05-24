# Embedded Systems Overview

Computers like laptops and desktops are designed to perform many different tasks. We can browse the internet, watch videos, write code, play games, edit documents etc. These are general purpose systems.

Embedded systems are different.

An embedded system is a computer system designed for a specific purpose inside a larger device. Instead of doing many unrelated tasks, it focuses on one dedicated function. i.e, It does not do multiple unrelated jobs like a laptop. It is made for one fixed purpose.


Examples:

- Washing machine → controls wash cycles
- Microwave → controls heating and timers
- Smart watch → tracks sensors and displays information
- Printer → controls printing operations
- Traffic light → manages signal timing
- Car braking system → monitors and controls braking

---

## Working of an Embedded System:

An embedded system usually contains two major parts: Hardware and Software

### Hardware

Hardware is the physical part of the system.

Examples:

- Microcontroller
- Sensors
- Memory
- Input devices
- Output devices

### Software

Software contains instructions/code that tell hardware what actions to perform.

Examples:

- Firmware
- Device drivers
- Application code

---

## Relationship between Hardware and Software 

The software provides instructions and controls behavior of the hardware.

For example:

When a Button (Hardware) is pressed, Microcontroller detects input and Software executes logic  
and then LED turns ON. 
This shows how hardware and software work together to perform a task.

---

## Usage of C in Embedded Systems

Embedded devices often have limited memory and processing power.
C became popular because it provides control + efficiency:

- Fast execution
- low memory usage
- can directly access to hardware
- Supports registers and bit operations
- Good balance between low-level and high-level programming

This gives programmers control over the hardware while keeping programs efficient.

---

## Key Idea

Embedded systems are specialized computers designed to perform dedicated tasks by combining hardware and software. Embedded system = combination of hardware + software designed for a specific task.