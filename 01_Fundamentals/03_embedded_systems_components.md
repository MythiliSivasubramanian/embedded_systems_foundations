# Embedded System Components

An embedded system is a combination of hardware and software designed to perform a specific task. Unlike general-purpose computers, embedded systems focus on dedicated functions such as controlling a washing machine, monitoring a sensor, or managing a car braking system.

An embedded system is not just one device or chip. Multiple components work together to read information from the environment, process it, and produce an output.

We can understand an embedded system using four major layers:

1. Core Processing Unit  
2. Memory  
3. Peripherals  
4. External World (Sensors & Actuators)

---

# 1. Core Processing Unit (MCU / MPU)

This is the brain of the embedded system. It receives input data, processes instructions, and controls outputs.

There are mainly two types:

## Microcontroller (MCU)

A microcontroller is a complete computer system on a single chip.

It contains:

- CPU
- Memory (Flash + RAM)
- Communication peripherals
- Timers
- GPIO pins
- ADC and many other modules

Examples:

- STM32
- ATmega (Arduino boards)
- PIC microcontrollers
- Renesas MCUs

Microcontrollers are commonly used in:

- washing machines
- automotive systems
- home appliances
- sensors

Microcontrollers are preferred because they consume less power, cost less, and provide real-time control.

---

## Microprocessor (MPU)

A microprocessor mainly contains only the CPU.

Memory and peripherals are connected externally.

Examples:

- Intel processors
- AMD processors
- ARM Cortex-A processors

Used in:

- laptops
- desktops
- servers
- smartphones

Microprocessors are used where high processing power and complex operating systems are required.

---

# 2. Memory in Embedded Systems

Memory stores instructions and data required by the system.

Different memories exist because different tasks require different behavior.

Some data must remain even after power is removed, while some data is needed only during execution.

---

## Flash Memory / ROM

Flash memory stores the program code and firmware.

ROM stands for Read Only Memory.

Flash is a type of ROM that can be erased and programmed again.

Characteristics:

- stores firmware permanently
- data remains after power OFF
- non-volatile memory

Example:

The embedded code written into STM32 is stored in Flash memory.

---

## RAM (Random Access Memory)

RAM is temporary working memory used during program execution.

The CPU stores temporary information here while the system runs.

Examples:

- variables
- sensor values
- counters

Characteristics:

- very fast
- temporary
- data lost when power OFF

---

## Why both Flash and RAM are needed

Flash stores instructions permanently.

RAM provides temporary working space during execution.

Without Flash:

There is no code available to execute.

Without RAM:

The program has no space to work.

---

# 3. Peripherals

Peripherals are hardware modules available inside the microcontroller.

They help the microcontroller communicate and perform specific tasks without constantly using CPU resources.

Instead of making the CPU handle everything manually, peripherals handle specialized work.

Examples:

- UART
- SPI
- I2C
- Timers
- ADC
- PWM

---