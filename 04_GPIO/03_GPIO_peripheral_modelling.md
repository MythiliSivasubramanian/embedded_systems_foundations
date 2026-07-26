# Modelling the Entire GPIO Peripheral

## 1. From Single Register Access to Peripheral Modelling

In the last file (`02_gpio_register_modelling.md`) we accessed and manipulated a single GPIO register (ODR/IDR). That was a complete chapter on register access.

But in STM32, GPIO is not just one register. A GPIO peripheral contains multiple registers, and STM32 has multiple GPIO peripherals:

- GPIOA
- GPIOB
- GPIOC
- ...
- GPIOI

The goal of this chapter is to model the **entire GPIO peripheral**, not just one register.

Once we understand one GPIO peripheral (GPIOA), the same model can be reused for GPIOB through GPIOI because they all share the same register layout.
So until now, we have been working with individual registers: GPIOA_ODR and GPIOA_IDR.

But in reality, GPIOA is a peripheral that contains many registers.
Here, ODR is not a peripheral, its a (one of the) registers inside the GPIOA peripheral.

## 2. Understanding Structure Memory Layout and Offsets

Before modelling GPIO, we need to understand how a C structure is arranged in memory. We first need to make the C structure behave like a hardware register map.
The idea is: **The C structure layout must match the hardware register layout.**

Consider a normal C structure in memory :

```c
struct TEST
{
    unsigned int A;
    unsigned int B;
    unsigned int C;
};
```

We know that this above Structure needs 12 bytes, since  
unsigned int A; requires 32 bits / 4 bytes
unsigned int B; requires 32 bits / 4 bytes
unsigned int C; requires 32 bits / 4 bytes

Now imagine memory representation in RAM: (conceptually)

Address        Content

0x1000         A
0x1004         B
0x1008         C

Why did B start at 0x1004?

Because A occupied:

0x1000
0x1001
0x1002
0x1003

Next free address is 0x1004. So B starts there.

**Structure members have offsets :**

The important concept: **A structure has a base address.**

Example:
```c
struct TEST my_test;
```

Suppose the compiler places my_test at: Base address = 0x1000
Then:

```text
| Member | Offset | Actual Address |
| ------ | ------ | -------------- |
| A      | 0x00   | 0x1000         |
| B      | 0x04   | 0x1004         |
| C      | 0x08   | 0x1008         |
```

Formula: Member address = Structure base address + Member offset

Exactly the same formula we used for registers: ***Register address = Peripheral base address + Register offset***

## 3. Connecting C Structures with Hardware Register Maps :

Now let's connect this idea to GPIO.
The STM32 reference manual says: GPIOA Base Address = 0x40020000

Inside GPIOA:

Offset  Register

0x00    MODER
0x04    OTYPER
0x08    OSPEEDR
0x0C    PUPDR
0x10    IDR
0x14    ODR

This is already a structure layout.

The hardware is effectively: GPIOA base = 0x40020000

GPIOA base = 0x40020000


Address          Register

0x40020000       MODER
0x40020004       OTYPER
0x40020008       OSPEEDR
0x4002000C       PUPDR
0x40020010       IDR
0x40020014       ODR


Now we want C to understand this.
We create:

```c
struct GPIO_TypeDef
{
    volatile unsigned int MODER;
    volatile unsigned int OTYPER;
    volatile unsigned int OSPEEDR;
    volatile unsigned int PUPDR;
    volatile unsigned int IDR;
    volatile unsigned int ODR;
};
```

The compiler creates:
```
Offset

0x00   MODER
0x04   OTYPER
0x08   OSPEEDR
0x0C   PUPDR
0x10   IDR
0x14   ODR
```

The compiler calculates the offsets based on:
-   order of members
-   size of each members

Result:

Offset

0x00   MODER
0x04   OTYPER
0x08   OSPEEDR
0x0C   PUPDR
0x10   IDR
0x14   ODR

The C structure now matches the hardware register map.

**Example: Structure Offset Calculation**
Consider:

```c
struct GPIO_TypeDef *ptr = (struct GPIO_TypeDef *)0x20000000;
````
The pointer stores:

ptr
 |
 |
 v
0x20000000

Accessing: ptr->MODER means: Base address + MODER offset
0x20000000 + 0x00 = 0x20000000

Accessing: ptr->OTYPER means: 
Base address + OTYPER offset 0x20000000 + 0x04 = 0x20000004

This is the exact trick behind: 
```c
GPIOA->ODR
```
GPIOA base address + ODR offset = ODR register address

## 4. How GPIOA->ODR Works Internally :
Later we create:
```C
GPIO_TypeDef *GPIOA = (GPIO_TypeDef *)0x40020000;
````

The pointer stores:
GPIOA
 |
 |
 v
0x40020000

Now:
```c
GPIOA->ODR
```
becomes: GPIOA base address + ODR offset
0x40020000 + 0x14 = 0x40020014. 0x40020014 is the actual hardware address of GPIOA ODR register.

The structure does not store copies of registers. It only describes the layout.
The pointer connects the structure to the real hardware address.
The CPU writes to the memory address mapped to the hardware register.

## 5. Finding GPIO Information from STM32 Documentation :

For any STM32 microcontroller, we need two official documents:

1. Datasheet
2. Reference Manual

Each document answers a different question.

### Datasheet vs Reference Manual
***1. Datasheet — Information About the Physical MCU***

The Datasheet tells us about the specific MCU device and package.

It provides:
-   Exact device variants
-   Available pins
-   Package information
-   Pin functions
-   Alternate functions
-   Flash and SRAM sizes
-   Electrical characteristics

Example: Our MCU: STM32F407VGT6

The datasheet: STM32F405xx, STM32F407xx, Pinouts and pin description shows the physical pins available for the package.

From the pin definition table, we can see GPIO pins such as:
PA0, PA1, ...
PB0, PB1, ...
PC0, PC1, ...
...
PI0, PI1, ...

Therefore, the STM32F407VG MCU provides GPIO ports:
GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOG, GPIOH, GPIOI

We verify this from the available GPIO pins in the pin definition table.

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

## Where do we find the Peripheral Base Address?

The Reference Manual contains a Memory Map chapter. It gives peripheral base addresses, but the exact peripheral availability depends on the MCU variant.

Example:
| Peripheral | Base Address |
| ---------- | ------------ |
| GPIOA      | `0x40020000` |
| GPIOB      | `0x40020400` |
| GPIOC      | `0x40020800` |
| ...        | ...          |

## 6. STM32F407VG GPIO Peripheral Map

Lets have a quick overview of all ports in STM32F407VG

### STM32F407VG GPIO Ports:
The STM32F407VG contains the following GPIO peripherals:

```
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
```

Here, every GPIO peripheral has the same register layout. The only thing that changes is the base address. This is why a single GPIO structure can represent GPIOA, GPIOB, GPIOC, etc. Only the pointer address changes.

## 7. Reusing One Structure for Multiple GPIO Ports:

Every GPIO peripheral (GPIOA, GPIOB, ..., GPIOI) contains the same registers.

| Offset | Register | Purpose                                                           |
| ------ | -------- | ----------------------------------------------------------------- |
| `0x00` | MODER    | Configure each pin as Input, Output, Alternate Function or Analog |
| `0x04` | OTYPER   | Configure Output Type (Push-Pull / Open-Drain)                    |
| `0x08` | OSPEEDR  | Configure Output Speed                                            |
| `0x0C` | PUPDR    | Configure Pull-up / Pull-down resistors                           |
| `0x10` | IDR      | Input Data Register (Read pin state)                              |
| `0x14` | ODR      | Output Data Register (Read/Write output state)                    |
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
This means we do not need separate structures for GPIOA, GPIOB, GPIOC, etc.
We create one GPIO_TypeDef structure. The base address decides which GPIO peripheral the structure represents.


Example:

```c
GPIO_TypeDef *GPIOA = (struct GPIO_TypeDef *)0x40020000;

GPIO_TypeDef *GPIOB = (struct GPIO_TypeDef *)0x40020400;
```

Same structure but Different hardware. The structure definition represents the register layout, while the pointer represents the actual GPIO peripheral instance.

## Key Takeaways

- A peripheral is a collection of registers.
- Each register has an offset from the peripheral base address.
- A C structure can represent this register layout.
- The structure does not contain hardware registers.
- A pointer connects the structure to the real hardware address.
- The compiler calculates member offsets based on structure layout.
- Same structure + different base address = different peripheral instance.