# Memory Layout of a C Program

## Table of Contents
1. [Why Do We Need Memory Sections?](#1-why-do-we-need-memory-sections)
   - [Q1. How would the CPU know where the program instructions begin?](#q1-how-would-the-cpu-know-where-the-program-instructions-begin)
   - [Q2. How would it distinguish program instructions from variables?](#q2-how-would-it-distinguish-program-instructions-from-variables)
   - [Q3. How would it know which memory should be read-only and which writable?](#q3-how-would-it-know-which-memory-should-be-read-only-and-which-writable)
   - [Q4. What problems occur if everything is mixed together?](#q4-what-problems-occur-if-everything-is-mixed-together)
   - [The Real Reasons Memory is Divided into Sections](#the-real-reasons-memory-is-divided-into-sections)
2. [Variable Storage and Memory Organization](#2-variable-storage-and-memory-organization)
   - [Goal](#goal)
3. [Overview of Program Memory Layout](#3-overview-of-program-memory-layout)
4. [Example Program](#4-example-program)
5. [From Source Code to Running Program](#5-from-source-code-to-running-program)
   - [Compiler](#compiler)
   - [Linker](#linker)
   - [Startup Code (Reset_Handler)](#startup-code-reset_handler)
6. [Deep Dive into Memory Sections](#6-deep-dive-into-memory-sections)
   - [Memory Sections in Flash](#memory-sections-in-flash)
   - [Memory Sections in RAM](#memory-sections-in-ram)
7. [What is a Stack Frame and How Does it Work?](#7-what-is-a-stack-frame-and-how-does-it-work)
   - [What is a Stack Frame?](#what-is-a-stack-frame)
   - [How the Stack Frame Works During Execution](#how-the-stack-frame-works-during-execution)
8. [Important Realization](#8-important-realization)

---

## 1. Why Do We Need Memory Sections?

Imagine if every variable, function, and string were stored randomly. 
Why does the compiler divide a program into sections?

Consider this simple C program:

```c
#include <stdio.h>

int global = 10;

int main(void)
{
    int local = 20;

    printf("Hello\n");

    return 0;
}

```

This program contains several different components:

* A function (`main`)
* Another function (`printf`)
* A global variable (`global`)
* A local variable (`local`)
* A string literal (`"Hello\n"`)

If the compiler simply put everything into memory randomly, something like this:

| Address | Content |
| --- | --- |
| `0x1000` | `global` |
| `0x1004` | machine instruction |
| `0x1008` | `"Hello"` |
| `0x100E` | local variable |
| `0x1012` | machine instruction |
| `0x1016` | another string |

Would this be a good design?

If so, consider these questions:

1. How would the CPU know where the program instructions begin?
2. How would it distinguish program instructions from variables?
3. How would it know which memory should be read-only and which should be writable?
4. If everything were mixed together, what problems could occur?

### Q1. How would the CPU know where the program instructions begin?

The CPU starts execution from a predefined address (after reset, STM32 uses the vector table and Reset Handler). The compiler and linker place the program instructions in a known section called `.text`. So the CPU knows exactly where executable instructions are stored.

### Q2. How would it distinguish program instructions from variables?

Imagine this memory layout:

```text
0x08000000    main()
0x08000020    global variable
0x08000024    printf()
0x08000050    local variable

```

Now suppose the CPU reaches `0x08000020`. Should it execute it as an instruction, or treat it as data?

It cannot magically know. That's why we separate `.text` from `.data`. Instructions live in one place, and variables live somewhere else. This separation makes execution predictable.

### Q3. How would it know which memory should be read-only and which writable?

The memory map and hardware decide.

For example:

* **Flash** is physically designed to be executable and generally not writable during normal program execution.
* **RAM** is designed for read/write access.

The linker places `.text` and `.rodata` into Flash, and `.data`, `.bss`, the stack, and the heap into RAM. Later we'll see how the linker script specifies this.

### Q4. What problems occur if everything is mixed together?

Imagine this layout:

```text
0x08000000    Instruction
0x08000004    Instruction
0x08000008    Variable
0x0800000C    Instruction

```

Suppose the program accidentally writes `variable = 100;`, but due to a bug, it writes to `0x08000004` instead.

What happened? We just changed the program itself. Imagine changing `ADD R0, R1` into `JUMP` somewhere—our program is now corrupted. Separating code and data prevents this.

---

### The Real Reasons Memory is Divided into Sections

The compiler divides the program into sections because each type of data has different requirements:

| Program Component | Why Separate It? |
| --- | --- |
| **Instructions** | CPU executes them; should not change accidentally. |
| **Constants** | Never change; can be read-only. |
| **Global variables** | Need permanent storage and can change. |
| **Local variables** | Exist only while a function runs. |
| **Dynamic memory** | Created and destroyed during runtime. |

---

## 2. Variable Storage and Memory Organization

### Goal

Today I wanted to deeply understand how and where different types of variables are stored in memory.

Below are the questions which I am trying to find out:

1. Who allocates the memory and where exactly is it stored?
2. What are the different memory sections? What is stored in Flash and what is stored in RAM? Why?
3. Who initializes the variable? What happens to uninitialized variables?
4. When does the initialization happen and how?
5. Does the startup code play any role?
6. Is the variable created before `main()` or only when a function is called?
7. What is the scope and lifetime of each variable?
8. What happens behind the scenes when the value of a variable changes later?

---

## 3. Overview of Program Memory Layout

We'll first see the complete picture without worrying about details:

```text
High Address
+----------------------+
| Stack                |  <-- Local variables, function calls, return addresses (grows down)
|        ↓             |
+----------------------+
|                      |
| Heap                 |  <-- Dynamic memory allocation (malloc/free) (grows up)
|        ↑             |
+----------------------+
| .bss                 |  <-- Uninitialized global/static variables (filled with 0 at boot)
+----------------------+
| .data                |  <-- Initialized global/static variables (copied from Flash to RAM)
+----------------------+
| .rodata              |  <-- Read-only constants and string literals (stored in Flash)
+----------------------+
| .text                |  <-- Program instructions (machine code in Flash)
+----------------------+
Low Address

```

---

## 4. Example Program

Rather than studying every variable separately, below is a C program that contains almost every storage class:

```c
#include <stdio.h>
#include <stdlib.h>

int g1;                     // (1) Uninitialized global variable (.bss)
int g2 = 100;               // (2) Initialized global variable (.data)

static int g3;              // (3) Uninitialized static global variable (.bss)
static int g4 = 200;        // (4) Initialized static global variable (.data)

const int g5 = 300;         // (5) Global constant (.rodata in Flash)

extern int ext_var;         // (6) Declaration only (Linker resolves symbol)

void add(int a, int b)      // Machine instructions for add() live in .text in Flash
{
    int sum;                // (7) Uninitialized local variable (Stack frame)

    int temp = 10;          // (8) Initialized local variable (Stack frame)

    static int count;       // (9) Uninitialized static local variable (.bss)

    static int total = 50;  // (10) Initialized static local variable (.data)

    sum = a + b + temp;

    count++;
    total += sum;

    printf("%d\n", total);
}

int main(void)              // Machine instructions for main() live in .text in Flash
{
    int x;                  // (11) Uninitialized local variable (Stack frame)

    int y = 20;             // (12) Initialized local variable (Stack frame)

    int *ptr = malloc(sizeof(int) * 5); // (13) Heap allocation

    add(y, 5);

    free(ptr);

    return 0;
}

```

Every variable in this program behaves differently, so I used it to understand exactly what happens behind the scenes.

---

## 5. From Source Code to Running Program

```text
main.c  -->  Compiler  -->  Object File (.o)  -->  Linker  -->  firmware.elf / firmware.bin  -->  Flash the MCU 

--> MCU Reset  -->  Hardware loads initial Stack Pointer from Vector Table (Address 0x0000_0000) ──--->

---> Reset_Handler (Startup Code)  --------------------->  main()
        ├── Copy .data from Flash 
        ├── Clear .bss by filling RAM with 0
        └── Jump to main()  

```
One of the biggest realizations during this study was that the **compiler**, **linker**, and **startup code** all have completely different responsibilities:

### Compiler

The compiler translates C code into machine instructions (Assembly code).

For functions like `add()` and `main()`, the compiler produces machine code instructions (like `ADD`, `MOV`, `PUSH`, `POP`, `BL`). It also generates instructions to handle stack frames whenever a function executes.

The compiler **does not** copy variables from Flash to RAM.

### Linker

The linker collects all object files (`.o`) and combines them into one final binary file. It places code and variables into their proper memory sections:

* `.text`
* `.rodata`
* `.data`
* `.bss`

The linker decides where each variable and function belongs in memory using the **Linker Script**.

### Startup Code (Reset_Handler)

After the MCU resets, the Cortex-M hardware automatically loads the initial Stack Pointer (SP) from address `0x0000_0000` (top of vector table). Then execution begins in the startup code (`Reset_Handler`) before `main()`.

Its major responsibilities are:

1. Copy the initial values of `.data` from Flash to RAM.
2. Clear the `.bss` section in RAM by writing zeros.
3. Jump to `main()`.

The startup code **does not** initialize local variables or create stack frames. Those are handled later by compiler-generated machine code instructions whenever a function is called during runtime.

---

## 6. Deep Dive into Memory Sections

### Memory Sections in Flash

#### `.text`

* Contains executable program instructions (Machine Code).
* Stored strictly in **Flash**.
* When we write a function like `void add(int a, int b)`, the compiler translates the C logic into raw ARM machine code instructions (e.g., `PUSH {r4, lr}`, `ADD r0, r0, r1`). These binary instructions live permanently in `.text` inside Flash memory.
* The CPU fetches these instructions directly from Flash over the instruction bus to execute them.

#### `.rodata`

* Contains read-only data such as constants and string literals.
* Stored in **Flash** because the program never needs to modify these values during execution.

*Example:*

```c
const int g5 = 300;

```

---

### Memory Sections in RAM

#### `.data`

* Contains initialized variables with static storage duration (Global and Static variables).
* **The dual address puzzle (LMA vs VMA):** The initial values (e.g., `100`, `200`, `50`) are stored in Flash (Load Memory Address / LMA). But during execution, these variables must be readable and writable, so their runtime addresses are mapped to RAM (Virtual Memory Address / VMA).
* During boot, the `Reset_Handler` uses linker symbols (`_sidata`, `_sdata`, `_edata`) to copy these initial byte patterns from Flash into RAM before `main()` starts.

*Examples:*

```c
int g2 = 100;
static int g4 = 200;
static int total = 50;

```

#### `.bss`

* Contains uninitialized variables with static storage duration.
* Since these variables have no explicit initial value, nothing needs to be saved in Flash. This saves valuable Flash space.
* Instead, during startup, the `Reset_Handler` uses linker symbols (`_sbss`, `_ebss`) to clear this section in RAM by writing zeros.

*Examples:*

```c
int g1;
static int g3;
static int count;

```

#### Heap

* Used for dynamic memory allocation requested by the programmer at runtime using `malloc()`, `calloc()`, or `realloc()`.
* Lives in RAM and **grows upward** toward higher memory addresses (toward the Stack).
* Unlike `.data` or `.bss`, the startup code doesn't set up dynamic variables. The C runtime library (heap manager) tracks free RAM blocks, and you must explicitly release this memory using `free()`.

*Example:*

```c
int *ptr = malloc(sizeof(int) * 5); // Allocated on Heap at runtime
free(ptr);                          // Freed back to Heap

```

---

## 7. What is a Stack Frame and How Does it Work?

The Stack is completely different from `.data`, `.bss`, or the Heap.

### What is a Stack Frame?

A **Stack Frame** (or Activation Record) is a contiguous block of memory allocated on the Stack specifically for **one active function call**.

When a function is called, a new frame is pushed onto the stack. When the function returns, its stack frame is popped off, and that memory becomes available for future function calls.

A stack frame stores:

1. **Function Parameters** passed into the call (`a` and `b`)
2. **Local Variables** (`sum`, `temp`, `x`, `y`)
3. **Return Address** (where the CPU should return after the function finishes)
4. **Saved Register States** (CPU registers preserved across calls)

### How the Stack Frame Works During Execution

Let's walk through what happens when `main()` calls `add(y, 5)` step by step:

```text
Stack Memory (Grows Downward from High RAM Address to Low RAM Address)

High Address
+-----------------------------------+
|  main() Stack Frame               |
|  - Local variable: x              |
|  - Local variable: y = 20         |
|  - Pointer: ptr                   |
+-----------------------------------+  <-- Frame Pointer / Stack Base
|  add() Stack Frame (Created on call)|
|  - Return address to main         |
|  - Argument: a = 20, b = 5        |
|  - Local variable: temp = 10      |
|  - Local variable: sum            |
+-----------------------------------+  <-- Stack Pointer (SP) points here
|                                   |
↓ Grows Downward                    ↓
Low Address

```

#### Step 1: `main()` is executing

* Space is reserved on the Stack for `main()`'s local variables (`x`, `y`, `ptr`).
* The Stack Pointer (SP) moves down to allocate space for these variables.

#### Step 2: `main()` calls `add(y, 5)`

* The compiler generates a call instruction (like `BL add` in ARM).
* The hardware/compiler pushes the return address (the next instruction in `main()`) onto the stack.
* Parameters `y` (20) and `5` are passed via registers or pushed onto the stack.

#### Step 3: `add()` creates its Stack Frame

* The compiler-generated function prologue in `add()` adjusts the Stack Pointer (SP) downward further to reserve bytes for `sum` and `temp`.
* The instruction `temp = 10;` writes `10` directly into this allocated stack space at runtime.

#### Step 4: `add()` finishes and returns

* The compiler-generated function epilogue in `add()` restores the Stack Pointer back to where `main()` left off.
* The CPU pops the saved return address and jumps back to `main()`.
* **Important:** The memory for `sum` and `temp` is not erased byte-by-byte—the Stack Pointer simply moves back up! The frame is destroyed instantly because the SP no longer points to it.

---

## 8. Important Realization

One of the biggest takeaways from this study was understanding that different parts of the system have different responsibilities:

* The **compiler** generates machine code instructions in `.text` and controls Stack frames at runtime.
* The **linker** decides where code and static variables belong in memory (Flash vs RAM mapping).
* The **startup code** prepares `.data` and `.bss` in RAM before `main()` starts running.

Understanding this made it much easier to reason about where each variable is stored and why.
