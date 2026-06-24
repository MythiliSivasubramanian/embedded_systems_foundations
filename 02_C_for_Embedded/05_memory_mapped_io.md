# Memory-Mapped I/O in Embedded C

In embedded systems, peripherals are often controlled through memory-mapped registers.
That means a hardware device appears as a block of addresses in the processor's address space.
Firmware reads and writes those addresses using pointers and structures.

## What is memory-mapped I/O?

Memory-mapped I/O is a model where hardware registers are accessed like normal memory.
Each register has a fixed address and a defined width.

For example, a GPIO peripheral may expose:

- a data register at address 0x40020000
- a direction register at address 0x40020004
- a status register at address 0x40020008

The firmware accesses those registers through pointers or through a C struct overlay.

## Using pointers for register access

The simplest form of access is a pointer cast.


#define GPIO_DATA_ADDR 0x40020000U

volatile uint32_t * const GPIO_DATA = (volatile uint32_t *)GPIO_DATA_ADDR;

void gpio_set_pin(void) {
    *GPIO_DATA = 0x00000020U;
}


This pattern is useful for single registers, but it becomes hard to manage when a peripheral has many registers.

## Using structs to represent peripheral registers

A more scalable approach is a register map structure.


typedef struct {
    volatile uint32_t DATA;
    volatile uint32_t DIR;
    volatile uint32_t STATUS;
} GPIO_Registers;

#define GPIO ((GPIO_Registers *)0x40020000U)


Now firmware can use named fields instead of manual address arithmetic.


void gpio_init(void) {
    GPIO->DIR = 0xFF;
}

void gpio_write(uint32_t value) {
    GPIO->DATA = value;
}


## Why volatile is essential here

Hardware registers can change value independently of program flow.
Without volatile, the compiler may optimize away reads or buffer writes incorrectly.

Example:


if (GPIO->STATUS & 0x01) {
    // wait until the status bit is set
}


The read of GPIO->STATUS must happen each time, so the pointer field must be volatile.

## Struct layout and alignment

Register structs must match the hardware layout exactly.
That means:

- fields appear in the same order as in hardware
- each field uses the exact width of the register
- padding does not change the compiled layout

Use fixed-width types from <stdint.h> and avoid packed structs unless necessary.

Example:


typedef struct {
    volatile uint32_t CTRL;
    volatile uint32_t STATUS;
    volatile uint32_t DATA;
} UART_Registers;


If the hardware includes reserved gaps, represent them explicitly:


typedef struct {
    volatile uint32_t CTRL;
    volatile uint32_t STATUS;
    uint32_t RESERVED;
    volatile uint32_t DATA;
} UART_Registers;


## Register definitions in headers and source files

A common pattern is to place the struct and base address in a header, and keep the actual pointer definition as a macro.


/* uart.h */
#include <stdint.h>

typedef struct {
    volatile uint32_t CR;
    volatile uint32_t SR;
    volatile uint32_t DR;
} UART_Registers;

#define UART0 ((UART_Registers *)0x40011000U)


In source code, use the macro directly:


void uart_send(uint8_t byte) {
    while (!(UART0->SR & (1U << 7))) {
        ; // wait until transmit buffer is empty
    }
    UART0->DR = byte;
}


## Practical guidelines

- Use volatile for all register fields.
- Use <stdint.h> types for fixed-width precision.
- Represent reserved gaps explicitly when needed.
- Keep the register map close to the hardware reference manual.
- Prefer named fields over raw pointer arithmetic for readability.

Memory-mapped I/O is the core mechanism for talking to hardware in bare-metal embedded C. A clean register map makes firmware easier to read and reduces mistakes when working with peripheral registers.
