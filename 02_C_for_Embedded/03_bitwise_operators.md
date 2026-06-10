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




### 1. AND operator (`&`)

The AND operator compares two bits.

Result is 1 only if both bits are 1.

```
0 & 0 = 0
0 & 1 = 0
1 & 0 = 0
1 & 1 = 1
```

Example in decimal:

```c
5 & 3 = 1
// binary: 0101 & 0011 = 0001
```

#### Use in embedded: extracting a bit or checking a bit

```c
#define GPIO_PIN_5 (1U << 5)

uint32_t port_value = 0x00000020; // bit 5 is set
uint32_t is_bit_5_set = port_value & GPIO_PIN_5;

if (is_bit_5_set) {
    // bit 5 is 1
}
```

The result is non-zero only if bit 5 is set.

#### Use: clearing bits

```c
uint32_t control_reg = 0xFF;
uint32_t mask = 0xF0;
uint32_t result = control_reg & mask;
// keeps upper 4 bits, clears lower 4 bits
```

---

