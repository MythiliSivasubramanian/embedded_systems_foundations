# Design Constraints and Tradeoffs

## Introduction

Embedded systems are built for a specific purpose, and that purpose drives design choices.
A good design balances performance, power, cost, size, and reliability.
This note explains the common constraints in embedded systems and how they affect decisions.

---

## Limited resources

Embedded devices usually have far fewer resources than general-purpose computers.
This is a deliberate tradeoff: smaller systems are cheaper, use less power, and fit inside the final product.

Common resource limitations include:

- small flash memory for firmware
- limited SRAM for variables and stack
- low CPU performance compared to desktop systems
- few peripheral pins and interfaces

When resources are limited, the firmware must be efficient and predictable.

---

## Power consumption

Many embedded systems run on batteries or energy-harvesting sources.
Power consumption becomes a primary constraint in such designs.

A few important strategies are:

- choose low-power microcontrollers
- use sleep modes when the system is idle
- minimize unnecessary peripheral activity
- reduce CPU clock speed when full performance is not needed

Power optimization often means trading raw performance for longer battery life.

---

## Real-time behavior

Some embedded systems must respond within a fixed time window.
This is called real-time behavior.

Hard real-time systems require guaranteed response times.
Soft real-time systems can tolerate occasional delays.

Examples of real-time requirements:

- a brake control system must respond immediately
- a temperature controller can tolerate small delays

Real-time design often uses:

- interrupt-driven processing
- prioritized tasks
- simple, deterministic loops

The tradeoff is usually increased design complexity in exchange for reliable timing.

---

## Cost and component selection

The final product cost is an important constraint in embedded design.
Choosing the right microcontroller and peripherals affects the bill of materials directly.

Cost-driven decisions include:

- selecting a lower-end MCU with enough performance
- using integrated peripherals instead of external chips
- minimizing PCB size and component count

The tradeoff is between a cheaper design and the flexibility or performance of the system.

---

## Reliability and robustness

Embedded systems often operate in harsh environments.
They must handle temperature changes, electrical noise, and occasional faults.

Design choices for reliability include:

- using protective components like TVS diodes and filters
- validating inputs before using them
- recovering cleanly from unexpected states
- avoiding complex runtime dependencies

A robust system may cost more or require more careful implementation.

---

## Development and maintainability

Embedded firmware should also be maintainable.
Clear architecture, modular code, and good documentation make the system easier to update.

This is especially important when the product will be used for years.

Good practices include:

- separating hardware abstraction from application logic
- using consistent naming and structure
- keeping critical code simple and well-tested

The tradeoff is sometimes writing a bit more code upfront to reduce future risk.

---

## Key idea

Embedded systems are defined by constraints.
Good designs do not try to maximize every metric at once.
Instead, they balance performance, power, cost, and reliability for the specific application.
