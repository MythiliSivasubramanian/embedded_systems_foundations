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

