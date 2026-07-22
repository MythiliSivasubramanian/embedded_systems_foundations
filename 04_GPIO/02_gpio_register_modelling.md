# GPIO Register Modelling using Structures, Unions and Pointers

Today I learned how C can represent actual hardware registers inside a microcontroller. This was the first time I felt the connection between C programming and embedded hardware become very clear.

## Why model hardware registers?

### The main idea

A GPIO register inside the STM32 is nothing more than a fixed memory location. Every bit inside that register has a meaning. Instead of using bit masks everywhere, C allows us to model the register in a much more readable way.

The goal is to access both:

- Individual GPIO pins
- The complete register

while referring to the same hardware memory.

## Modelling individual GPIO pins using Bit-fields

Each GPIO output pin occupies only one bit inside the Output Data Register (ODR).

Instead of allocating an entire integer for every pin, bit-fields allow each field to occupy exactly one bit inside a 32-bit storage unit.

Example:

```c
struct GPIO_ODR_BITS
{
    unsigned int PA0  : 1;
    unsigned int PA1  : 1;
    ...
    unsigned int PA15 : 1;

    unsigned int reserved : 16;
};
```

The important realization was that:

- `:1` determines that each field occupies one bit.
- `unsigned int` determines the storage unit used by the compiler.
- The compiler packs all these bit-fields into one 32-bit storage area.

The bit-fields are not separate variables. They are simply named views of individual bits inside the same storage unit.

**Caveat:** bit-field layout and ordering are implementation-defined in C. For STM32 with GCC, this model typically matches the hardware layout, but the C standard does not guarantee it on every compiler or architecture.

## Why include the reserved bits?

The STM32F407 GPIO Output Data Register (GPIOx_ODR) is a 32-bit register.

According to the Reference Manual:

- Bit 0 to Bit 15 are Read/Write.
- Bit 16 to Bit 31 are Reserved and should remain at their reset value.

To model the complete hardware register, the reserved bits are also represented.

```c
unsigned int reserved : 16;
```

This makes the C structure match the actual register layout.

> Note: the reserved bits are included only to preserve the register size and layout. They should not be used directly, and the field exists to ensure the bit-field model matches the full 32-bit register.

## Using a Union for Two Different Views

### Two views of the same register

A union allows multiple members to share the same memory.

```c
union GPIO_ODR_REGISTER
{
    unsigned int value;
    struct GPIO_ODR_BITS bits;
};
```

Both members occupy the same 32-bit storage.

This allows the register to be accessed in two different ways.

### Example usage

This example shows the same register being used in two different ways.

```c
volatile union GPIO_ODR_REGISTER *ODR = (volatile union GPIO_ODR_REGISTER *)0x40020014;

/* write the whole register */
ODR->value = 0x00000020;

/* write only one pin */
ODR->bits.PA5 = 1;
```

Nothing is copied between `value` and `bits`.

They simply provide two different views of the same memory.

## Connecting the C model to Hardware

A pointer is used to connect the C representation to the actual hardware register.

```c
union GPIO_ODR_REGISTER *ODR;
```

The pointer itself only stores an address.

The hardware address is obtained from the Reference Manual.

Instead of memorizing register addresses, they can be calculated as:

```text
Register Address = Peripheral Base Address + Register Offset
```

For GPIOA Output Data Register:

```text
GPIOA Base Address = 0x40020000
ODR Offset         = 0x14

GPIOA_ODR Address  = 0x40020014
```

The address can then be assigned to the pointer using a type cast.

```c
ODR = (union GPIO_ODR_REGISTER *)0x40020014;
```

The cast tells the compiler to interpret this numeric address as a pointer to the register model.

## GPIO General Purpose Input Output

An STM32 MCU contains a CPU and hardware peripherals such as GPIO. The CPU does not drive pins directly; instead it writes a register and the GPIO hardware changes the pin state.

```text
                     STM32 MCU

          +----------------------+ 
          |                      |
          |       CPU            |
          |                      |
          |   GPIO Peripheral    |
          |                      |
          +----------------------+ 

       PA0 PA1 PA2 PA3 ... PB0 PB1 ...
               │
               │
          Physical Pins
```

Those metal legs around the MCU are physical pins. They are how the MCU communicates with LEDs, buttons, sensors, motors, LCDs, UART, SPI, and more.

For example, to turn on an LED connected to PA5:

```text
      VCC
       │
      LED
       │
PA5 -------- STM32
```

The CPU does not directly send power to PA5. Instead the CPU updates a GPIO register and the GPIO peripheral hardware changes the voltage on the pin.

```text
CPU
 ↓
GPIO Peripheral
 ↓
Physical Pin
 ↓
LED
```

When you execute:

```c
GPIOA->ODR |= (1U << 5);
```

the CPU writes to the GPIO register. The GPIO hardware sees the changed bit and drives PA5 high.

### Where are these registers?

Hardware peripherals also have addresses. The GPIO registers are mapped into the processor address space, just like RAM.

```text
0x40020000  ---> GPIOA MODER Register
0x40020014  ---> GPIOA ODR Register
```

These are not normal RAM variables. They are special addresses connected to hardware, a concept called Memory-Mapped I/O.

So the simpler pointer form:

```c
#define GPIOA_ODR (*(volatile uint32_t *)0x40020014)
```

is conceptually the same as the register struct access in vendor headers.

```c
GPIOA->ODR |= (1U << 5);
GPIOA_ODR |= (1U << 5);
```

Both write the same bit to the same hardware register when the underlying address matches.

## Understanding volatile

One of the biggest lessons today was understanding why `volatile` exists.

A normal variable can only be changed by my program.

A hardware register, however, may change because of:

- Interrupts
- Hardware peripherals
- Other parts of the system

Therefore, the compiler must not assume the value remains unchanged.

`volatile` does not change the hardware.

It tells the compiler:

> This memory location may change unexpectedly. Every read and write must access memory directly instead of using an optimized cached value.

This is essential when working with memory-mapped peripheral registers.

There are actually four different declarations:

- `volatile int *p;`
- `int * volatile p;`
- `volatile int * volatile p;`
- `int *p;`

They all mean something different.

#### Deep understanding of the four volatile pointer declarations:

##### Case 1: `volatile int *p;`
Meaning:

- `p ---> volatile integer`
- The address is normal.
- The data is volatile.

##### Case 2: `int * volatile p;`
Meaning:

- `volatile p ---> normal integer`
- The pointer address itself may change.

Example use:

- An interrupt changes where a pointer points.

##### Case 3: `volatile int * volatile p;`
Meaning:

- `volatile p ---> volatile integer`
- Both can change.

##### Case 4: `int *p;`

Meaning:

- `normal pointer ---> normal integer`

So now,
`volatile union GPIO_ODR_REGISTER *ODR;` is case 1.

The register contents are volatile. The pointer is not.

## What I Learned

Before this lesson, I understood structures, unions, pointers and bit-fields individually.

After combining them, I now understand how embedded C models hardware registers.

The connection is:

- Reference Manual
- Peripheral Base Address
- Register Offset
- Register Address
- Pointer
- Union
- Structure
- Bit-fields
- Hardware Register

This was the first time I could clearly see how C interacts directly with STM32 hardware instead of treating registers as “magic addresses”.

The following code combines all the concepts learned in this chapter.

This example brings all the ideas together in one complete program.

It demonstrates how:

* Bit-fields model individual GPIO pins.
* A structure represents the GPIO Output Data Register layout.
* A union provides two views of the same 32-bit register.
* A pointer connects the C representation to the actual hardware register address.

```c
#include <stdio.h>

// GPIO_ A Output Data Register  STM32F407VG 
// Bit 0  - Bit 15 : Output Data Bits Read/Write
// Bit 16 - Bit 31 : Reserved (must keep at reset value)

struct GPIO_ODR_BITS
{
    /* Each field occupies 1 bit inside the 32-bit storage unit. 
    The compiler does not allocate a separate integer for each pin.
    */

    unsigned int PA0  : 1;
    unsigned int PA1  : 1;
    unsigned int PA2  : 1;
    unsigned int PA3  : 1;
    unsigned int PA4  : 1;
    unsigned int PA5  : 1;
    unsigned int PA6  : 1;
    unsigned int PA7  : 1;
    unsigned int PA8  : 1;
    unsigned int PA9  : 1;
    unsigned int PA10 : 1;
    unsigned int PA11 : 1;
    unsigned int PA12 : 1;
    unsigned int PA13 : 1;
    unsigned int PA14 : 1;
    unsigned int PA15 : 1;

    // Reserved bits (Bit 16 - Bit 31)
    unsigned int reserved : 16;
};

// Union provides two views of the same 32-bit register.
/*
Size of structure 4 bytes / 32 bits, but with bit fields since all members are of same type unsigned int , complier allocates 4 // bytes (32 bits) based on the base type unsigned int.
*/

union GPIO_ODR_REGISTER
{
    // Access entire register.
    unsigned int value;

    // Access individual GPIO pins.
    struct GPIO_ODR_BITS bits;
};

// Pointer to the GPIO Output Data Register.
volatile union GPIO_ODR_REGISTER *ODR;

int main(void)
{
    // GPIO_A Base Address = 0x40020000
    // GPIO_ODR Offset    = 0x14
    // GPIOA_ODR Address  = 0x40020014
    ODR = (volatile union GPIO_ODR_REGISTER *)0x40020014;

    // Set PA5 HIGH
    ODR->bits.PA5 = 1;

    // Write entire register
    ODR->value = 0x00000020;
    return 0;
}
```

With this ODR = (volatile union GPIO_ODR_REGISTER *)0x40020014;
If we access the register 50 times, then how many times should we write this?
This example shows why writing the address every time becomes inconvenient.
Probably something like:

```c
ODR->bits.PA5 = 1;

...

ODR->value = 0;

...

ODR->bits.PA7 = 1;
```

The question is, suppose we didn't have the pointer variable ODR or a macro.

### Why macros help

Every time we wanted to access the register, we would have to write, (volatile union GPIO_ODR_REGISTER *)0x40020014 everytime which is hard to read, easy to mistype, and difficult to maintain.
Hence we are giving this long expression a short name like GPIOA_ODR which is a macro (without creating a variable).

So #define GPIOA_ODR ((volatile union GPIO_ODR_REGISTER *)0x40020014) is just a name that the preprocessor replaces before compilation. During preprocessing stage in build process, the preprocessor changes GPIOA_ODR to (volatile union GPIO_ODR_REGISTER *)0x40020014. The complier doesnt even know that GPIOA_ODR even existied. Hence, it doesnt create a varibale, no memory allocated in RAM or elsewhere.

Note: Although we use macro, Volatile is used, because the hardware register value at 0x40020014 can change unexpectedly

Now the question arises, cant we access the individual pins (in structure) using a pointer variable? Yes, we can do that, but it would have few disadvantages, compared to using a macro.

- A pointer variable requries memroy allocation in RAM
- when the file is large with many ports (each port has many registers), then it could be messy with large memory allocated.
- Whereas, macro doesnt need a storage. It directly substitutes the expression.

| Pointer variable                | Macro                            |
| ------------------------------- | -------------------------------- |
| Stored as a variable            | Replaced before compilation      |
| Needs memory                    | No memory                        |
| Can be assigned                 | Fixed expression                 |
| Created at runtime in your code | Exists only during preprocessing |

#### Example:

This example shows how the preprocessor turns the macro into the real address expression.

```c
GPIOA_ODR->bits.PA5 = 1;
```

The preprocessor transforms it into:

```c
((volatile union GPIO_ODR_REGISTER *)0x40020014)->bits.PA5 = 1;
```

Then the compiler generates machine code. The macro itself does not exist anymore.

Hence, Macros are commonly used for:

Hardware addresses :
Example:

```c
#define GPIOA_BASE 0x40020000
```

Register addresses :
Example:

```c
#define GPIOA_ODR_ADDRESS (GPIOA_BASE + 0x14)
```

Bit positions :
Example:

```c
#define PIN5 (1U << 5)
```

#### Comparision of two ways : Using Macro and using a pointer variable :

##### 1. Using macro:

```text
C code
     |
     |
     v
Preprocessor
     |
     |  replaces #define names
     |
     v
Compiler
     |
     |
     v
Machine code
     |
     |
     v
CPU writes to address 0x40020014
     |
     |
     v
GPIO hardware changes PA5
```

Now let's compare the two styles you already know.

Style 1: Pointer variable

```c
volatile union GPIO_ODR_REGISTER *ODR;

ODR = (volatile union GPIO_ODR_REGISTER *)0x40020014;

ODR->bits.PA5 = 1;
```

Memory:

```text
RAM

ODR variable
+-------------+
| 0x40020014  |
+-------------+

        |
        v

Hardware register
0x40020014
```

The pointer variable exists in memory.

Style 2: Macro

```c
#define GPIOA_ODR ((volatile union GPIO_ODR_REGISTER *)0x40020014)

GPIOA_ODR->bits.PA5 = 1;
```

Memory:

```text
No GPIOA_ODR variable

GPIOA_ODR
    |
    | replaced before compilation
    v

0x40020014
```

No pointer storage is needed.

### Types of macros

There are two types of macros we'll commonly see in embedded C.

1. Object-like macros
2. Function-like macros

#### 1. Object-like macros

These replace a name with a value or expression.

Example:

```c
#define GPIOA_BASE 0x40020000U
```

or

```c
#define GPIOA_ODR ((volatile union GPIO_ODR_REGISTER *)0x40020014)
```

These don't take any arguments.

#### 2. Function-like macros

These look like functions because they take arguments.

Example:

```c
#define SQUARE(x) ((x) * (x))
```

Usage:

```c
int y = SQUARE(5);
```

The preprocessor replaces it with:

```c
int y = ((5) * (5));
```

Again, no function is called. It is still just text replacement.

Now, which type of macro is this?

```c
#define GPIOA_BASE 0x40020000U
```

A) Object-like macro

B) Function-like macro

#define GPIOA_BASE 0x40020000U is an object-like macro.

Because it has:

- a name (GPIOA_BASE)
- a replacement (0x40020000U)
- no parameters

So the preprocessor simply does:

GPIOA_BASE
      ↓
0x40020000U

Nothing more.

Now look at this:

#define GPIOA_ODR ((volatile union GPIO_ODR_REGISTER *)0x40020014) Is this also an object-like macro? Or is it a function-like macro?

This is also an object-like macro.

Why? Because it has:

- A name: GPIOA_ODR
- A replacement text: ((volatile union GPIO_ODR_REGISTER *)0x40020014)
- No parameters

So the preprocessor performs:

GPIOA_ODR
        ↓
((volatile union GPIO_ODR_REGISTER *)0x40020014)

Again, it is just text replacement.

## 1. Read a pin :

- - How does a C program read the state of a physical GPIO pin? - - 
- - if(GPIOA_IDR->bits.PA5) Lets Understand:
    - What does the CPU actually read?
    - Where did that bit come from?
    - How does pressing a button change the register? - - 
    
    
Basically, the GPIO Pin can be configured either as Input or as output.

Output Mode : 

The MCU controls the pin. 

Example:

```text
CPU
 ↓
GPIO
 ↓
PA5 voltage HIGH/LOW
 ↓
LED
```

Here, C code writes a register. Example: GPIOA_ODR->bits.PA5 = 1;

Input mode :

The outside world controls the pin.

Example :

```text
Button
   ↓
PA0 pin
   ↓
GPIO hardware
   ↓
CPU reads the state
```
Here, C code reads a register. Example: value = GPIOA_IDR->bits.PA0;

So: ODR = Output Data Register. Used when: We want to control a pin.

IDR = Input Data Register. Used when: We want to know the pin state.

Suppose, if we connect a push button to PA0 and want to know whether the button is pressed, should we use:
GPIOA_ODR or  GPIOA_IDR ?

GPIOA_IDR because a button is an external signal. The MCU is not controlling it; the MCU is observing it.

So the path is:

```text
Button
  |
  | voltage changes
  ↓
PA0 physical pin
  |
  ↓
GPIO input circuitry
  |
  ↓
GPIOA_IDR bit 0
  |
  ↓
C code reads it
```

Earlier we modelled ODR:

struct GPIO_ODR_BITS
{
    unsigned int PA0 : 1;
    ...
    unsigned int PA15 : 1;
    unsigned int reserved : 16;
};

For IDR, the hardware layout is almost the same idea.

The STM32 reference manual says:

Bits 0–15 → input data bits
Bits 16–31 → reserved

So we can create:

struct GPIO_IDR_BITS
{
    unsigned int PA0 : 1;
    unsigned int PA1 : 1;
    ...
    unsigned int PA15 : 1;

    unsigned int reserved : 16;
};

Same concept.

Now think about the hardware.

Suppose:

Button is not pressed
PA0 voltage = LOW

The hardware register contains:

GPIOA_IDR

Bit 31                         Bit 0

00000000 00000000 00000000 00000000
                               ^
                               PA0 = 0

The CPU reads:

GPIOA_IDR->bits.PA0

Result:

0

Now button pressed:

Physical voltage changes:

LOW → HIGH

GPIO hardware updates the register:

00000000 00000000 00000000 00000001
                                      ^
                                      PA0 = 1

Now:

GPIOA_IDR->bits.PA0

returns:

1

Notice something important:

The CPU did not write anything.

The hardware changed the register.

This is exactly why volatile matters.

So, When the button is pressed and PA0 changes from 0 → 1, who changed the value inside GPIOA_IDR? Is that the C program or the GPIO hardware based on the electrical signal?

The GPIO hardware based on the electrical signal. A register is not always changed by our C code. Hardware can update registers independently.

In this case:
Button pressed
      ↓
Voltage changes on PA0
      ↓
GPIO input circuitry detects it
      ↓
GPIOA_IDR bit 0 changes
      ↓
CPU reads the new value

The C program only observes the register.

Now let's connect this to volatile again.

Imagine this code:

while(GPIOA_IDR->bits.PA0 == 0)
{
}

It mneans Keep checking PA0 until it becomes HIGH. Without volatile, a compiler would  read PA0 once, it was 0, and the program itself never changes it. so it will keep using the same value 0.

But that is wrong because:
Outside world
      |
      ↓
Hardware
      |
      ↓
GPIOA_IDR changes

Therefore the register must be declared volatile so every read actually accesses the hardware register.

Now let's build the C model.

We already have:

union GPIO_ODR_REGISTER
{
    unsigned int value;
    struct GPIO_ODR_BITS bits;
};

For input, we create another model:

union GPIO_IDR_REGISTER
{
    unsigned int value;
    struct GPIO_IDR_BITS bits;
};

We know the register address comes from:

Peripheral Base Address + Register Offset

For GPIOA:

GPIOA Base Address = 0x40020000
GPIOA IDR Offset   = 0x10
--------------------------------
GPIOA IDR Address  = 0x40020010


## 2. Write a Pin

Before writing any C code, let's understand what writing a pin actually means.
Suppose PA5 is connected to an LED.

```
      3.3V
        │
      (Circuit)
        │
      LED
        │
PA5 ---------------- STM32
```

Earlier, we learned:
```
GPIOA_IDR->bits.PA0    This was reading.
```

The flow was:

```
Button
   │
   ▼
PA0 physical pin
   │
   ▼
GPIO input circuitry
   │
   ▼
GPIOA_IDR bit 0
   │
   ▼
CPU reads it
```

Notice something important: The outside world changed the voltage.

Now let's compare it with writing.
Suppose we execute:

```
CPU
 │
 ▼
Writes GPIOA_ODR bit 5 = 1
 │
 ▼
GPIO peripheral
 │
 ▼
PA5 physical pin becomes HIGH
 │
 ▼
LED turns ON
```

### Key Notes

When we read a pin: The CPU is observing the outside world.

When we write a pin: The CPU is controlling the outside world.

### Example

Suppose when we write: `GPIOA_ODR->bits.PA5 = 1;`

**Who actually makes PA5 become 3.3V?**

A. The CPU directly drives the voltage on the pin.
B. The CPU writes the ODR register, and the GPIO hardware drives the pin HIGH.

The answer is the CPU writes the ODR register, and the GPIO hardware drives the pin HIGH.

When we write: `GPIOA_ODR->bits.PA5 = 1;` the CPU is not electrically connected to PA5.

#### What Actually Happens

```
           STM32 MCU

+----------------------------------+
|                                  |
|      CPU                         |
|        │                         |
|        │ writes                  |
|        ▼                         |
|   GPIO Output Data Register      |
|        │                         |
|        ▼                         |
|    GPIO Peripheral Logic         |
|        │                         |
+--------│-------------------------+
         │
         ▼
     PA5 Physical Pin
         │
         ▼
        LED
```

The CPU's job ends here:

```
CPU
 │
 ▼
GPIOA_ODR bit 5 = 1
```

Then the GPIO peripheral hardware says: "Bit 5 became 1." So it changes the voltage on PA5 to HIGH.

```
CPU
 │
 ▼
GPIO Register
 │
 ▼
GPIO Hardware
 │
 ▼
Pin Voltage Changes
```

**So does the CPU directly control the GPIO pin?**

No. The CPU writes to the GPIO peripheral registers. The GPIO peripheral hardware interprets those register values and drives the physical pin accordingly.

Let's now consider:

```
GPIOA_ODR->bits.PA5 = 1;
```

At this moment, what actually changes inside the register first?

**Is it:**
- A) The entire 32-bit register becomes `0x00000020`
or
- B) Only bit 5 changes from 0 to 1, while all the other bits remain unchanged?

Actually, there are two different levels of understanding:

**Level 1 (C language)**

Our statement: `ODR->bits.PA5 = 1;` means just modify only bit 5.

**Level 2 (Machine instructions)**

The compiler might generate something like this:

1. Read the 32-bit register.
2. Modify bit 5.
3. Write the 32-bit register back.

So the hardware bus may still perform a 32-bit write, even though logically only one bit changed.

> **Note:** This depends on the architecture, compiler, optimization, and peripheral. We'll learn about this part at a later stage.

For now, let's consider Level 1, which is `GPIOA_ODR->bits.PA5 = 1;` where only bit 5 changes from 0 to 1, while all the other bits remain unchanged.

Conceptual flow:

```
CPU
 │
 ▼
Change PA5 bit in ODR
 │
 ▼
GPIO hardware notices bit 5 = 1
 │
 ▼
PA5 pin goes HIGH
 │
 ▼
LED turns ON
```

There are actually two different ways to set pin 5 to HIGH:

1. `ODR->bits.PA5 = 1;` (bit-field approach)
2. `GPIOA->ODR |= (1U << 5);` (bit-mask approach)

Both seem to set PA5 HIGH. So why are there two different ways?

Both methods express the same intention: setting PA5 HIGH. However, they represent the operation differently. One uses a named bit-field, while the other uses bit manipulation with a mask.

**Method 1: Bit-field approach**

```c
ODR->bits.PA5 = 1;
```

Meaning: Go to the PA5 bit field and set it to 1.

**Method 2: Bit-mask approach**

```c
GPIOA->ODR |= (1U << 5);
```

Let's ignore the `|=` for one minute and first understand this:

```
1U << 5
Start:        0000 0001
Shift left 5 positions: 0010 0000
```

So `(1U << 5)` creates a mask:

```
Bit:   7 6 5 4 3 2 1 0
       0 0 1 0 0 0 0 0
             ^
             |
            PA5
```

**Why do we need a mask?**

Imagine the register currently contains: `GPIOA_ODR`

```
Bit15                         Bit0
0000 0000 0000 1000
                   ^
                   PA3 = 1
```

PA3 is already HIGH.

Now we want: `GPIOA->ODR |= (1U << 5);`

**The goal is:**
- PA5 becomes 1
- PA3 remains 1
- All other pins remain unchanged

After operation: `GPIOA->ODR |= (1U << 5);`

```
0000 0000 0010 1000
             ^
             PA5 = 1
                   ^
                   PA3 = 1
```

The important idea: `|` means: **Keep existing 1s, and force this bit to 1.**

So:

```
Existing register: 0000 1000
OR mask:          0010 0000
Result:           0010 1000
```

**Why not simply write?**

Why not: `GPIOA->ODR = 0x20;`

Because this means: **Replace the entire register with this value.**

Example:

Before:

```
PA7 PA6 PA5 PA4 PA3 PA2 PA1 PA0
 0   0   0   0   1   0   0   0
```

Write: `GPIOA->ODR = 0x20;`

After:

```
PA7 PA6 PA5 PA4 PA3 PA2 PA1 PA0
 0   0   1   0   0   0   0   0
```

PA3 was lost!

**The difference:**

- `GPIOA->ODR = (1U << 5);` means: Replace the whole register.
- `GPIOA->ODR |= (1U << 5);` means: Set bit 5, keep everything else.

This operation is called: **Read-Modify-Write (RMW)**

Because internally the CPU conceptually does:

```
1. Read ODR
    ↓
2. Modify bit 5
    ↓
3. Write ODR back
```

We will explore this more later when we discuss register operations.

## Clearing a GPIO Pin (Writing LOW)

We already know how to set PA5:

```c
GPIOA->ODR |= (1U << 5);
```

Now we want to clear PA5. The code is:

```c
GPIOA->ODR &= ~(1U << 5);
```

We'll first understand only this part: `~(1U << 5)`

### Step 1: `1U << 5`

```
Bit: 7 6 5 4 3 2 1 0
     0 0 1 0 0 0 0 0
           ↑
          PA5 (only PA5 is 1)
```

### Step 2: Apply ~ (Bitwise NOT)

The `~` operator flips every bit.

**Before:**
```
0010 0000
```

**After:**
```
1101 1111
```

Notice what happened:
- PA5 became 0
- Every other bit became 1

**Visualized:**

Before NOT operation:
```
Bit: 7 6 5 4 3 2 1 0
     0 0 1 0 0 0 0 0
           ↑
          PA5
```

After NOT operation:
```
Bit: 7 6 5 4 3 2 1 0
     1 1 0 1 1 1 1 1
           ↑
          PA5
```

Basically:
- `1U << 5` creates `0010 0000`
- `~(1U << 5)` creates `1101 1111`

### Why Create a Mask with All 1s Except PA5?

Suppose the current register is:

```
0000 0000 0010 1100
```

This means:
- PA5 = 1
- PA3 = 1
- PA2 = 1

Our mask is: `~(1U << 5)`, which (showing 8 bits for simplicity) is: `1101 1111`

Now perform an AND (`&`) operation:

```
Register:    0010 1100
Mask:        1101 1111
             ----------
Result:      0000 1100
```

Changes:
- PA5 changed from 1 → 0
- PA3 stayed 1
- PA2 stayed 1
- Everything else stayed exactly as it was

### Why Does This Work?

The AND (`&`) operator follows a simple rule:

| Register Bit | Mask Bit | Result |
| ------------ | -------- | ------ |
| 0            | 1        | 0      |
| 1            | 1        | 1      |
| 0            | 0        | 0      |
| 1            | 0        | 0      |

Key observations:
- **AND with 1** keeps the original value unchanged
- **AND with 0** forces the result to 0

That's why we build the mask with all 1s except for the bit we want to clear:

```
1101 1111
```

Every bit is 1 except the one we want to clear.

### The Complete Operation

```c
GPIOA->ODR &= ~(1U << 5);
```

Conceptually, this means:

1. Create a mask with only PA5 as 0
2. Read the current register
3. AND the register with the mask
4. Write the result back

**Result:** PA5 is cleared, while every other pin keeps its previous state.

## Toggle a GPIO Pin

The code is:

```c
GPIOA->ODR ^= (1U << 5);
```

Ignore `^=` for now. Let's first understand the mask: `1U << 5`

We already know it creates:

```
Bit: 7 6 5 4 3 2 1 0
     0 0 1 0 0 0 0 0
           ↑
          PA5
```

Exactly the same mask we used for Set and Clear. But what's different? **The operator.**

Earlier we used:
- `|` for Set
- `&` for Clear
- Now we use: `^` for Toggle

### XOR (Exclusive OR) Operation

The XOR rule:

| Register Bit | Mask Bit | Result |
| ------------ | -------- | ------ |
| 0            | 0        | 0      |
| 0            | 1        | 1      |
| 1            | 0        | 1      |
| 1            | 1        | 0      |

**How XOR Works:**
- XOR (`^`) toggles the selected bit
- If the bit is 0, it becomes 1
- If the bit is 1, it becomes 0
- Bits corresponding to 0 in the mask remain unchanged

### Summary of GPIO Operations

| Operation | Code                        | Meaning           |
| --------- | --------------------------- | ----------------- |
| Set       | `GPIOA->ODR \|= (1U << 5);` | Force PA5 to `1`  |
| Clear     | `GPIOA->ODR &= ~(1U << 5);` | Force PA5 to `0`  |
| Toggle    | `GPIOA->ODR ^= (1U << 5);`  | Flip PA5          |

### Key Insights

- **OR (`|`)** with a mask sets the selected bit while leaving all other bits unchanged
- **AND (`&`)** with an inverted mask (`~mask`) clears the selected bit while leaving all other bits unchanged
- **XOR (`^`)** with a mask toggles the selected bit while leaving all other bits unchanged


## Read the Complete Register :
Until now, we've been reading one pin.

Example: GPIOA_IDR->bits.PA0

This returns only one bit:
0 → LOW
1 → HIGH

Now suppose instead of reading just PA0, you want to know the state of all 16 GPIO pins at once.

Remember your union:

union GPIO_IDR_REGISTER
{
    unsigned int value;
    struct GPIO_IDR_BITS bits;
};

This gives you two views of the same register:

**View 1: Individual pins**
IDR->bits.PA0
IDR->bits.PA1
IDR->bits.PA2

Each accesses one bit.

**View 2: Entire register**
IDR->value
This accesses the entire 32-bit register in one operation.

Example : 

Suppose the hardware register contains:

Bit15                     Bit0

0000 0000 0010 0101

Then: IDR->value returns: 0000 0000 0010 0101 as a single 32-bit unsigned integer.

Suppose the register contains: 0000 0000 0010 0101 Which pins are HIGH?  Pin 0, pin 2 and pin 5 are HIGH

Now let's connect this to our union.

We have:

union GPIO_IDR_REGISTER
{
    unsigned int value;
    struct GPIO_IDR_BITS bits;
};

Suppose:

unsigned int x = IDR->value; and the register contains: 0000 0000 0010 0101

Then after executing: unsigned int x = IDR->value;

conceptually:

Hardware Register (GPIOA_IDR)

0000 0000 0010 0101
         │
         │ Read whole register
         ▼
x

0000 0000 0010 0101

Now x is an ordinary C variable stored in RAM.

Notice the difference:

IDR->value → hardware register
x → normal RAM variable

From this point onward, if the hardware changes the register, x does not automatically change because it is just a copy.

Suppose: unsigned int x = IDR->value;

At that moment: IDR = 0000 0000 0010 0101

A moment later, the user presses another button and the hardware changes the register to: 0000 0000 0010 0111

What is the value of x now? Will x also become 0000 0000 0010 0111 or will it remain 0000 0000 0010 0101 ?

Suppose we do: unsigned int x = IDR->value;

Imagine the hardware register contains:

0000 0000 0010 0101

The CPU performs one read:

Hardware Register
        │
        ▼
Read value
        │
        ▼
Copy into x

Now memory looks like:

Hardware Register

0000 0000 0010 0101


RAM

x

0000 0000 0010 0101

These are now two separate places.

If the hardware later changes the register:

Hardware Register

0000 0000 0010 0111

then:

RAM

x

0000 0000 0010 0101

x remains unchanged.

## Write a complete register :

Earlier, we wrote one pin: ODR->bits.PA5 = 1; to set only PA5 HIGH.

Now let's look at: ODR->value = 0x00000020;

Notice the difference. Here we're not accessing bits.
We're accessing:

ODR->value

which represents the entire 32-bit register.

Suppose the register initially contains
Bit15                     Bit0

0000 0000 0000 1000
This means:

PA3 = HIGH. All other pins = LOW

Now execute: ODR->value = 0x00000020;

Remember: 0x20 = 32 decimal

Binary:

0000 0000 0010 0000

So after the write, the register becomes:
0000 0000 0010 0000

Notice what happened:

Before:

PA5 = 0
PA3 = 1

After:

PA5 = 1
PA3 = 0

The important difference

When you write: ODR->bits.PA5 = 1; we are changing one bit (conceptually).

When you write: ODR->value = 0x00000020; we are replacing the entire 32-bit register with a new value.

Think of it this way
Bit-field access
Register

0000 1000

       ↓

Change only PA5

       ↓

0010 1000
Whole-register access
Register

0000 1000

       ↓

Write new value

0010 0000

The old contents are completely replaced.

This is why writing the entire register is very powerful—but also something we must use carefully. If another pin was HIGH and we didn't include it in the new value, it will become LOW.

## Toggle the complete register :

So far, we have toggled one pin: GPIOA->ODR ^= (1U << 5); This flips only PA5.

Now suppose we want to toggle every GPIO pin at the same time. Instead of using a mask with one bit set, we use a value where all GPIO bits are 1.

For GPIOA, only bits 0–15 are GPIO pins.

So the mask is: 0x0000FFFF

Binary:
0000 0000 0000 0000 1111 1111 1111 1111

Notice:

Bits 0–15 = 1
Bits 16–31 = 0 (reserved bits are left unchanged)

ODR->value ^= 0x0000FFFF; What does it do?

Suppose the register initially contains:

0000 0000 0000 0000 0000 0000 0010 0101

This means:

PA5 = 1
PA2 = 1
PA0 = 1

Now XOR with:

0000 0000 0000 0000 1111 1111 1111 1111

The result is:
0000 0000 0000 0000 1111 1111 1101 1010

Let's verify the lower 8 bits:

Original : 0010 0101
Mask     : 1111 1111
--------------------
Result   : 1101 1010

Every GPIO bit has flipped.

Why 0x0000FFFF and not 0xFFFFFFFF?

This is an important STM32-specific point.

From the reference manual:

Bits 0–15 → GPIO output pins
Bits 16–31 → Reserved

We should not intentionally modify reserved bits.

So this is preferred: ODR->value ^= 0x0000FFFF; rather than ODR->value ^= 0xFFFFFFFF; because it leaves the reserved bits unchanged.
