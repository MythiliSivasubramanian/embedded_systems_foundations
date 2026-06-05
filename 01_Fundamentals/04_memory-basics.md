# Memory Basics

Memory is a core part of every embedded system. It stores the program, configuration data, and temporary values while the system runs.

Embedded systems usually have limited memory, so understanding how memory is organized helps write efficient and reliable firmware.

---

## Types of memory in embedded systems

Most embedded devices use two main memory categories:

- **Non-volatile memory**: retains data when power is removed.
- **Volatile memory**: loses data when power is removed.

### Non-volatile memory

Non-volatile memory stores the program and configuration permanently.

Common examples:

- Flash memory: stores firmware code and constants
- EEPROM: stores small amounts of data that can change at runtime
- ROM: read-only memory used for fixed data or bootloaders

### Volatile memory

Volatile memory stores temporary data used while the system is running.

Common examples:

- SRAM: stores variables, stack, and heap
- DRAM: not common in small microcontrollers, but used in larger systems

---

## Program memory vs data memory

Embedded systems often separate program memory from data memory.

- Program memory holds the executable code.
- Data memory holds variables, stack frames, and temporary buffers.

On many microcontrollers, the CPU fetches instructions from flash and reads/writes variables in SRAM.

---

## Why memory organization matters

Embedded systems usually have tight memory budgets.

Good memory decisions help:

- reduce firmware size
- avoid stack overflows
- prevent memory corruption
- improve performance

---

## Common memory regions

A typical embedded memory map includes:

- **Flash**: where the application code is stored
- **SRAM**: where runtime data lives
- **Peripheral registers**: memory-mapped I/O for controlling hardware
- **Bootloader area**: optional region used for firmware updates

### Memory-mapped I/O

Many microcontrollers use memory addresses to access peripherals.

That means a register for a GPIO pin or a timer appears as a location in memory.

Software can read or write that address to control the hardware.

---

## Simple example

A small program may use memory like this:

- Flash stores the program instructions and constant lookup tables
- SRAM stores global variables, local variables, and the call stack
- peripheral registers store current hardware state

This separation helps the system run predictably and makes it easier to debug.

---

## Key idea

Memory basics are essential for embedded development. Knowing the difference between volatile and non-volatile memory, and understanding how program and data memory are used, helps build more efficient and stable firmware.