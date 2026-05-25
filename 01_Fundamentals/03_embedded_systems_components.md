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

# 3.1 Communication Interfaces

Embedded devices often need to exchange information with other devices.

Examples:

- microcontroller from/to computer
- microcontroller from/to  display
- microcontroller from/to  sensor

Communication interfaces make this possible.

---

## 3.1.1 UART (Universal Asynchronous Receiver Transmitter)

Microcontrollers often need to exchange information with other devices. For example, a microcontroller may need to send sensor values to a computer or receive commands from another device. UART is one of the simplest communication methods used for this purpose.

UART is a serial communication protocol, meaning data is sent one bit at a time instead of sending many bits together. It mainly uses two lines:

TX (Transmit) → sends data
RX (Receive) → receives data

Suppose a microcontroller wants to send the text "Hello" to a computer. The data travels through the TX line of the microcontroller and reaches the RX line of the computer.

UART is commonly used for:

communication between PC and microcontroller
debugging programs through serial monitor
communication between small embedded devices

UART is simple and reliable, but it becomes slower compared to protocols like SPI.

Example:

Suppose a microcontroller sends:

"Temperature = 30°C"

The message travels from TX of MCU to RX of the computer.

UART is commonly used for:

- PC and microcontroller communication
- debugging programs
- serial monitoring

UART is simple and reliable but slower than some other protocols.

---

## 3.1.2 SPI (Serial Peripheral Interface)

Some devices need faster communication. For example, displays and SD cards exchange larger amounts of data. UART may not be fast enough in such situations.

SPI is a communication protocol designed for high-speed communication between devices.

SPI uses multiple lines:

MOSI (Master Out Slave In) : sends data from controller to device
MISO (Master In Slave Out) : sends data back
SCLK (Serial Clock) :  synchronizes communication

The clock signal is important because both devices must know exactly when data is being sent and read.

Example:

When a microcontroller writes data to an SD card, SPI allows data transfer much faster than UART.

SPI is widely used with:

displays
SD cards
fast sensors
memory chips

SPI provides high speed, but it requires more wires.

---

## 3.1.3 I2C (Inter-Integrated Circuit)

In embedded systems, several sensors may need to connect to the same microcontroller. Using many wires for each sensor would become messy.

I2C solves this problem by allowing multiple devices to communicate using only two wires:

SDA (Serial Data)
SCL (Serial Clock)

Each device connected to I2C has its own address. The microcontroller communicates by selecting the required device address.

Example:

A system may have:

temperature sensor
pressure sensor
display module

All three devices can share the same two communication wires.

I2C is slower than SPI but uses fewer wires and supports many connected devices.

All devices can share the same communication lines. I2C reduces wiring complexity.

---

## Communication Comparison

| Protocol | Speed | Wires | Common Usage |
|-----------|--------|--------|---------------|
| UART | Medium | 2 | PC communication |
| SPI | Fast | 3+ | Displays, SD cards |
| I2C | Medium | 2 | Sensors and modules |

---

# 3.1.4 Timers

A timer is a built-in hardware module inside the microcontroller that keeps track of time automatically.

Without timers, the CPU would have to continuously count numbers just to create delays. This wastes processing power.

Example:

Suppose an LED should blink every 1 second.

Without a timer:

The CPU repeatedly counts numbers and waits.

With a timer:

The timer counts automatically and notifies the CPU after 1 second.

Timers are used for:

-       delays
-       measuring time intervals
-       periodic tasks
-       event scheduling

Timers help reduce CPU workload.

---

# 3.1.5 PWM (Pulse Width Modulation)

PWM stands for Pulse Width Modulation. It is a technique used to control power delivered to devices.

Microcontrollers can only output digital signals:

ON (1)
OFF (0)

But many devices need gradual control rather than simple ON/OFF operation.

PWM solves this by switching signals ON and OFF very quickly.

If the signal stays ON longer:

→ more power delivered

If the signal stays ON for shorter duration:

→ less power delivered

Examples:

controlling motor speed
LED brightness
fan speed control

Example:

50% PWM:

Signal ON half the time and OFF half the time.

Result:

motor runs around half speed
LED appears medium brightness

Although output is digital, the device behaves as if it receives analog control.


# 3.1.6 ADC (Analog to Digital Converter)

Microcontrollers understand only digital values:

0 and 1

However real-world values are analog.

Examples:

- temperature
- light intensity
- pressure

Sensors often generate voltages.

ADC converts analog signals into digital values that the microcontroller can understand.

Example:

Temperature increases  
↓  
Sensor voltage changes  
↓  
ADC converts voltage  
↓  
MCU receives value like 512

---

## DAC (Digital to Analog Converter)

DAC performs the opposite operation.

It converts digital values into analog signals.

Used in:

- audio systems
- signal generation
- speaker control

ADC:

Analog → Digital

DAC:

Digital → Analog

---

# 4. External World (Sensors and Actuators)

Embedded systems interact with the outside world using sensors and actuators.

---

## Sensors

Sensors are input devices.

They measure physical conditions and convert them into electrical signals.

Examples:

- temperature sensor
- ultrasonic sensor
- pressure sensor
- light sensor
- button

Example:

Temperature increases  
↓  
Voltage changes  
↓  
MCU reads signal

---

## Actuators

Actuators are output devices.

They convert electrical signals into physical action.

Examples:

- motors
- LEDs
- relays
- buzzers
- servo motors

Example:

Button pressed  
↓  
MCU processes input  
↓  
Motor turns ON

---

# Software / Firmware

Software controls how hardware behaves.

Firmware is software written specifically for embedded devices.

Examples:

- device drivers
- application code
- RTOS software

Complex systems may use RTOS (Real Time Operating System).

RTOS helps:

- task scheduling
- resource management
- real-time performance

Examples:

- FreeRTOS
- Zephyr

---

# Final System Flow

Sensors → Microcontroller → Processing → Actuators  
                      ↑  
        Memory + Peripherals + Firmware

---

# Key Understanding

An embedded system is not only a microcontroller.

It is a complete system containing:

- Processing unit
- Memory
- Peripherals
- Software
- Sensors
- Actuators