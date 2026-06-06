# Architecture Overview

Understanding the architecture of a microcontroller helps connect the hardware blocks you use in code with how they are implemented on silicon.


---

## CPU core

The CPU core executes instructions and performs arithmetic and logical operations. Common ARM cores used in embedded systems include Cortex-M0, M3, M4, and M7.

Key points:

- Cores differ in performance, instruction set features, and power consumption.
- Some cores include DSP or floating-point units for signal processing.

---

## Memory system and bus matrix

Microcontrollers expose several memory types (flash, SRAM, peripheral registers) connected via internal buses.

- A bus matrix or interconnect routes requests between masters (CPU, DMA) and slaves (memory, peripherals).
- Bus contention and priority affect latency for time-sensitive operations.

---

## Von Neumann vs Harvard

- **Von Neumann**: program and data share the same memory and bus.
- **Harvard**: separate buses for instructions and data (common in many MCUs for better throughput).

Many modern MCUs use a modified Harvard architecture: flash for instructions, SRAM for data, with caches or prefetch buffers to improve performance.

---

## Peripherals and memory-mapped I/O

Peripherals (GPIO, timers, UART, SPI, I2C, ADC) are accessed through memory-mapped registers.

- Each peripheral exposes registers at fixed addresses.
- Reading/writing these addresses changes hardware behavior.

This makes peripheral access in C as simple as reading and writing pointers to specific addresses.

---

## Interrupts and exception model

Interrupts allow hardware events to pre-empt the CPU.

- NVIC (Nested Vectored Interrupt Controller) commonly used in ARM Cortex-M.
- Priorities, preemption, and tail-chaining influence real-time responsiveness.

Good interrupt design reduces latency and avoids long blocking critical sections.

---

## Clock, reset, and power domains

Clocks drive CPU and peripheral timing. Reset lines and power domains control startup and energy usage.

- Configuring clocks affects peripheral speeds and power consumption.
- Entering low-power modes requires careful state management of peripherals and memory.

---

## Simple example: toggling a GPIO

When software toggles a GPIO pin:

1. CPU writes to a memory-mapped GPIO register (bus transaction).
2. The interconnect routes the write to the GPIO peripheral.
3. The GPIO peripheral updates hardware registers and drives the output pin.

Understanding the architectural steps helps debug timing and bus-related problems.

---

## Key idea

Microcontroller architecture describes how CPU, memory, buses, and peripherals are connected. Knowing these blocks and their interactions helps write efficient, predictable firmware and troubleshoot hardware/software interactions.
