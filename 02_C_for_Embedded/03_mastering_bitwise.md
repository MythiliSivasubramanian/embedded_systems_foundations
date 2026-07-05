# Mastering Bitwise

## Left Shift (`<<`) and Right Shift (`>>`)

### What is a Bit Shift?
A shift literally moves bits left or right.

Example:
```text
0010
Shift left by 1: 0100
Shift right by 1: 0001
```

Nothing more than moving bits.

## Left Shift (`<<`)

Example: `5 << 1`

Convert to binary.

`5 = 00000101`

Move every bit one position left.
```text
00000101
<< 1
---------
00001010
```

Convert back to decimal.

`10`

So `5 << 1 = 10`

Another example

`3 << 2 = ?`

`3 = 00000011`
```text
00000011
<< 2
---------
00001100
```

Decimal:

`12`

So `3 << 2 = 12`

### Shortcut
For positive integers that don't overflow:

`x << n` means `x × 2ⁿ`

Examples:

| Expression | Result |
|------------|--------|
| `7 << 1` | `7 × 2 = 14` |
| `7 << 2` | `7 × 4 = 28` |
| `7 << 3` | `7 × 8 = 56` |

This shortcut is valid only when the shifted value still fits in the type.

## Right Shift (`>>`)

Example: `20 >> 1`

Binary:

`20 = 00010100`
```text
00010100
>> 1
---------
00001010
```

Decimal:

`10`

Another example

`20 >> 2`
```text
00010100
>> 2
---------
00000101
```

Decimal : `5`

### Shortcut
For unsigned values and positive integers:

`x >> n` means `x ÷ 2ⁿ`

Examples:

| Expression | Result |
|------------|--------|
| `32 >> 1` | `16` |
| `32 >> 2` | `8` |
| `32 >> 3` | `4` |

### Important Rule
When we shift left:
```text
00010101
↓
00101010
```

The leftmost bit that falls off is lost forever. A `0` enters from the right.

When we shift right on an unsigned value:
```text
00101010
↓
00010101
```

The rightmost bit is discarded. A `0` enters from the left.

This expression: `1U << 5` is everywhere in embedded code.

Let's calculate it.

Start with: `00000001`

Shift left 5 times.
```text
00000001
↓
00000010
↓
00000100
↓
00001000
↓
00010000
↓
00100000
```

Decimal:

`32`

So: `1U << 5` creates a value where only bit 5 is set.

This is why it's used to create bit masks.

We'll use this constantly in STM32.

## Bit Masks

Everything we learned about shifting now has a purpose.

For example: `1U << 5` creates:

```text
00100000
```

That pattern is called a mask.

Using masks, we will learn to:

1. Set a bit
2. Clear a bit
3. Toggle a bit
4. Read a bit

And that's exactly how we control GPIO pins, timers, UART, SPI, I2C, RCC, interrupts—almost every peripheral on an STM32.

### What is a Mask?

A mask is simply a binary value used to operate on specific bit(s).

Example: `00100000`

This mask has only bit 5 set.

How do we create it? `1U << 5`

Let's prove it.

```text
00000001

<<5

00100000
```

So:

`1U << 5` creates a mask where only bit 5 is `1`.

#### Why do we need masks?

Suppose we have an 8-bit register.

```text
REG

Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 Bit0

0    1    0    1    1    0    0    1
```

Suppose we want to change only Bit 5.

We must leave every other bit unchanged.

That's what masks are for.

### Setting a Bit

Suppose: `REG 00001001`

We want to set Bit 5.

Mask: `1U << 5`

Binary: `00100000`

Now OR them.

```text
00001001

00100000

OR
----------------

00101001
```

Bit 5 becomes `1`.

Everything else remains unchanged.

Therefore: `REG |= (1U << 5);`

means `REG = REG | (1U << 5);`

Rule: OR (`|`) is used to SET bits.

