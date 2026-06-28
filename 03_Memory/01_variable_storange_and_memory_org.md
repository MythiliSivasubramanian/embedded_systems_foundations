# Variable Storage and Memory Organization

## Goal

Today I wanted to deeply understand how and where different types of variables are stored in memory.

Below are the questions which I am trying to find out:

1. Who allocates the memory and where exactly is it stored?
2. What are the different memory sections? What is stored in Flash and what is stored in RAM? Why?
3. Who initializes the variable? What happens to uninitialized variables?
4. When does the initialization happen and how?
5. Does the startup code play any role?
6. Is the variable created before main() or only when a function is called?
7. What is the scope and lifetime of each variable?
8. What happens behind the scenes when the value of a variable changes later?

Rather than studying every variable separately, below is a C program that contains almost every storage class.

---

# Example Program

c
#include <stdio.h>

int g1;                     // (1) Uninitialized global variable
int g2 = 100;               // (2) Initialized global variable

static int g3;              // (3) Uninitialized static global variable
static int g4 = 200;        // (4) Initialized static global variable

const int g5 = 300;         // (5) Global constant

extern int ext_var;         // (6) Declaration only

void add(int a, int b)
{
    int sum;                // (7) Uninitialized local variable

    int temp = 10;          // (8) Initialized local variable

    static int count;       // (9) Uninitialized static local variable

    static int total = 50;  // (10) Initialized static local variable

    sum = a + b + temp;

    count++;
    total += sum;

    printf("%d\n", total);
}

int main(void)
{
    int x;                  // (11) Uninitialized local variable

    int y = 20;             // (12) Initialized local variable

    add(y, 5);

    return 0;
}


Every variable in this program behaves differently, so I used it to understand exactly what happens behind the scenes.

---

# From Source Code to Running Program


main.c
   │
   ▼
Compiler
   │
   ▼
Object File (.o)
   │
   ▼
Linker
   │
   ▼
firmware.elf / firmware.bin
   │
   │ Flash the MCU
   ▼

MCU Reset
   │
   ▼
Reset_Handler (Startup Code)
   │
   ├── Initialize Stack Pointer
   ├── Copy .data from Flash → RAM
   ├── Clear .bss (fill with 0)
   └── Jump to main()

main()


One of the biggest realizations during this study was that the **compiler**, **linker**, and **startup code** all have completely different responsibilities.

---

# Compiler

The compiler translates C code into machine instructions.

For local variables, it also generates instructions that create stack frames, reserve stack space, and initialize variables whenever a function executes.

The compiler **does not** copy variables from Flash to RAM.

---

# Linker

The linker collects all object files and combines them into one executable.

It also places variables into their respective memory sections such as:

- .text
- .rodata
- .data
- .bss

The linker decides where each variable belongs in memory.

---

# Startup Code (Reset_Handler)

After the MCU resets, execution begins in the startup code before main().

Its important responsibilities are:

- Initialize the Stack Pointer.
- Copy the .data section from Flash to RAM.
- Clear the .bss section by filling it with zero.
- Jump to main().

The startup code **does not** initialize local variables or create stack frames.

Those are handled later by compiler-generated instructions whenever a function is called.

---

# Memory Sections in Flash 

## .text

    Contains executable program instructions.

    Stored in Flash.

## .rodata

    Contains read-only data such as constants.

    Typically stored in Flash because the program never modifies these values.

Example:


const int g5 = 300;


# Memory sections in RAM :

## .data

    Contains initialized variables with static storage duration.

    The initial values are stored in Flash.

    During startup, the Reset_Handler copies these initial values into RAM before main() executes.

Examples:


int g2 = 100;

static int g4 = 200;

static int total = 50;



## .bss

    Contains uninitialized variables with static storage duration.

    Since these variables have no explicit initial value, nothing is copied from Flash.

    Instead, during startup, the Reset_Handler clears the entire .bss section by writing zeros.

Examples:

int g1;

static int g3;

static int count;


## Stack

The stack is completely different from .data and .bss.

It is mainly used for:

- Automatic local variables
- Function parameters
- Return addresses
- Saved registers

The startup code does not initialize stack variables.

Whenever a function is called, the compiler generates instructions that adjust the Stack Pointer to reserve memory for local variables.

When the function returns, that stack frame is removed automatically.

Examples:


int sum;

int temp = 10;

int x;

int y = 20;


---

# Important Realization

One of the biggest takeaways from this study was understanding that different parts of the system have different responsibilities.

- The **compiler** generates instructions.
- The **linker** decides where variables belong in memory.
- The **startup code** prepares .data and .bss before main().

Understanding this made it much easier to reason about where each variable is stored and why.

