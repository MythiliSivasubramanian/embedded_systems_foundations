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