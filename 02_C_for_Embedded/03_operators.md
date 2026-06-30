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

###### Example 2:

int a = 5;
int result = !a;    // result = 0

###### Example 3:
int a = 3;
int b = 4;
int result = !(a < b);  // result = 0