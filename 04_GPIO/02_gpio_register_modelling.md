# GPIO Register Modelling using Structures, Unions and Pointers

Today I learned how C can represent actual hardware registers inside a microcontroller. This was the first time I felt the connection between C programming and embedded hardware become very clear.

## Why model hardware registers?

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