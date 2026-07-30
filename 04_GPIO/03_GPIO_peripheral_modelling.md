# Modelling the Entire GPIO Peripheral

## Index

1. [From Single Register Access to Peripheral Modelling](#1-from-single-register-access-to-peripheral-modelling)
2. [Understanding Structure Memory Layout and Offsets](#2-understanding-structure-memory-layout-and-offsets)
3. [Connecting C Structures with Hardware Register Maps](#3-connecting-c-structures-with-hardware-register-maps)
4. [How GPIOA->ODR Works Internally](#4-how-gpioa-odr-works-internally)
5. [Finding GPIO Information from STM32 Documentation](#5-finding-gpio-information-from-stm32-documentation)
6. [STM32F407VG GPIO Peripheral Map](#6-stm32f407vg-gpio-peripheral-map)
7. [Same Register Layout, Different Peripheral Instance](#7-same-register-layout-different-peripheral-instance)
8. [GPIO Register Operations](#8-gpio-register-operations)
   - [8.1 Write Whole Register](#81-write-whole-register)
   - [8.2 Read Whole Register](#82-read-whole-register)
   - [8.3 Toggle Whole Register](#83-toggle-whole-register)
9. [GPIO Single Pin Operations](#9-gpio-single-pin-operations)
   - [9.1 Set a Single Pin](#91-set-a-single-pin)
   - [9.2 Clear a Single Pin](#92-clear-a-single-pin)
   - [9.3 Read a Single Pin](#93-read-a-single-pin)
   - [9.4 Toggle a Single Pin](#94-toggle-a-single-pin)
10. [BSRR Register](#10-bsrr-register)
    - [Why ODR Read-Modify-Write is risky](#why-odr-read-modify-write-is-risky)
    - [BSRR Register Layout](#bsrr-register-layout)
    - [Bit Set operation](#bit-set-operation)
    - [Bit Reset operation](#bit-reset-operation)
    - [Atomic GPIO control comparison](#atomic-gpio-control-comparison)
    - [Case 1: Set a Single Pin (e.g., PA5 HIGH)](#case-1-set-a-single-pin-eg-pa5-high)
    - [Case 2: Reset a Single Pin (e.g., PA5 LOW)](#case-2-reset-a-single-pin-eg-pa5-low)
    - [Case 3: Set Multiple Pins (e.g., PA1, PA3, PA5 HIGH)](#case-3-set-multiple-pins-eg-pa1-pa3-pa5-high)
    - [Case 4: Reset Multiple Pins (e.g., PA1, PA3, PA5 LOW)](#case-4-reset-multiple-pins-eg-pa1-pa3-pa5-low)
    - [Case 5: Set and Reset ONE Pin in the Same Command (Collision Edge-Case)](#case-5-set-and-reset-one-pin-in-the-same-command-collision-edge-case)
    - [Case 6: Set Some Pins and Reset Other Pins Simultaneously](#case-6-set-some-pins-and-reset-other-pins-simultaneously)
11. [Key Takeaways](#11-key-takeaways)

---

## 1. From Single Register Access to Peripheral Modelling

In the last file (`02_gpio_register_modelling.md`) we accessed and manipulated a single GPIO register (ODR/IDR). That was a complete chapter on register access.

But in STM32, GPIO is not just one register. A GPIO peripheral contains multiple registers, and STM32 has multiple GPIO peripherals:

- GPIOA
- GPIOB
- GPIOC
- ...
- GPIOI

The goal of this chapter is to model the **entire GPIO peripheral**, not just one register.

Once we understand one GPIO peripheral (GPIOA), the same structure can be reused for GPIOB through GPIOI because they all share the same register layout.
So until now, we have been working with individual registers: GPIOA_ODR and GPIOA_IDR.

But in reality, GPIOA is a peripheral that contains many registers.
Here, ODR is not a peripheral, its a (one of the) registers inside the GPIOA peripheral.

---

## 2. Understanding Structure Memory Layout and Offsets

Before modelling GPIO, we need to understand how a C structure is arranged in memory. We first need to make the C structure behave like a hardware register map.
The idea is: **The C structure layout must match the hardware register layout.**

Consider a normal C structure in memory:

```c
struct TEST
{
    unsigned int A;
    unsigned int B;
    unsigned int C;
};

```

We know that this above Structure needs 12 bytes, since:

* `unsigned int A;` requires 32 bits / 4 bytes
* `unsigned int B;` requires 32 bits / 4 bytes
* `unsigned int C;` requires 32 bits / 4 bytes

Now imagine memory representation in RAM: (conceptually)

```text
Address        Content

0x1000         A
0x1004         B
0x1008         C

```

Why did B start at 0x1004?

Because A occupied:

* 0x1000
* 0x1001
* 0x1002
* 0x1003

Next free address is 0x1004. So B starts there.

**Structure members have offsets:**

The important concept: **A structure has a base address.**

Example:

```c
struct TEST my_test;

```

Suppose the compiler places my_test at: Base address = 0x1000
Then:

| Member | Offset | Actual Address |
| --- | --- | --- |
| A | 0x00 | 0x1000 |
| B | 0x04 | 0x1004 |
| C | 0x08 | 0x1008 |

Formula: Member address = Structure base address + Member offset

Exactly the same formula we used for registers: ***Register address = Peripheral base address + Register offset***

---

## 3. Connecting C Structures with Hardware Register Maps

Now let's connect this idea to GPIO.
The STM32 reference manual provides the GPIO peripheral base addresses. For STM32F407VG, GPIOA base address is 0x40020000.

Inside GPIOA:

| Offset | Register |
| --- | --- |
| 0x00 | MODER |
| 0x04 | OTYPER |
| 0x08 | OSPEEDR |
| 0x0C | PUPDR |
| 0x10 | IDR |
| 0x14 | ODR |
| 0x18 | BSRR |
| 0x1C | LCKR |
| 0x20 | AFRL |
| 0x24 | AFRH |

This is already a structure layout.

The hardware is effectively: GPIOA base = 0x40020000

```text
Address          Register

0x40020000       MODER
0x40020004       OTYPER
0x40020008       OSPEEDR
0x4002000C       PUPDR
0x40020010       IDR
0x40020014       ODR
0x40020018       BSRR
0x4002001C       LCKR
0x40020020       AFRL
0x40020024       AFRH

```

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
    volatile unsigned int BSRR;
    volatile unsigned int LCKR;
    volatile unsigned int AFRL;
    volatile unsigned int AFRH;
};

```

The `volatile` keyword tells the compiler that these memory locations can change outside normal program flow because they are hardware registers. Therefore, the compiler must always perform actual memory reads and writes.

The compiler creates:

```text
Offset   Register

0x00     MODER
0x04     OTYPER
0x08     OSPEEDR
0x0C     PUPDR
0x10     IDR
0x14     ODR
0x18     BSRR
0x1C     LCKR
0x20     AFRL
0x24     AFRH

```

Real STM32 register maps sometimes have gaps:

Example:

* Offset 0x00 Register A
* Offset 0x04 Register B
* Offset 0x0C Register C

Then the structure needs: `uint32_t RESERVED;` to maintain correct offsets.
For GPIO currently we don't need it because the registers are continuous here.

The compiler calculates the offsets based on:

* order of members
* size of each member

### Important: Structure Alignment

The compiler may insert padding bytes inside structures for alignment.
For hardware register mapping, we must ensure the structure layout exactly matches the hardware layout.

In STM32 register maps, registers are normally aligned to 32-bit boundaries, so using 32-bit members (`unsigned int` or `uint32_t`) naturally creates the required offsets.

Result:

```text
Offset   Register

0x00     MODER
0x04     OTYPER
0x08     OSPEEDR
0x0C     PUPDR
0x10     IDR
0x14     ODR
0x18     BSRR
0x1C     LCKR
0x20     AFRL
0x24     AFRH

```

The C structure now matches the hardware register map.

**Example: Structure Offset Calculation**

Consider:

```c
struct GPIO_TypeDef *ptr = (struct GPIO_TypeDef *)0x20000000;

```

The pointer stores:

```text
ptr
 |
 |
 v
0x20000000

```

Accessing: `ptr->MODER` means: Base address + MODER offset

`0x20000000 + 0x00 = 0x20000000`

Accessing: `ptr->OTYPER` means: Base address + OTYPER offset

`0x20000000 + 0x04 = 0x20000004`

This is the exact trick behind:

```c
GPIOA->ODR

```

GPIOA base address + ODR offset = ODR register address

---

## 4. How GPIOA->ODR Works Internally

Later we create:

```c
struct GPIO_TypeDef *GPIOA = (struct GPIO_TypeDef *)0x40020000;

```

The pointer stores:

```text
GPIOA
 |
 |
 v
0x40020000

```

Now:

```c
GPIOA->ODR

```

becomes: GPIOA base address + ODR offset

`0x40020000 + 0x14 = 0x40020014`. `0x40020014` is the actual hardware address of GPIOA ODR register.

The structure does not store copies of registers. It only describes the layout.
The pointer connects the structure to the real hardware address.
The CPU writes to the memory address mapped to the hardware register.

Basically, the structure only defines the layout. Memory is not allocated for the registers because the pointer directly points to the peripheral memory address.

Now that we understand how a C structure can represent a peripheral,
the next question is: how do we get the information needed to create this structure?

---

## 5. Finding GPIO Information from STM32 Documentation

For any STM32 microcontroller, we need two official documents:

1. Datasheet
2. Reference Manual

Each document answers a different question.

### Datasheet vs Reference Manual

**1. Datasheet — Information About the Physical MCU**

The Datasheet tells us about the specific MCU device and package.

It provides:

* Exact device variants
* Available pins
* Package information
* Pin functions
* Alternate functions
* Flash and SRAM sizes
* Electrical characteristics

Example: Our MCU: STM32F407VGT6

The datasheet: STM32F405xx, STM32F407xx, Pinouts and pin description shows the physical pins available for the package.

From the pin definition table, we can see GPIO pins such as:

* PA0, PA1, ...
* PB0, PB1, ...
* PC0, PC1, ...
* ...
* PI0, PI1, ...

Therefore, the STM32F407VG MCU provides GPIO peripherals:
GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOG, GPIOH and GPIOI.

We verify this from the available GPIO pins in the pin definition table.

**2. Reference Manual — Information about the peripheral hardware:**

The Reference Manual explains how the internal peripherals work.

For GPIO, it provides:

* GPIO register list
* Register offsets
* Register descriptions
* Bit definitions
* Reset values
* Access type (Read/Write, Read Only)
* Peripheral memory map

For example, the GPIO chapter lists registers such as:

* MODER
* OTYPER
* OSPEEDR
* PUPDR
* IDR
* ODR
* BSRR
* LCKR
* AFRL
* AFRH

It also specifies each register's offset from the peripheral base address.

Example:

| Register | Offset |
| --- | --- |
| MODER | 0x00 |
| OTYPER | 0x04 |
| OSPEEDR | 0x08 |
| PUPDR | 0x0C |
| IDR | 0x10 |
| ODR | 0x14 |
| BSRR | 0x18 |
| LCKR | 0x1C |
| AFRL | 0x20 |
| AFRH | 0x24 |

### Where do we find the Peripheral Base Address?

The Reference Manual contains a Memory Map chapter. It gives peripheral base addresses, but the exact peripheral availability depends on the MCU variant.

Example:

| Peripheral | Base Address |
| --- | --- |
| GPIOA | `0x40020000` |
| GPIOB | `0x40020400` |
| GPIOC | `0x40020800` |
| ... | ... |

---

## 6. STM32F407VG GPIO Peripheral Map

Let's have a quick overview of all ports in STM32F407VG.

### STM32F407VG GPIO Ports:

The STM32F407VG contains the following GPIO peripherals:

| GPIO Peripheral | Base Address |
| --- | --- |
| GPIOA | `0x40020000` |
| GPIOB | `0x40020400` |
| GPIOC | `0x40020800` |
| GPIOD | `0x40020C00` |
| GPIOE | `0x40021000` |
| GPIOF | `0x40021400` |
| GPIOG | `0x40021800` |
| GPIOH | `0x40021C00` |
| GPIOI | `0x40022000` |

Here, every GPIO peripheral has the same register layout. The only thing that changes is the base address. This is why a single GPIO structure can represent GPIOA, GPIOB, GPIOC, etc. Only the pointer address changes.

---

## 7. Same Register Layout, Different Peripheral Instance

Every GPIO peripheral (GPIOA, GPIOB, ..., GPIOI) contains the same registers. The register offsets remain identical for every GPIO peripheral. Only the base address changes.

| Offset | Register | Purpose |
| --- | --- | --- |
| `0x00` | MODER | Configure each pin as Input, Output, Alternate Function or Analog |
| `0x04` | OTYPER | Configure Output Type (Push-Pull / Open-Drain) |
| `0x08` | OSPEEDR | Configure Output Speed |
| `0x0C` | PUPDR | Configure Pull-up / Pull-down resistors |
| `0x10` | IDR | Input Data Register (Read pin state) |
| `0x14` | ODR | Output Data Register (Read/Write output state) |
| `0x18` | BSRR | Bit Set/Reset Register (Atomic pin set/reset) |
| `0x1C` | LCKR | Lock Configuration Register |
| `0x20` | AFRL | Alternate Function Register Low (Pins 0–7) |
| `0x24` | AFRH | Alternate Function Register High (Pins 8–15) |

**GPIOA Register Addresses:**

Since the GPIOA base address is `0x40020000`, the register addresses become:

| Register | Address |
| --- | --- |
| MODER | `0x40020000` |
| OTYPER | `0x40020004` |
| OSPEEDR | `0x40020008` |
| PUPDR | `0x4002000C` |
| IDR | `0x40020010` |
| ODR | `0x40020014` |
| BSRR | `0x40020018` |
| LCKR | `0x4002001C` |
| AFRL | `0x40020020` |
| AFRH | `0x40020024` |

The address of any GPIO register can always be calculated using:

***Register Address = Peripheral Base Address + Register Offset***

For example:

```text
GPIOA Base Address = 0x40020000
ODR Offset         = 0x14
--------------------------------
GPIOA_ODR Address  = 0x40020014

GPIOB Base Address = 0x40020400
ODR Offset         = 0x14
--------------------------------
GPIOB_ODR Address  = 0x40020414

```

The register offsets never change; only the base address changes from one GPIO peripheral to another.
This means we do not need separate structures for GPIOA, GPIOB, GPIOC, etc.
We create one `GPIO_TypeDef` structure. The base address decides which GPIO peripheral the structure represents.

Example:

```c
struct GPIO_TypeDef *GPIOA = (struct GPIO_TypeDef *)0x40020000;
struct GPIO_TypeDef *GPIOB = (struct GPIO_TypeDef *)0x40020400;

```

Same structure but Different hardware. The structure definition represents the register layout, while the pointer represents the actual GPIO peripheral instance.

```c
#define GPIOA ((struct GPIO_TypeDef *)0x40020000)

```

Then:

```c
GPIOA->ODR = 1;

```

becomes `0x40020000 + 0x14 = 0x40020014`.

---

## 8. GPIO Register Operations

Now that we modeled a GPIO Peripheral, let's see what operations we can do at the whole register level and at individual pin levels.

### 8.1 Write Whole Register

Write 0x05 to `GPIOB_ODR` (Write `0x05` at `0x40020414` (GPIOB address + ODR Offset)):

```c
GPIOB->ODR = 0x05; // Same as (*GPIOB).ODR = 0x05

```

Flow:

```text
CPU
 |
 v
Write entire 32-bit value
 |
 v
GPIOB ODR register

```

So the complete C code so far:

```c
/*
  03_gpio_peripheral_modelling.c
  Modelling GPIO peripheral using C Structures
*/

#include <stdint.h>

/*
  GPIO peripheral register layout Offset as per STM32F407VG reference manual
  MODER   -> 0x00
  OTYPER  -> 0x04
  OSPEEDR -> 0x08
  PUPDR   -> 0x0C
  IDR     -> 0x10
  ODR     -> 0x14 
  BSRR    -> 0x18
  LCKR    -> 0x1C
  AFRL    -> 0x20
  AFRH    -> 0x24
 */

// The structure represents the Hardware register layout
typedef struct
{
    volatile uint32_t MODER;      
    volatile uint32_t OTYPER;     
    volatile uint32_t OSPEEDR;    
    volatile uint32_t PUPDR;      
    volatile uint32_t IDR;        
    volatile uint32_t ODR;        
    volatile uint32_t BSRR;       
    volatile uint32_t LCKR;       
    volatile uint32_t AFRL;       
    volatile uint32_t AFRH;       
} GPIO_TypeDef;

/*
  GPIO peripheral base addresses
  STM32F407VG Reference Manual:
*/

#define GPIOA ((GPIO_TypeDef *) 0x40020000U)
#define GPIOB ((GPIO_TypeDef *) 0x40020400U)
#define GPIOC ((GPIO_TypeDef *) 0x40020800U)
#define GPIOD ((GPIO_TypeDef *) 0x40020C00U)
#define GPIOE ((GPIO_TypeDef *) 0x40021000U)
#define GPIOF ((GPIO_TypeDef *) 0x40021400U)
#define GPIOG ((GPIO_TypeDef *) 0x40021800U)
#define GPIOH ((GPIO_TypeDef *) 0x40021C00U)
#define GPIOI ((GPIO_TypeDef *) 0x40022000U)

int main(void)
{
    /* Write a register: CPU sends data to the memory address of the hardware register. */
  
    // Write 0x05 to GPIOB_ODR (Write 0x05 at 0x40020414 (GPIOB address + ODR Offset))
    GPIOB->ODR = 0x05;       // Same as (*GPIOB).ODR = 0x05

    /* Read a register */

    return 0;
}

```

### 8.2 Read Whole Register

We have learned writing to a register: `GPIOB->ODR = 0x05;`

Meaning: CPU sends data to the memory address of the hardware register.

Now we move to the opposite direction which is reading a register. Before GPIO, let's understand the basic C idea.
A register is just a memory location from the CPU's point of view.

When we read a register, we are doing: **Hardware register → CPU → C variable**

**Step 1: Hardware side**

Suppose GPIOB IDR register address is:

* GPIOB Base Address = `0x40020400`
* IDR Offset = `0x10`
* IDR Address = `0x40020400` + `0x10` = `0x40020410`

So:

```text
Address          Register

0x40020410       GPIOB_IDR

```

The GPIO hardware continuously updates this register based on the physical pin voltage.

**Example:**

Imagine:

* PB0 = HIGH
* PB1 = LOW
* PB2 = HIGH

Then IDR contains:

```text
Bit:
31 ........ 3 2 1 0

IDR = 000000000000 1 0 1

```

**Step 2: Reading using our structure**

We already created:

```c
typedef struct
{
    volatile uint32_t MODER;
    volatile uint32_t OTYPER;
    volatile uint32_t OSPEEDR;
    volatile uint32_t PUPDR;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t LCKR;
    volatile uint32_t AFRL;
    volatile uint32_t AFRH;
} GPIO_TypeDef;

```

And:

```c
#define GPIOB ((GPIO_TypeDef *)0x40020400U)

```

Now:

```c
uint32_t pin_state;
pin_state = GPIOB->IDR;

```

#### What happens internally?

Let's follow the compiler.

1. `GPIOB->IDR` becomes `((GPIO_TypeDef *)0x40020400U)->IDR`
2. **Find IDR offset:**
From the structure:
```text
MODER   0x00
OTYPER  0x04
OSPEEDR 0x08
PUPDR   0x0C
IDR     0x10   <--
ODR     0x14

```


Compiler knows: IDR offset = `0x10`
3. **Calculate address:**
```text
Base address = 0x40020400
IDR offset   = 0x10
--------------------
Address      = 0x40020410

```


4. **CPU performs READ:**
The CPU executes something like: Read memory address `0x40020410`
Hardware responds: IDR value = `0x00000005`
CPU stores it into: `pin_state`
Now, `pin_state` = `0x05`;

**Complete Example: Read a whole Register**

```c
int main(void)
{
    uint32_t pin_state;

    // Read GPIOB input data register
    pin_state = GPIOB->IDR;  // Same as pin_state = (*GPIOB).IDR;

    return 0;
}

```

#### Important difference: Write vs Read register

**Write register:**

```c
GPIOB->ODR = 0x05; // Same as (*GPIOB).ODR = 0x05;

```

Flow:

```text
CPU
 |
 | sends data to
 v
GPIOB ODR register
 |
 |
 v
GPIO pin changes

```

**Read register:**

```c
pin_state = GPIOB->IDR;

```

Flow:

```text
GPIO pin voltage
 |
 |
 v
GPIOB IDR register
 |
 | sends to 
 v
CPU variable

```

### 8.3 Toggle Whole Register

Toggle means: 0 becomes 1 and 1 becomes 0.

For a register: Current value: `1010 0101`

After toggling: `0101 1010`

In C, toggling uses XOR.

**The XOR truth table:**

| A | B | A ^ B |
| --- | --- | --- |
| 0 | 0 | 0 |
| 0 | 1 | 1 |
| 1 | 0 | 1 |
| 1 | 1 | 0 |

**Example (Toggle one bit):**

Suppose ODR: `0000 0000`

We want to toggle bit 0.

Mask: `0000 0001`

Operation:

```text
  0000 0000
^ 0000 0001
------------
  0000 0001

```

Bit 0 changed from Low → High.

**Example (Toggle whole register):**

To toggle the entire register, we XOR with all bits set to 1.

Current register: `1010 0101`

To XOR, Mask with `1111 1111`

```text
  1010 0101
^ 1111 1111
------------
  0101 1010

```

Every bit flips.

In C, `GPIOB->ODR ^= 0xFFFFFFFF;` or `GPIOB->ODR = GPIOB->ODR ^ 0xFFFFFFFF;`

Meaning:

1. Read current ODR value.
2. XOR it with all 1s.
3. Write the result back.

However, one important embedded point:

***A whole-register toggle is usually not common for GPIO.***

Because a GPIO port has multiple pins.

**Example:**

```text
ODR:

Bit 15 ........ Bit 0

0000 0000 0000 1010
             ^ ^
            PB3 PB1

```

If we toggle the whole register:

```c
GPIOB->ODR ^= 0xFFFFFFFF;

```

we flip all the pins, which may accidentally change pins connected to other hardware.

#### What happens internally?

For `GPIOB->ODR ^= 0xFFFFFFFF;` The CPU does:

**Step 1: Read:**

```text
GPIOB ODR address: 0x40020414
       |
       v
CPU reads current value

```

Example: `ODR = 0x00000005`

**Step 2: XOR:**

```text
  00000000 00000000 00000000 00000101
^ 11111111 11111111 11111111 11111111
--------------------------------------
  11111111 11111111 11111111 11111010 (32 bit binary number)

```

**Step 3: Write back:**

Since `11111111 11111111 11111111 11111010` (32 bit binary number) is hard to read every time, hexadecimal (base 16) is preferred. 1 hexadecimal digit = 4 binary bits (1 nibble).

`11111111 11111111 11111111 11111010` in hexadecimal is `0xFFFFFFFA`.

CPU writes: `0xFFFFFFFA` to `0x40020414`.

But in a real application: `GPIOB->ODR ^= 0xFFFFFFFF;` is dangerous.

Because GPIOB may have:

* PB0 → LED
* PB1 → Sensor
* PB2 → Communication line
* PB3 → Debug pin
* and so on.

We don't want to accidentally toggle everything. Usually we toggle only the pin we want: `GPIOB->ODR ^= (1U << 5);`

**Example:**

Suppose `GPIOB->ODR = 0x0000000F;`, then the register has `0000 1111`.

Then we execute: `GPIOB->ODR ^= 0xFFFFFFFF;`

What will be the new ODR value?

**Initial value:**

* `GPIOB->ODR = 0x0000000F;`
* `0000 0000 0000 0000 0000 0000 0000 1111` which is `0x0000000F`

**XOR mask:**

* `1111 1111 1111 1111 1111 1111 1111 1111` which is `0xFFFFFFFF`

**XOR operation:**

```text
  0000 0000 0000 0000 0000 0000 0000 1111
^ 1111 1111 1111 1111 1111 1111 1111 1111
-----------------------------------------
  1111 1111 1111 1111 1111 1111 1111 0000

```

Hexadecimal: `0xFFFFFFF0`

4 binary bits (1 nibble) = 1 hexadecimal digit.

1111 = F

0000 = 0

Notice what the CPU actually did for: `GPIOB->ODR ^= 0xFFFFFFFF;`

It didn't perform a single "toggle" instruction. Internally, it performed three operations:

```text
Step 1: Read GPIOB->ODR -> CPU register
Step 2: CPU computes: Old Value XOR 0xFFFFFFFF
Step 3: Write the new value back to GPIOB->ODR

```

This is called a **Read-Modify-Write (RMW) operation.** Chances could be that between these steps (Read, Modify, Write), the hardware could change the register values unexpectedly. Hence, we can use BSRR (Bit Set/Reset Register). More on that topic in detail later.

**Complete Example: Toggle a whole register**

```c
int main(void)
{
    // Toggle GPIOB ODR (Whole register toggle is dangerous)
    GPIOB->ODR ^= 0xFFFFFFFF; // Same as (*GPIOB).ODR ^= 0xFFFFFFFF;
    // Same as (*GPIOB).ODR = (*GPIOB).ODR ^ 0xFFFFFFFF;
    // Same as GPIOB->ODR = GPIOB->ODR ^ 0xFFFFFFFF;

    return 0;
}

```

---

## 9. GPIO Single Pin Operations

### 9.1 Set a Single Pin (Write 1)

**What does "Set a Pin" mean?**

Suppose GPIOB ODR contains:

```text
Bit:  7 6 5 4 3 2 1 0
Val:  1 0 1 0 0 1 0 1

```

Now, we want to set PB3 to HIGH / 1. Only Bit 3 should change and every other bit should remain same. That's the goal.

So what bitwise operator could we use here? Logical OR.

**Bitwise OR truth table:**

| A | B | A | B |
| --- | --- | --- |
| 0 | 0 | 0 |
| 0 | 1 | 1 |
| 1 | 0 | 1 |
| 1 | 1 | 1 |

Now look only at the second column. If we OR with 1:

* `0 | 1 = 1`
* `1 | 1 = 1`

Whether the original bit was 0 or 1, the result is always 1. That's exactly what we want.

Now back to our example:

```text
ODR         = 1010 0101
Mask        = 0000 1000
-----------------------
ODR | Mask  = 1010 1101

```

Comparing the `ODR | Mask` result `1010 1101` with original `ODR` `1010 0101`, only the required bit 3 is changed from 0 to 1. All other bits remain the same.

**Complete Example: Write / Set a single pin**

```c
int main(void)
{
    // Write / Set a single pin GPIOB_ODR PIN 3
    GPIOB->ODR |= (1U << 3);

    return 0;
}

```

Notice that this is another ***Read-Modify-Write (RMW) operation.***

The CPU does:

```text
Read GPIOB->ODR
        │
        ▼
CPU performs OR with the mask
        │
        ▼
Write the new value back to GPIOB->ODR

```

So, in order to set a particular pin, we have to do the masking, then logical OR with the register.
But what if we want a particular bit to be written or force a bit as 0 instead of 1? That's covered below: Clearing a bit.

### 9.2 Clear a Single Pin (Write 0)

If OR can set a bit, then could AND clear a bit? But it would force other bits to 0 as well. So we have to use AND along with `~` (Bitwise NOT).

Let's consider that we would like to clear bit 3:

```c
GPIOB->ODR &= ~(1U << 3);

```

We'll first answer:

* Why do we create `(1U << 3)`?
* Why do we invert it using `~`?
* Why does `&=` clear only one bit while leaving all the others unchanged?

**Step 1: Create the mask**

First create the normal mask: `(1U << 3)`

`0000 1000` — This identifies bit 3. But we need the opposite, so we invert it using Bitwise NOT `~`:

`After ~: ~(1U << 3) = 1111 0111`

**Step 2: Perform AND**

Suppose: ODR = `1010 1101` (Clear bit 3)

```text
ODR         = 1010 1101
~(1U << 3)  = 1111 0111
------------------------
ODR & Mask  = 1010 0101

```

Before ODR was `1010 1101` and now ODR is `1010 0101`. Exactly bit 3 is set to 0 / cleared and all other bits remain the same.

**Complete Example: Clear a single pin**

```c
int main(void)
{
    // Clear a single pin GPIOB_ODR PIN 5
    GPIOB->ODR &= ~(1U << 5);

    return 0;
}

```

### 9.3 Read a Single Pin

Suppose the GPIOB IDR register currently contains:

```text
Bit:  7   6   5   4   3   2   1   0
Pin: PB7 PB6 PB5 PB4 PB3 PB2 PB1 PB0
Val:  0   0   1   0   1   0   0   0

```

From this register we can say:

* PB5 = HIGH
* PB3 = HIGH
* All other pins = LOW

Suppose, if we wanted to know if PB5 is HIGH, do we need to read all 32 bits? Definitely not! Because we just want to know about bit / pin 5. Hence we need to look exactly at pin 5 and ignore the other 31 bits.

Bitwise AND is used to read a particular bit.

**Example:**

Suppose: `IDR = 1011 0100` and we want to read pin 5.

* What is the mask?
* What is the result of `IDR & mask`?
* Is Pin 5 HIGH or LOW?

**Step 1: Create the mask**

We need to read Pin 5, so `(1U << 5)`:

`(1U << 5) = 0010 0000`

**Step 2: Perform AND**

```text
IDR         = 1011 0100
Mask        = 0010 0000
-----------------------
IDR & Mask  = 0010 0000

```

**Step 3: Interpret the result**

The result is: `0010 0000`

This is:

* Not zero
* Therefore Bit 5 must be 1

So: PB5 = HIGH

Note: The AND operation preserves the original bit position.

**Complete Example: Read a single pin**

```c
int main(void)
{
    // Read a single pin GPIOC_IDR PIN 5
    if (GPIOC->IDR & (1U << 5))
    // or if ((GPIOC->IDR & (1U << 5)) != 0)
    {
        // pin5 is 1
    } 
    else
    {
        // pin5 is 0
    }

    return 0;
}

```

### 9.4 Toggle a Single Pin

Toggle means:

* If bit is 0 → make it 1
* If bit is 1 → make it 0

So, what operator toggles? It's XOR (`^`).

**XOR Truth table again:**

| A | B | A ^ B |
| --- | --- | --- |
| 0 | 0 | 0 |
| 0 | 1 | 1 |
| 1 | 0 | 1 |
| 1 | 1 | 0 |

The important part: When XORing with 1:

* `0 ^ 1 = 1`
* `1 ^ 1 = 0`

So XOR with 1 flips the bit.

**Example:**

Suppose `ODR = 1010 0101` and we want to toggle bit 2: `GPIOB->ODR ^= (1U << 2);`

**Step 1: Create a mask**

`(1U << 2) = 0000 0100`

**Step 2: Now XOR**

```text
ODR         = 1010 0101
Mask        = 0000 0100
-----------------------
ODR ^ Mask  = 1010 0001    (only bit 2 was changed from 1 to 0)

```

**Complete Example: Toggle a single pin**

```c
int main(void)
{
    // Toggle a single pin GPIOB_ODR PIN 2
    GPIOB->ODR ^= (1U << 2);

    return 0;
}

```

Flow:

```text
Read ODR
   |
   v
XOR selected bit
   |
   v
Write back to ODR

```

---

## 10. BSRR Register

### Why ODR Read-Modify-Write is risky

Until now we used: `GPIOB->ODR |= (1U << 5);` to set a pin. 
To understand why BSRR exists, we first need to see why modifying `ODR` directly can lead to dangerous bugs.
When we write code to set a single bit using `ODR`:

Internally, this is:

```text
Read ODR
    |
    v
Modify bit 5
    |
    v
Write back ODR

```

This is called a **Read-Modify-Write operation**.

The problem is that it requires multiple CPU operations. Between the read and write operation, an interrupt or another part of the program could modify the same register.

Example:

```text
Initial ODR: 0000 0000 0000 0000 0000 0000 0000 0000

CPU wants to set PB5: GPIOB->ODR |= (1U << 5);
CPU reads: 0000 0000

Before CPU writes back, an interrupt changes PB2.
Now the CPU writes its old modified value. The PB2 change can be lost!

```

For this reason, STM32 provides: **BSRR (Bit Set Reset Register)**.

### BSRR Register Layout

GPIO BSRR is a 32-bit register.

```text
31                 16 15                 0
+--------------------+--------------------+
|     Reset bits     |      Set bits      |
+--------------------+--------------------+

```

* **Lower 16 bits (Bits 0–15):** Writing 1 sets the corresponding GPIO pin HIGH.
* **Upper 16 bits (Bits 16–31):** Writing 1 resets the corresponding GPIO pin LOW.

### Bit Set operation

Suppose we want to set PB5 HIGH.

Instead of: `GPIOB->ODR |= (1U << 5);`

we use: `GPIOB->BSRR = (1U << 5);`

The value written: `0x00000020` (Bit 5 of BSRR is 1).

Hardware action:

```text
BSRR bit 5 = 1
        |
        v
GPIOB Pin 5 becomes HIGH

```

No read operation happens.

```text
CPU
 |
 | Write
 v
BSRR register
 |
 v
GPIO pin changes

```

This is an atomic operation.

### Bit Reset operation

To clear PB5:

We use the upper half of BSRR. Reset bits start from bit 16.

Formula: **Reset bit position = Pin number + 16**

For PB5: `5 + 16 = 21`

So: `GPIOB->BSRR = (1U << 21);` or `GPIOB->BSRR = (1U << (5 + 16));`

Hardware action:

```text
BSRR bit 21 = 1
        |
        v
GPIOB Pin 5 becomes LOW

```

### Atomic GPIO control comparison

| Operation | ODR method | BSRR method |
| --- | --- | --- |
| Set pin | `ODR |= mask` | `BSRR = mask` |
| Clear pin | `ODR &= ~mask` | `BSRR = (1U << (pin+16))` |
| CPU operation | Read + Modify + Write | Single Write |
| Atomic | No | Yes |
| Recommended | Sometimes | Preferred |

### Complete Example

```c
int main(void)
{
    // Set PB5 HIGH
    GPIOB->BSRR = (1U << 5);

    // Reset PB5 LOW
    GPIOB->BSRR = (1U << (5 + 16));

    return 0;
}

```

---
Now that we see how we can set and reset a pin using BSRR, let's drive down to see all the different ways we can manipulate GPIO state using this register.
Below are all **6 precise use cases** of BSRR, showing the C implementation along with the exact 32-bit register layout in binary for each.

---

### Case 1: Set a Single Pin (e.g., PA5 HIGH)

To set PA5 HIGH, send `1` to **Bit 5** ($S5$).

```c
GPIOA->BSRR = (1U << 5);

```

```text
Bit 31                                 Bit 5          Bit 0
  │                                      │              │
  0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0

```

* **Result:** PA5 goes **HIGH**. All other pins stay unchanged.

---

### Case 2: Reset a Single Pin (e.g., PA5 LOW)

To reset PA5 LOW, send `1` to **Bit 21** ($R5$, which is $5 + 16$).

```c
GPIOA->BSRR = (1U << (5 + 16)); // or (1U << 21)

```

```text
Bit 31               Bit 21                    Bit 0
  │                    │                         │
  0 0 0 0 0 0 0 0 0 0  1  0 0 0 0 0 0 0 0 0 0 0 0 0

```

* **Result:** PA5 goes **LOW**. All other pins stay unchanged.

---

### Case 3: Set Multiple Pins (e.g., PA1, PA3, PA5 HIGH)

Combine them using bitwise OR (`|`) in the lower half (Bits 0–15).

```c
GPIOA->BSRR = (1U << 1) | (1U << 3) | (1U << 5);

```

```text
Bit 31                               Bit 5   Bit 3   Bit 1  Bit 0
  │                                    │       │       │      │
  0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1   0   1   0   1   0

```

* **Result:** PA1, PA3, and PA5 go **HIGH** at the exact same instant.

---

### Case 4: Reset Multiple Pins (e.g., PA1, PA3, PA5 LOW)

Combine them using bitwise OR (`|`) in the upper half (Bits 16–31).

* Pin 1 Reset = Bit $1 + 16 = \text{Bit } 17$
* Pin 3 Reset = Bit $3 + 16 = \text{Bit } 19$
* Pin 5 Reset = Bit $5 + 16 = \text{Bit } 21$

```c
GPIOA->BSRR = (1U << 17) | (1U << 19) | (1U << 21);

```

```text
Bit 31       Bit 21  Bit 19  Bit 17                         Bit 0
  │            │       │       │                              │
  0 0 0 0 0 0  1   0   1   0   1   0 0 0 0 0 0 0 0 0 0 0 0 0 0

```

* **Result:** PA1, PA3, and PA5 go **LOW** at the exact same instant.

---

### Case 5: Set and Reset ONE Pin in the Same Command (Collision Edge-Case)

What if we accidentally write a `1` to both Set ($S5$) and Reset ($R5$) for PA5 at the same time?

```c
GPIOA->BSRR = (1U << 5) | (1U << 21);

```

```text
Bit 31               Bit 21                  Bit 5          Bit 0
  │                    │                       │              │
  0 0 0 0 0 0 0 0 0 0  1  0 0 0 0 0 0 0 0 0 0  1  0 0 0 0 0 0 0

```

* **STM32 Hardware Priority Rule:** According to the reference manual, if both **BSx** and **BRx** are set for the same pin, **Bit Set (BSx) takes priority**.
* **Result:** PA5 goes **HIGH**.

---

### Case 6: Simultaneous Set and Reset of Different Pins (Atomic Parallel Control)

Set PA0 and PA2 **HIGH**, while pulling PA1 and PA3 **LOW** simultaneously in a single clock cycle.

* Set Pins: Bit 0 ($S0$), Bit 2 ($S2$)
* Reset Pins: Bit 17 ($R1 = 1 + 16$), Bit 19 ($R3 = 3 + 16$)

```c
GPIOA->BSRR = (1U << 0) | (1U << 2) | (1U << 17) | (1U << 19);

```

```text
Bit 31       Bit 19  Bit 17                         Bit 2 Bit 0
  │            │       │                              │     │
  0 0 0 0 0 0  1   0   1   0 0 0 0 0 0 0 0 0 0 0 0 0  1  0  1

```

* **Result:** PA0 and PA2 switch **HIGH**, and PA1 and PA3 switch **LOW** at the exact same physical instant. This is incredibly useful for driving parallel buses (e.g., LCD lines).

---

### Complete Code Example for All BSRR Use Cases

Below is the complete executable-style C code illustrating all 6 use cases of the BSRR register inside a standard peripheral context:

```c
/*
  03_gpio_bsrr_all_use_cases.c
  Complete demonstration of all 6 BSRR operations on STM32 GPIO
*/

#include <stdint.h>

typedef struct
{
    volatile uint32_t MODER;      
    volatile uint32_t OTYPER;     
    volatile uint32_t OSPEEDR;    
    volatile uint32_t PUPDR;      
    volatile uint32_t IDR;        
    volatile uint32_t ODR;        
    volatile uint32_t BSRR;       
    volatile uint32_t LCKR;       
    volatile uint32_t AFRL;       
    volatile uint32_t AFRH;       
} GPIO_TypeDef;

#define GPIOA ((GPIO_TypeDef *) 0x40020000U)

int main(void)
{
    /* Use Case 1: Set a Single Pin HIGH */
    // Atomic set PA5 to 1
    GPIOA->BSRR = (1U << 5);

    /* Use Case 2: Reset a Single Pin LOW */
    // Atomic reset PA5 to 0 (Pin 5 + 16 offset = Bit 21)
    GPIOA->BSRR = (1U << (5 + 16));

    /* Use Case 3: Set Multiple Pins HIGH simultaneously */
    // Sets PA1, PA3, and PA5 to 1 in a single write operation
    GPIOA->BSRR = (1U << 1) | (1U << 3) | (1U << 5);

    /* Use Case 4: Reset Multiple Pins LOW simultaneously */
    // Resets PA1, PA3, and PA5 to 0 (Bits 17, 19, and 21)
    GPIOA->BSRR = (1U << (1 + 16)) | (1U << (3 + 16)) | (1U << (5 + 16));

    /* Use Case 5: Collision handling (Set and Reset target the SAME pin) */
    // Hardware resolves conflict by prioritizing Set over Reset -> PA5 goes HIGH
    GPIOA->BSRR = (1U << 5) | (1U << (5 + 16));

    /* Use Case 6: Parallel control (Set some pins while Resetting others) */
    // Sets PA0 & PA2 HIGH while resetting PA1 & PA3 LOW at the exact same instant
    GPIOA->BSRR = (1U << 0) | (1U << 2) | (1U << (1 + 16)) | (1U << (3 + 16));

    while (1)
    {
        // Application loop
    }

    return 0;
}

```

---

## 11. Key Takeaways

* A peripheral is a collection of registers.
* Each register has an offset from the peripheral base address.
* A C structure can represent this register layout.
* The structure does not contain hardware registers; a pointer connects the structure to the real memory address.
* The compiler calculates member offsets based on structure layout and member sizes.
* **Same structure + different base address = different peripheral instance.**
* ODR is used when we need to read or manipulate the complete output register.
* Bitwise operations on ODR are Read-Modify-Write operations.
* BSRR is provided by STM32 for atomic pin set and reset operations.
* For controlling individual GPIO pins, BSRR is safer and preferred over modifying ODR directly.

---