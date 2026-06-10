# Bitwise Operators in Embedded C

Bitwise operators are tools for working with individual bits inside a number. In embedded systems, they are essential because firmware often needs to:

- set or clear specific bits in hardware registers
- check if certain bits are set or cleared
- manipulate bit patterns to communicate with hardware

This file explains what bitwise operators are, why they are used, and how to use them with real embedded examples.

---

## What are bitwise operators?

Bitwise operators work on the binary representation of numbers.

Every integer is stored as a sequence of bits. For example:

```
decimal 5 = binary 0000 0101
decimal 6 = binary 0000 0110
```

Bitwise operators let you read, set, clear, or flip individual bits.

---

## Why bitwise operators matter in embedded systems

Hardware registers are the interface between software and hardware.

Each register is a fixed-size number (usually 8, 16, or 32 bits).

Each bit often controls something specific:

- bit 0 might enable a peripheral
- bit 1 might set an output high or low
- bit 2 might indicate a status flag
- bits 3-5 might store a mode setting

Bitwise operators let firmware read and modify these bits safely and efficiently.

---

## The six main bitwise operators
    
###  1. AND operator (`&`)
### 2. OR operator (`|`)
### 3. XOR operator (`^`)
### 4. NOT operator (`~`)
### 5. Left shift operator (`<<`)
### 6. Right shift operator (`>>`)

