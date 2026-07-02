# Operators

## Contents
- Arithmetic Operators
- Increment / Decrement
- Assignment Operators
- Comparison Operators
- Logical Operators
- Bitwise Operators
- Conditional Operator
- sizeof Operator
- Comma Operator
- Pointer Operators
- Type Casting
- Operator Precedence

## Arithmetic Operators (+, -, *, /, %)

Arithmetic operators are used not only for calculations but also for:
- pointer arithmetic
- timer calculations
- baud-rate configuration
- memory address offsets
- embedded register manipulation

### Addition

Let's consider the example of addition.


int a = 10;
int b = 20;
int c = a + b;   // c = 10 + 20


But what exactly happens behind the scenes in an ARM Cortex-M CPU?
How is 30 stored in C and where does the addition happen? Does addition happen inside RAM since the values are stored there?

Before execution:

RAM

| Address | Value |
|--------:|:------|
| 1000 | 10 (a) |
| 1004 | 20 (b) |
| 1008 |  |

When the compiler generates instructions and reaches the line c = a + b, it doesn't add the values directly in RAM. Instead, the compiler generates assembler instructions something like this:


LDR R1, [1000]   ; load a into R1
LDR R2, [1004]   ; load b into R2
ADD R3, R1, R2   ; R3 = a + b
STR R3, [1008]   ; store result into c


- Values are loaded from RAM into CPU registers.
- Arithmetic is performed inside the ALU in the CPU using CPU registers.
- This is not done in RAM.
- Registers are the CPU’s fastest storage used to hold operands and intermediate results during instruction execution.
- RAM only stores values before and after computation.

So after execution:

RAM

| Address | Value |
|--------:|:------|
| 1000 | 10 (a) |
| 1004 | 20 (b) |
| 1008 | 30 (c) |

RAM → Registers → ALU → Registers → RAM

### Subtraction

Let's consider another example with subtraction.


int a = 20;
int b = 6;
int c = a - b;


Similar to the above addition example, the compiler generates assembler instructions:

asm
LDR R1, [1000]   ; load a into R1
LDR R2, [1004]   ; load b into R2
SUB R3, R1, R2   ; R3 = a - b  (two’s complement + addition)
STR R3, [1008]   ; store result into c


Most processors implement subtraction by using binary addition with the two's complement of the second operand. This allows the same adder hardware to perform both addition and subtraction efficiently.

a - b = a + (~b + 1)

Example:

20 - 6

Convert to binary:
- 20 = 00010100
- 6 = 00000110

The ALU does subtraction like this:
A - B = A + (two’s complement of B)

So we convert 6 → -6 in binary form.

Two’s complement of 6:
- Invert bits (1’s complement)
  - 6 = 00000110
  - ~6 = 11111001
- Add 1

text
11111001
+       1
---------
11111010


This is -6.

Now the ALU performs addition:

text
  00010100   (20)
+ 11111010   (-6)
--------------
1 00001110


Handle carry:
- Leftmost carry is ignored in 8-bit arithmetic.
- Result: 00001110
- 00001110 = 14 (20 - 6 = 14)

What the ALU actually did:
- Inside the CPU:
  - Took 20 in a register.
  - Took 6 in a register.
  - Converted 6 → two’s complement (-6).
  - Used the same adder circuit as +.
  - Produced result in a register.
  - Stored back to RAM.

ALU does NOT have a separate subtract circuit in basic form. It reuses the ADDER hardware using two’s complement.

### Multiplication

Consider:


int a = 20;
int b = 6;
int c = a * b;


Conceptual ARM assembly:


LDR R1, [1000]    ; load a
LDR R2, [1004]    ; load b
MUL R3, R1, R2    ; R3 = a × b
STR R3, [1008]    ; store result


What happens?
- 20 is loaded into R1.
- 6 is loaded into R2.
- The CPU executes the MUL instruction in the ALU.
- The result (120) is stored in R3.
- The result is written back to RAM.

After execution:

RAM

| Address | Value |
|--------:|:------|
| 1000 | 20 |
| 1004 | 6 |
| 1008 | 120 |

### Division

c
int a = 20;
int b = 6;
int c = a / b;


Conceptual assembly:

asm
LDR R1, [1000]
LDR R2, [1004]
SDIV R3, R1, R2      ; signed division
STR R3, [1008]


Result:
- 20 / 6 = 3
- Integer division discards the fractional part.

After execution:

RAM

| Address | Value |
|--------:|:------|
| 1000 | 20 |
| 1004 | 6 |
| 1008 | 3 |

- Division is generally slower than addition, subtraction, and multiplication.
- Some ARM Cortex-M processors have a hardware divide instruction (SDIV / UDIV).
- Some smaller Cortex-M processors do not. In that case, the compiler generates a software routine to perform the division.
- Division may be hardware-accelerated or implemented in software depending on the Cortex-M core.

### Modulus (%)

Consider:

c
int a = 20;
int b = 6;
int c = a % b;


The modulus operator returns the remainder after integer division.

20 ÷ 6

- Quotient = 3
- Remainder = 2

Therefore:

20 % 6 = 2

Conceptual assembly:

asm
LDR R1, [1000]
LDR R2, [1004]
...
STR R3, [1008]


(The compiler may use hardware division instructions if available, or call a software routine. There is no dedicated ARM instruction that computes only the remainder.)

After execution:

RAM

| Address | Value |
|--------:|:------|
| 1000 | 20 |
| 1004 | 6 |
| 1008 | 2 |

### Operator summary

| Operator | Purpose | Typical CPU support |
|---|---|---|
| + | Addition | ADD instruction |
| - | Subtraction | SUB instruction (implemented using binary subtraction logic, commonly based on two's complement arithmetic) |
| * | Multiplication | MUL instruction (hardware multiplier on many Cortex-M cores) |
| / | Integer Division | SDIV / UDIV if supported, otherwise software routine |
| % | Remainder | Derived from integer division; no dedicated remainder instruction on ARM Cortex-M |

## Increment (++) and Decrement (--)

These are mostly used in loops (for, while), array traversal, pointer movement, counters, timers, etc.

### Increment Operator (++)

Adds 1 to a variable.

Example:

c
int count = 5;
count++;


After execution: count = 6

Equivalent to:

c
count = count + 1;


CPU view (conceptual):
- count = 5
- Address = 1000

Conceptual assembly:

asm
LDR R1, [1000]     ; R1 = 5
ADD R1, R1, #1     ; R1 = 6
STR R1, [1000]     ; store back


Notice:
- There is no special ALU magic.
- ++ is simply: Read → Add 1 → Write back.

### Decrement Operator (--)

Subtracts 1.

Example:

c
count--;


Equivalent to:

c
count = count - 1;


Conceptual assembly:

asm
LDR R1, [1000]
SUB R1, R1, #1
STR R1, [1000]


### Preincrement and Postincrement

There are two forms:

c
int i = 3;
++i;
i++;


They both increment the variable. The difference is when the increment happens.

- ++i; increments first, then uses the new value. (i = 4)
- i++; uses the original value first, then increments it. (i = 5)

Similarly for decrement:
- --i decrements first and then uses the value.
- i-- uses the original value first and then decrements it.

### Assignment Operator
The assignment operator stores a value into a variable. The most common assignment operator is = 

###### Example:
```c
int a = 10; // Value 10 is assigned to the variable.
```
##### CPU View: 

Conceptually, the compiler may generate something like
```asm
MOV R1, #10
STR R1, [a]
```
Register R1 = 10
↓
Store to RAM
↓
a = 10

#### Compound Assignment Operators

Instead of writing: a = a + 5; 
C allows: a += 5;
Both mean exactly the same thing.

##### CPU View Conceptually:
```asm
LDR R1, [a]
ADD R1, R1, #5
STR R1, [a]
```

```text
RAM
↓
Register
↓
ALU
↓
Register
↓
```
RAM

Exactly the same flow as + operator.

#### Other Assignment Operators

| Operator | Equivalent |
|----------|------------|
| +=       | a = a + b  |
| -=       | a = a - b  |
| *=       | a = a * b  |
| /=       | a = a / b  |
| %=       | a = a % b  |


### Comparison (Relational) Operators:

These operators are used to compare two values. Unlike arithmetic operators, they do not produce a mathematical result.
Instead, they produce:
1 → True
0 → False

This result is then used by statements like if, while, and for.

#### Comparison Operators

| Operator | Meaning |
|----------|---------|
| ==       | Equal to |
| !=       | Not equal to |
| >        | Greater than |
| <        | Less than |
| >=       | Greater than or equal to |
| <=       | Less than or equal to |


###### Example 
int a = 10;
int b = 20;

int result = (a < b); // int result = 1

##### CPU view : conceptual 

LDR R1, [a]
LDR R2, [b]
CMP R1, R2
Notice something important:
There is no less_than instruction. It uses CMP.

###### What does CMP do?
Conceptually:

CMP R1, R2 means compare R1 with R2
Internally, the processor performs a subtraction only to determine the relationship between the two values. It does not store the subtraction result in a register. Instead, it updates special status flags inside the CPU.
Lets study after those flags (Zero, Negative, Carry, Overflow) when we cover branching and decision making. For now, just know:

CMP compares values and updates CPU status flags; it does not write a result back to a register.

###### Example 2
int a = 15;
int b = 15;

int result = (a == b);  // int result = 1

CPU:
LDR R1, [a]
LDR R2, [b]
CMP R1, R2

###### Example 3
int a = 25;
int b = 30;
int result = (a > b); // result = 0

##### Note:
if (a > b)
{
...
}
Notice there is no == 1.
That's because in C:
0 means false. Any non-zero value means true. So these are equivalent:
if (a > b)

and

if ((a > b) != 0)


#### Logical Operators: 
Used to combine multiple conditions or modify comparison results
They work with true (non-zero) and false (0) values.


&& (Logical AND) : true only if BOTH conditions are true.
|| (Logical OR) : 
! (Logical NOT)

##### Logical AND (&&):

The result is true only if BOTH conditions are true.

###### Example:

int temperature = 85;
int fan_enabled = 1;

if ((temperature > 80) && (fan_enabled == 1))  // 1 (both conditions true)
{
    // Turn on cooling
}

###### Truth table:

| Condition 1 | Condition 2 | Result |
|-------------|-------------|--------|
| 0           | 0           | 0      |
| 0           | 1           | 0      |
| 1           | 0           | 0      |
| 1           | 1           | 1      |


In logical And &&, when the 1st condition is not true( false), then the second condition is not evaluated (short-circuit evaluation).  

##### Example

Suppose you only want to start a motor if:
Start button is pressed
Emergency stop is NOT active

if ((start_button == 1) && (emergency_stop == 0))
{
   // Start motor
}

Both conditions must be satisfied.

##### CPU View (Conceptual)

Suppose:
if ((a > b) && (c == d))

Conceptually, the CPU performs:
LDR R1, [a]
LDR R2, [b]
CMP R1, R2
       ; First comparison

LDR R3, [c]
LDT R4, [d]
CMP R3, R4
       ; Second comparison
       
Then it combines the results according to the rules of &&.


###### Logical OR (||):
The result is true if at least one condition is true.

###### Example
int temperature = 90;
int emergency = 0;

if ((temperature > 80) || (emergency == 1))
{
   // Take action
}

Let's evaluate it.

Here, the first condition (90 > 80)  is true, the second condition (0 == 1) is false. The result is true since one of the conditions(atleast one condition) is true. 

###### Truth Table:

| Condition 1 | Condition 2 | Result |
|-------------|-------------|--------|
| 0           | 0           | 0      |
| 0           | 1           | 1      |
| 1           | 0           | 1      |
| 1           | 1           | 1      |


###### Example:
Suppose you want to stop a motor if:
Emergency button is pressed OR
Motor temperature is too high

if ((emergency_button == 1) || (motor_temp > 90))
{
   // Stop motor
}
The block executes, when one of the conditions is true.


##### Logical NOT (!):
It inverts a condition.

true (1) → becomes false (0)
false (0) → becomes true (1)

###### Example: 
int a = 5;
int result = !(a > 3);  // result = !(5 > 3)   which is !(1). ie 0


| Input | Output |
|-------|--------|
| 0     | 1      |
| non-zero | 0 |


###### Example:

if (!sensor_ok)      // means  if (sensor_ok == 0)
if (!ptr)   // means if (ptr == NULL)
if (!UART_READY)   // Meaning UART is NOT ready

###### Example 1:

int a = 0;
int result = !a;  // result = 1

#### Example 2

```c
int a = 5;
int result = !a;    // result = 0
```

#### Example 3

```c
int a = 3;
int b = 4;
int result = !(a < b);  // result = 0
```

## Bitwise Operators

Unlike logical operators (`&&`, `||`), bitwise operators work on individual bits of a number.

| Operator | Name | Purpose |
|---|---|---|
| `&` | Bitwise AND | Returns `1` only if both bits are `1` |
| `|` | Bitwise OR | Returns `1` if any bit is `1` |
| `^` | Bitwise XOR | Returns `1` if bits are different |
| `~` | Bitwise NOT | Inverts all bits |
| `<<` | Left Shift | Shifts bits left |
| `>>` | Right Shift | Shifts bits right |

### Bitwise AND (`&`)

Used to check a bit.

- `1 & 1 = 1`
- `1 & 0 = 0`
- `0 & 1 = 0`
- `0 & 0 = 0`

#### Example

```c
int a = 6;
int b = 3;
int result = a & b;  // result = 2
```

Step 1: Convert to binary (8-bit)
- `6 = 0000 0110`
- `3 = 0000 0011`

Step 2: Apply AND bit by bit

```text
00000110
&00000011
---------
00000010
```

Step 3: Convert back
- `00000010 = 2`
- `result = 2`

Why are bitwise operators important in embedded systems?
Bitwise operators are used to check bits inside hardware registers.

#### Example: Checking a pin state

```c
// Bitwise AND is used to CHECK a bit
if (GPIOA_IDR & (1 << 5))
```

Meaning:
- Check if bit `5` is `HIGH`

Assume register:
- `GPIOA_IDR = 00010000` (Input Data Register of Port A)

Mask:
- `(1 << 5) = 00100000` (Mask only the 5th bit)

Now:

```text
00010000
&00100000
---------
00000000
```

Result = `0` → pin is `LOW`

If bit was set:

```text
00100000
&00100000
---------
00100000
```

Result ≠ `0` → pin is `HIGH`

#### CPU view (simple)

```asm
AND R3, R1, R2
```

The ALU compares each bit, produces the result bit-by-bit, and stores it in a register.

### Bitwise OR (`|`)

Used to set a bit.

- `1 | 1 = 1`
- `1 | 0 = 1`
- `0 | 1 = 1`
- `0 | 0 = 0`

If any bit is `1`, the result becomes `1`.

#### Example: Turning ON an LED (STM32)

```c
GPIOA_ODR |= (1 << 5);
```

`ODR` is the Output Data Register. It controls output pins.

| Bit | Pin |
|---|---|
| 0 | PA0 |
| 1 | PA1 |
| 5 | PA5 (LED maybe) |

Assume current state:
- `GPIOA_ODR = 00000000`
- All pins are OFF.

Create mask:
- `(1 << 5) = 00100000`
- Meaning: only pin `5` is targeted.

Apply OR:

```text
00000000
|00100000
---------
00100000
```

Result:
- `GPIOA_ODR = 00100000`
- Pin `5` is now `HIGH` (LED ON)

Important concept:
We did not overwrite everything. We only changed one bit.

Why `|=` is used:

```c
GPIOA_ODR |= (1 << 5);
```

means:

```c
GPIOA_ODR = GPIOA_ODR | (1 << 5);
```

So: “Keep old values + set bit 5”.

#### Comparison with simple assignment

```c
GPIOA_ODR = (1 << 5);
```

This is wrong and dangerous in embedded systems because it wipes all other pins and only keeps pin `5`.

Hence:

```c
GPIOA_ODR |= (1 << 5);
```

is correct because it preserves other pins and only turns ON pin `5`.

#### Example

Assume:
- `REG = 00001000`

Apply:

```c
REG |= (1 << 1);
```

Question:
What is the final value of `REG`?

```text
0000 0010
```

### Bitwise NOT (`~`)

Used to clear a bit.

The `~` operator inverts every bit.
- Every `0` becomes `1`
- Every `1` becomes `0`

This is how we:
- turn OFF LEDs
- reset registers
- disable peripherals safely

#### Example

```c
GPIOA_ODR &= ~(1 << 5);
```

The `~` operator inverts every bit.
Every `0` becomes `1`, and every `1` becomes `0`.

Example:
- Original: `00000101`
- `~Original`: `11111010`

That is all `~` does.

Why is `~` useful in embedded C?
By itself, `~` is rarely used. The real power comes when it is combined with `AND (&)`.

The common pattern is:

```c
REG &= ~(1 << n);
```

This means:
Clear (make `0`) bit `n` without changing any other bits.

#### Example

Suppose:
- `REG = 00101110`

We want to clear bit `3`.

Step 1: Create the mask

```text
1 << 3
```

```text
00000001
<< 3
00001000
```

This mask has only bit `3` set.

Step 2: Apply `~`

```text
00001000
~
11110111
```

Notice:
- The bit we want to clear becomes `0`
- Everything else becomes `1`

Step 3: AND with the register

```text
REG
00101110

Mask
11110111

AND
00100110
```

Final result:
- `00100110`
- Only bit `3` changed from `1` to `0`
- Everything else stayed exactly the same.

Why does this work?
Remember the AND rule:
- `1 & x = x`
- `0 & x = 0`

The mask created by:

```c
~(1 << 3)
```

looks like:

```text
11110111
```

Every `1` says: leave this bit unchanged.
The single `0` says: force this bit to `0`.
That is why this technique is used everywhere in embedded programming.

#### Real STM32 example

Suppose `PA5` controls an LED.
To turn it OFF:

```c
GPIOA->ODR &= ~(1U << 5);
```

Read it in English:
Read the Output Data Register, clear bit `5`, and write the updated value back.

### The four most common bit operations

| Operation | Code | Meaning |
|---|---|---|
| Check bit | `REG & (1U << n)` | Is bit `n` set? |
| Set bit | `REG |= (1U << n)` | Make bit `n = 1` |
| Clear bit | `REG &= ~(1U << n)` | Make bit `n = 0` |
| Toggle bit | `REG ^= (1U << n)` | Flip bit `n` |

Note: These patterns should be used when you want to modify only the specified bit without affecting other bits in the register.

These three patterns appear throughout STM32 reference manuals, HAL code, LL drivers, and bare-metal firmware.

#### Mini practice

Assume:
- `REG = 00001111`

Now execute:

```c
REG &= ~(1 << 2);
```

Question:
What is the final value of `REG` in binary?

Take it step by step:

```c
REG &= ~(1 << 2);
REG = REG & (~(1 << 2));
```

Compute `1 << 2`:

```text
0000 0100
```

`~(1 << 2) = 1111 1011`

```text
REG = REG & (~(1 << 2));
REG = 00001111
0000 1111   &
1111  1011
-----------
REG = 0000 1011
```

### Bitwise XOR (`^`)

The result is `1` only when the two bits are different.
- `0 ^ 0 = 0`
- `0 ^ 1 = 1`
- `1 ^ 0 = 1`
- `1 ^ 1 = 0`

#### Example

```c
int a = 12;
int b = 10;

int result = a ^ b;
```

Step 1: Convert to binary
- `12 = 00001100`
- `10 = 00001010`

Step 2: XOR

```text
00001100
^00001010
---------
00000110
```

Step 3: Convert back
- `00000110 = 6`
- Final answer: `result = 6`

#### Embedded uses of XOR

Unlike `&` and `|`, XOR is not used as frequently, but it is still useful.

1. Toggle a bit

Suppose an LED is connected to bit `5`.

```c
GPIOA->ODR ^= (1U << 5);
```

If bit `5` is `0`:

```text
00000000
^00100000
---------
00100000
```

LED turns ON.

If bit `5` is already `1`:

```text
00100000
^00100000
---------
00000000
```

LED turns OFF.

So every execution toggles the LED.

### Compare the four bitwise operators

| Operator | Embedded purpose |
|---|---|
| `&` | Check a bit |
| `|` | Set a bit |
| `~` | Invert bits (commonly used with `&` to clear a bit) |
| `^` | Toggle a bit |

#### Question 1

```c
int a = 9;
int b = 5;

int result = a ^ b;
```

Hint:
- `9 = 00001001`
- `5 = 00000101`

What is result?

```text
00001001
^00000101
---------
00001100
```

### Right Shift (`>>`)

#### Example

```c
int x = 40;
int y = x >> 2;
```

Step 1: Convert to binary
- `40 = 00101000`

Step 2: Shift right by `2`

```text
00101000
>> 1
00010100

>> 2
00001010
```

Result:
- `00001010 = 10`
- So: `y = 10;`

What does right shift do?
For unsigned positive integers, each right shift by `1` is equivalent to dividing by `2` and discarding any remainder.

#### Examples

| Expression | Result |
|---|---:|
| `40 >> 1` | `20` |
| `40 >> 2` | `10` |
| `40 >> 3` | `5` |

#### Embedded uses

Right shift is commonly used to:
- Extract fields from hardware registers
- Scale values efficiently
- Decode packed data

For example:

```c
status = (REG >> 4) & 0x0F;
```

#### Example

```c
int x = 48;
int y = x >> 3;
```

### Bitwise operator summary

- `&` → Check bits
- `|` → Set bits
- `~` → Invert bits (used with `&` to clear bits)
- `^` → Toggle bits
- `<<` → Left shift
- `>>` → Right shift

```c
REG & (1U << n);   // Check bit
REG |= (1U << n);  // Set bit
REG &= ~(1U << n); // Clear bit
```

### Conditional (Ternary) Operator (`?:`)

The conditional operator is a short form of `if...else`.

**Syntax**

```
condition ? expression_if_true : expression_if_false;
```

Read it as: if the condition is true, use the first expression; otherwise, use the second expression.

#### Example 1

```c
int a = 10;
int b = 20;

int max = (a > b) ? a : b;
```

Let's evaluate it.

Step 1 — Condition:

```
a > b  // 10 > 20
```

Result: false (`0`).

Step 2 — Since the condition is false, the expression after `:` is chosen:

```
max = b; // max = 20
```

Equivalent `if...else`:

```c
int max;
if (a > b) {
   max = a;
} else {
   max = b;
}
```

The compiler will generate appropriate instructions for either form. The ternary operator is simply a more compact way to express the same decision.

#### Embedded example

```c
brightness = (battery_low) ? 20 : 100;
```

Meaning:
- If `battery_low` is true → `brightness = 20`%
- Otherwise → `brightness = 100`%

Good for simple value selection. Avoid it for long or complex logic where an `if...else` is easier to read.

### `sizeof` Operator

`sizeof` returns the number of bytes occupied by a type or an object.

**Syntax**:

```
sizeof(type)
```
or
```
sizeof(variable)
```

#### Example 1

```c
int a = 10;
printf("%zu\n", sizeof(a));
```

On an STM32 (32-bit ARM Cortex-M) the output is `4` because `int = 4` bytes.

#### Example 2

```c
char c = 'A';
sizeof(c); // 1
```

#### Example 3

```c
float f;
sizeof(f); // 4
```

`sizeof` is determined by the compiler at compile time. For example, `sizeof(int)` may be replaced by `4` on this target before the program runs.

CPU view (conceptual):

```asm
MOV R1, #4
STR R1, [size]
```

#### Embedded uses

1. Array size

```c
int arr[10];
// instead of writing 40, write sizeof(arr)
```

`sizeof(arr)` becomes `80` automatically if the array size changes to `int arr[20];`.

2. UART transmission

```c
char msg[] = "Hello";
send(msg, sizeof(msg));
```

3. Structures

```c
struct SensorData data;
sizeof(data);
```

##### Important difference

Don't confuse `sizeof(arr)` with `sizeof(arr[0])`.

```c
int arr[10];
sizeof(arr);      // 40 bytes (whole array)
sizeof(arr[0]);   // 4 bytes (one element)
```

To get the number of elements:

```c
sizeof(arr) / sizeof(arr[0]);
```

#### Example

```c
int adc_samples[100];
for (int i = 0; i < sizeof(adc_samples) / sizeof(adc_samples[0]); i++) {
   ...
}
```

### Comma Operator (`,`) 

Don't confuse the comma as a separator with the comma operator — they are different.

Comma as a separator (most common):

```c
int a = 10, b = 20, c = 30;
func(a, b, c);
```

The comma operator evaluates expressions from left to right and the value of the last expression becomes the result.

```c
int x;
x = (5, 10); // x = 10
```

Example:

```c
int a = 5;
int b = 10;
int x = (a++, b++);
// After: a = 6, b = 11, x = 10
```

The comma operator is rarely used in embedded systems except in constructs like:

```c
for (i = 0, j = 10; i < j; i++, j--) {
   ...
}
```

##### Questions

```c
int x;
x = (3, 8); // x = 8

int a = 2;
int b = 5;
int x = (a++, ++b); // a = 3, b = 6, x = 6
```