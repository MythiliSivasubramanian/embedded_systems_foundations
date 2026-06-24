# Embedded C Storage and Visibility

This file explains the storage-class and type qualifiers that matter most in embedded firmware: volatile, const, static, and extern.

## Why these qualifiers matter in embedded systems

Embedded code often interacts directly with hardware and runs in resource-constrained environments.

That means we must be explicit about:

- whether a variable lives in RAM, flash, or peripheral memory
- whether the compiler may cache or optimize access to a variable
- whether a name is visible only inside one file or across the entire firmware image
- whether a value is fixed at compile time or may change at runtime

Using the right qualifier makes code more correct, safer, and easier to reason about.

## volatile

Use volatile when a value can change outside the normal flow of the C program.

Common examples:

- hardware registers updated by peripherals
- variables changed by interrupt service routines
- memory locations shared with DMA or another processor

Example:


volatile uint32_t * const GPIO_DATA = (uint32_t *)0x40020000;


This tells the compiler:

- do not optimize away repeated reads
- always write through to memory
- treat the pointed-to data as hardware state

Without volatile, the compiler may assume the value cannot change spontaneously and may remove necessary reads or writes.

## const

Use const for values that should not change after initialization.

In embedded systems, const is often used to place data in flash memory and help the compiler catch unintended writes.

Example:


const uint8_t lookup_table[16] = {
    0x00, 0x01, 0x02, 0x03,
    0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F
};


Using const also supports read-only data in flash on systems where RAM is limited.

## static

static changes visibility and lifetime depending on where it appears.

- At file scope, static makes a symbol private to that source file.
- At function scope, static gives a local variable persistent lifetime.

Example:


static uint32_t chip_config;

static void configure_chip(void) {
    static uint8_t retry_count;
    retry_count++;
    chip_config = 0xA5A5;
}


In embedded code, file scope static is often used to keep helper functions and internal state hidden from other modules.

## extern

Use extern to declare a symbol that is defined in another source file.

This is useful for separating hardware definitions and shared state while keeping the actual storage in one place.

Example in gpio.c:


#include <stdint.h>

uint32_t system_status;


Example in main.c:


#include <stdint.h>

extern uint32_t system_status;


extern does not allocate storage. It only tells the compiler the symbol exists elsewhere.

## Combined usage patterns

These qualifiers often appear together in embedded headers and source files.

Example:


/* hardware.h */
extern volatile uint32_t * const TIMER_CTRL;

/* hardware.c */
volatile uint32_t * const TIMER_CTRL = (uint32_t *)0x40010000;


Example with read-only flash and private state:


static const uint8_t boot_signature[8] = {0xDE, 0xAD, 0xBE, 0xEF};


## Practical guidance

- Mark peripheral registers volatile.
- Mark lookup tables and firmware constants const.
- Mark internal module state static at file scope.
- Use extern only when you need shared access across translation units.

Keeping these qualifiers consistent helps avoid subtle bugs from compiler optimizations, unintended symbol exports, and improper memory placement.
