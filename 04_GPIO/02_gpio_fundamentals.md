# 1. What is a Pin?

A pin is a physical electrical connection point on the MCU package. It is the metal leg/pad that connects the internal MCU circuitry to the outside world.

**Example STM32F407VG:**

```text

          STM32 chip

        ┌─────────────┐
        │             │
 PA0 ---│             │--- PB0
 PA1 ---│             │--- PB1
 PA2 ---│             │--- PC0
        │             │
        └─────────────┘     
```

The physical pin can be connected to:
-   LED
-   Button
-   Sensor
-   Motor driver
-   Communication lines (UART, SPI, I2C)

**Example :**

```text
STM32 pin PA5
       |
       |
       v
      LED
```

The pin is the physical thing that we can touch in hardware.

# 2. What is a Port?

A port is a named group of GPIO pins that share the same GPIO hardware and registers. Instead of naming every pin individually, STM32 groups them.

**Example :**

```text
GPIOA Port

PA0
PA1
PA2
PA3
...
PA15
```

All these pins belong to Port A.

Similarly:

```text
GPIOB Port

PB0
PB1
PB2
...
PB15
```

All these pins belong to Port B.

PA5 means:
1. P  = Port
2. A  = Port name A
3. 5  = Pin number

So: PA5 = Pin 5 of GPIO Port A

A port usually contains 16 pins in STM32F407.

# 3. What is a Peripheral?

A peripheral is a hardware block inside the microcontroller that performs a specific function. The CPU is not alone. Inside the MCU there are many hardware blocks.


**Example :**

```text
STM32 Microcontroller

              CPU
               |
               |
 --------------------------------
 |              |               |
GPIO          UART             SPI
Peripheral    Peripheral       Peripheral

Timer         ADC              I2C
Peripheral    Peripheral       Peripheral
```

Each block is called a peripheral.

**Examples:**

**GPIO Peripheral :** Controls digital input/output pins.

**UART Peripheral :** Handles serial communication.

**ADC Peripheral :** Converts analog voltage into digital values.

**Timer Peripheral :** Counts time and generates PWM signals.

# 4. What is a GPIO Peripheral?

GPIO means: General Purpose Input Output. It is the hardware inside STM32 that controls normal digital pins.

**Example:**

We want to turn on an LED.

The path is:

```text
Our C code
    |
    |
GPIO Register
    |
    |
GPIO Peripheral
    |
    |
Physical Pin PA5
    |
    |
LED
```

The peripheral contains the registers.
The registers control the pins.

The CPU writes to the register first. The register is part of the GPIO peripheral, and changing the register causes the peripheral to drive the pin.



The GPIO peripheral decides:
-   Is PA5 input or output?
-   Should PA5 output HIGH or LOW?
-   Should PA5 have pull-up/pull-down?
-   What speed should it operate?

# 5. What is a Register?

A register is a small memory location inside a peripheral used to configure or control that peripheral.
The CPU cannot directly say: "GPIOA, please make PA5 output."

Instead, the CPU writes a value into a register.

**Example:**

GPIOA has a register called: **MODER.** This register controls pin mode.

Inside MODER:

```text
Bit 10 Bit 11
   |
   |
   +---- PA5 mode
```
   
If we write: 01 PA5 becomes output.

```text
CPU
 |
 |
writes value
 |
 |
MODER register
 |
 |
GPIO hardware
 |
 |
PA5 pin becomes OUTPUT
```

# How everything connects together

Let's take one example: We want LED ON at PA5.

The complete chain:
```text
STM32
 |
 |
GPIOA Peripheral
 |
 |
ODR Register
 |
 |
Bit 5
 |
 |
PA5 Pin
 |
 |
LED
```

**C Code:**

```c
GPIOA->ODR |= (1 << 5);
```
# The simplest definition

| Term           | Meaning                                            |
| -------------- | -------------------------------------------------- |
| **Pin**        | Physical electrical connection on MCU              |
| **Port**       | Group of pins                                      |
| **Peripheral** | Hardware block inside MCU that performs a function |
| **Register**   | Memory location used to control/read a peripheral  |


**The relationship :**

```
Microcontroller
      |
      |
contains
      |
      v
 Peripheral (GPIOA), (GPIOB)..
      |
      |
contains
      |
      v
 Registers
      |
      |
control
      |
      v
 Pins (PA0 - PA15)
```

GPIOA is the GPIO peripheral for Port A. In STM32 documentation and code, "GPIOA" is commonly referred to as both the GPIO peripheral and Port A.

People often say:
-   GPIO Peripheral
-   GPIO Port

and in STM32 they usually refer to the same hardware block.