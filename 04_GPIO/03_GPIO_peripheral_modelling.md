## Modelling the entire GPIO peripheral

[In the last file] (02_gpio_register_modelling.md) we have accessed and manipulated a GPIO single register (ODR/IDR). That is a complete chapter on register access. But In STM32, we have multiple ports, PORT A ,... PORT F and each PORT has multiple registers. It is about modelling the entire GPIO peripheral, not just one Port / register. Once we understand one GPIO peripheral (GPIOA), the same model can be reused for GPIOB through GPIOI because they all share the same register layout.

So until now, we've been working with just one register: GPIOA_ODR or GPIOA_IDR. 
But in reality, GPIOA is a peripheral that contains many registers.

Here, ODR is not a peripheral, its a (one of the) register inside the GPIOA peripheral.

### Finding GPIO Information in Any STM32 MCU:

For any STM32 microcontroller, we need two official documents:

1.  Datasheet
2.  Reference Manual

Each document answers a different question.

**1. Datasheet — Information about the physical MCU :**

The Datasheet tells us about the specific device and package.

It provides:
-   Exact device variants
-   Available pins
-   Package information
-   Pin functions
-   Alternate functions
-   Flash and SRAM sizes
-   Electrical characteristics

Example:
Our MCU: STM32F407VGT6

The datasheet:
STM32F405xx, STM32F407xx
Pinouts and pin description

shows the physical pins available for the package.

From the pin definition table, we can see GPIO pins such as:
PA0, PA1, ...
PB0, PB1, ...
PC0, PC1, ...
...
PI0, PI1, ...

Therefore, the STM32F407VG package exposes GPIO ports:
GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOG, GPIOH, GPIOI

We verify it from the available GPIO pins in the pin definition table.

**2. Reference Manual — Information about the peripheral hardware :**

The Reference Manual explains how the internal peripherals work.

For GPIO, it provides:

-   GPIO register list
-   Register offsets
-   Register descriptions
-   Bit definitions
-   Reset values
-   Access type (Read/Write, Read Only)
-   Peripheral memory map

For example, the GPIO chapter lists registers such as:
-   MODER
-   OTYPER
-   OSPEEDR
-   PUPDR
-   IDR
-   ODR
-   BSRR
-   LCKR
-   AFRL
-   AFRH

It also specifies each register's offset from the peripheral base address.

Example:
| Register | Offset |
| -------- | ------ |
| MODER    | 0x00   |
| OTYPER   | 0x04   |
| OSPEEDR  | 0x08   |
| PUPDR    | 0x0C   |
| IDR      | 0x10   |
| ODR      | 0x14   |
| BSRR     | 0x18   |
| LCKR     | 0x1C   |
| AFRL     | 0x20   |
| AFRH     | 0x24   |

**Where do we find the Peripheral Base Address?**
The Reference Manual contains a Memory Map chapter.

This chapter lists the base addresses of all peripherals.

Example:
| Peripheral | Base Address |
| ---------- | ------------ |
| GPIOA      | `0x40020000` |
| GPIOB      | `0x40020400` |
| GPIOC      | `0x40020800` |
| ...        | ...          |


Lets have a quick overview of all ports in STM32F407VG

### STM32F407VG GPIO Ports:
The STM32F407VG contains the following GPIO peripherals:

| GPIO Peripheral | Base Address |
| --------------- | ------------ |
| GPIOA           | `0x40020000` |
| GPIOB           | `0x40020400` |
| GPIOC           | `0x40020800` |
| GPIOD           | `0x40020C00` |
| GPIOE           | `0x40021000` |
| GPIOF           | `0x40021400` |
| GPIOG           | `0x40021800` |
| GPIOH           | `0x40021C00` |
| GPIOI           | `0x40022000` |

Here, every GPIO peripheral has the same register layout. The only thing that changes is the base address.

### Registers inside every GPIO peripheral:

Every GPIO peripheral (GPIOA, GPIOB, ..., GPIOI) contains the same registers.

| Offset | Register | Purpose                                                           |
| ------ | -------- | ----------------------------------------------------------------- |
| `0x00` | MODER    | Configure each pin as Input, Output, Alternate Function or Analog |
| `0x04` | OTYPER   | Configure Output Type (Push-Pull / Open-Drain)                    |
| `0x08` | OSPEEDR  | Configure Output Speed                                            |
| `0x0C` | PUPDR    | Configure Pull-up / Pull-down resistors                           |
| `0x10` | IDR      | Input Data Register (Read pin state)                              |
| `0x14` | ODR      | Output Data Register (Write output state)                         |
| `0x18` | BSRR     | Bit Set/Reset Register (Atomic pin set/reset)                     |
| `0x1C` | LCKR     | Lock Configuration Register                                       |
| `0x20` | AFRL     | Alternate Function Register Low (Pins 0–7)                        |
| `0x24` | AFRH     | Alternate Function Register High (Pins 8–15)                      |

**GPIOA Register Addresses :**

Since the GPIOA base address is 0x40020000, the register addresses become:

| Register | Address      |
| -------- | ------------ |
| MODER    | `0x40020000` |
| OTYPER   | `0x40020004` |
| OSPEEDR  | `0x40020008` |
| PUPDR    | `0x4002000C` |
| IDR      | `0x40020010` |
| ODR      | `0x40020014` |
| BSRR     | `0x40020018` |
| LCKR     | `0x4002001C` |
| AFRL     | `0x40020020` |
| AFRH     | `0x40020024` |

The address of any GPIO register can always be calculated using:

***Register Address = Peripheral Base Address + Register Offset***

For example:
GPIOA Base Address = 0x40020000
ODR Offset         = 0x14
--------------------------------
GPIOA_ODR Address  = 0x40020014

GPIOB Base Address = 0x40020400
ODR Offset         = 0x14
--------------------------------
GPIOB_ODR Address  = 0x40020414

The register offsets never change; only the base address changes from one GPIO peripheral to another.

