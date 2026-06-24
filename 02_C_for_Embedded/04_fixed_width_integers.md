# Fixed-Width Integer Types in Embedded C

Embedded firmware must often reason about exact memory layouts and hardware register sizes. Fixed-width integer types such as uint8_t, int16_t, and uint32_t give that precision explicitly.

## Why fixed-width types are important

General C types like int and long can vary in size across compilers and platforms.

In embedded systems, that variability is usually unacceptable because:

- hardware registers have a fixed width
- communication protocols require exact byte sizes
- memory layout must be predictable for structures and buffers
- low-level bit masks depend on known integer widths

Fixed-width types from <stdint.h> remove ambiguity.

## Common fixed-width types

- uint8_t — 8-bit unsigned integer
- int8_t — 8-bit signed integer
- uint16_t — 16-bit unsigned integer
- int16_t — 16-bit signed integer
- uint32_t — 32-bit unsigned integer
- int32_t — 32-bit signed integer
- uint64_t — 64-bit unsigned integer
- int64_t — 64-bit signed integer

Use unsigned types when working with raw bits or hardware registers, because they avoid implementation-defined sign behavior during shifts and bit operations.

## Fixed-width types in hardware register definitions

When defining peripheral registers, the width should match the register size exactly.

Example:

c
#define UART_BASE 0x40011000U

typedef struct {
    volatile uint32_t DR;
    volatile uint32_t SR;
    volatile uint32_t CR;
} UART_Registers;

#define UART ((UART_Registers *)UART_BASE)


Using uint32_t makes the register layout clear and avoids surprises from platform-dependent type sizes.

## Why uint32_t is often better than unsigned int

On one MCU, unsigned int may be 16 bits, and on another it may be 32 bits.

That variation can break code that assumes register widths or buffer sizes are fixed.

If a register is 32 bits wide, use uint32_t.

If a value is always 8 bits wide, use uint8_t.

## Fixed-width types in data structures

Precise structure layout is essential for memory-mapped data and communication formats.

Example:


typedef struct {
    uint8_t command;
    uint8_t status;
    uint16_t length;
    uint32_t data;
} PacketHeader;


This definition makes the size of each field explicit and avoids platform-dependent padding assumptions.

## When to choose signed vs unsigned

- Use unsigned types for raw hardware values, flags, and bitfields.
- Use signed types for arithmetic where negative values are meaningful.

Example:

c
int16_t temperature_celsius;
uint16_t adc_reading;


## Practical rules for embedded C

- Prefer <stdint.h> types like uint8_t, int16_t, uint32_t.
- Avoid char, short, int, and long when exact width matters.
- Use size_t only for sizes and memory lengths, not for hardware registers.
- Match the integer width to the hardware or protocol requirement.

Using fixed-width integers consistently helps make embedded code portable, predictable, and easier to review.
