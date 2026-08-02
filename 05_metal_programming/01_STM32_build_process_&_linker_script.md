# STM32 Build Process & Linker Scripts

## The Complete Build Process

We already know:

```text
main.c  ---->  Compiler ----> main.o ----> Linker  ----> ELF ----> BIN

```

**Now lets deep dive into what the linker actually does.**

## Why the Linker Needs a Script

Imagine we compiled this:

```c
int global = 10;

const int MAX = 100;

int main(void)
{
    int local = 20;
}

```

The compiler creates sections like:

* `.text`
* `.data`
* `.rodata`
* `.bss`

But here's the important question: **How does the linker know where to place these sections?**

* `.text` belongs in Flash?
* `.rodata` belongs in Flash?
* `.data` should execute from RAM but be copied from Flash?
* `.bss` belongs in RAM?

The compiler doesn't know. The linker as well doesn't magically know either. Someone has to tell it. The linker script will have the information as to where to place these sections.

---

## The Memory Map

Before we even write a linker script, we must answer another question:

**What memory does the MCU actually have?**

For **STM32F407VG**:

```text
Flash
--------------------
Start : 0x08000000
Size  : 1 MB

RAM
--------------------
Start : 0x20000000
Size  : 128 KB

```

This information comes from the datasheet. Without knowing the available memory, writing a linker script is impossible.

---

## Building Our Own Linker Script

We'll start with something very small:

```ld
MEMORY
{
    FLASH (...)
    RAM (...)
}

```

Then we'll slowly add:

* `.text`
* `.rodata`
* `.data`
* `.bss`
* Stack
* Heap

One section at a time.

---

## Connect Everything Together

Eventually, we'll understand exactly how this works:

```text
main.c ---->Compiler----> main.o ----> Linker  --------------> ELF ----> Flash Image ----> MCU Boots ----> Reset Handler
                                        │ uses Linker Script                                                │
                                                                                                            ├── Copy .data
                                                                                                            ├── Clear .bss
                                                                                                            └── Call main()

```

---

## Key Topics Overview

* Compiler vs Linker (what each one does)
* What's inside a `.o` file (sections)
* Why the linker needs a linker script
* MCU memory map (Flash and RAM)
* Write our own linker script from scratch
* Startup code + Reset Handler + linker symbols

---

# Why Do We Even Need a Linker?

Before we learn the linker script, let's answer a much more fundamental question.

Imagine this program:

```c
int global = 10;

const int MAX = 100;

int main(void)
{
    int local = 20;

    return 0;
}

```

When we press **Build**:

```text
main.c ----> Compiler ----> main.o ----> Linker ---->  ELF

```

Let's deep dive into each step.

---

## What Does the Compiler Actually See?

The compiler does not know anything about STM32 memory addresses.

It does not know:

* Flash starts at `0x08000000`
* RAM starts at `0x20000000`
* `.data` will live in RAM
* `.bss` will live in RAM

The compiler only sees one C source file.

```c
int global = 10;

const int MAX = 100;

int main(void)
{
    int local = 20;
}

```

The compiler's job is simply to: *Convert this C code into machine instructions and organize the output into sections.*

It organizes into sections, but it does not place them into STM32 memory.

### What does the compiler produce?

It creates an object file.

```text
main.c ---> Compiler -----> main.o

```

Inside `main.o`, the compiler creates sections such as:

* `.text`
* `.data`
* `.rodata`
* `.bss`

But there are no STM32 addresses yet. Just sections.

So after compilation, let's imagine `main.o` looks like this:

```text
main.o

+----------------+
| .text          |
| machine code   |
+----------------+

+----------------+
| .data          |
| initial values |
+----------------+

+----------------+
| .rodata        |
| constants      |
+----------------+

+----------------+
| .bss           |
| uninitialized  |
+----------------+

```

**The compiler knows:**

* *"These instructions belong to `.text`"*
* *"This variable needs `.data`"*
* *"This constant belongs to `.rodata`"*
* *"This uninitialized variable belongs to `.bss`"*

**But the compiler does NOT know:**

* `.text` starts at `0x08000000`
* `.data` starts at `0x20000000`
* `.bss` starts at `0x200xxxxx`
* Where the stack should be placed

---
 
 ## The Linker

The compiler created organized pieces. But when we have multiple files, each file has these sections individually.

Imagine we have:

`main.o`

* `.text`
* `.data`
* `.rodata`
* `.bss`

And another file: `gpio.o`

* `.text`
* `.data`
* `.bss`

And another: `uart.o`

* `.text`
* `.data`

Now the linker receives three object files (`main.o`, `gpio.o`, `uart.o`) and each file has their own sections (`.text`, `.bss`, `.data`, `.rodata`).

So the linker has to merge all these sections from multiple files and place them in the final MCU memory.

For example:

```text
FLASH
0x08000000
 │
 ├── .text
 │
 └── .rodata

RAM
0x20000000
 │
 ├── .data
 │
 └── .bss

```

But the linker cannot guess. It needs instructions. Those instructions come from: **The Linker Script**.

---

## The Linker Script & Startup Code

The linker script is also responsible for creating information needed by startup code.

For example, the **Reset Handler** needs to know:

* Where does `.data` start in RAM?
* Where is the initial `.data` image in Flash?
* How big is `.data`?
* Where does `.bss` start?
* Where does `.bss` end?

The linker script creates symbols like:

* `_data_start`
* `_data_end`
* `_data_load`
* `_bss_start`
* `_bss_end`

Then the Reset Handler uses those addresses.

```text
C file ----> compiler ---> Object file (.o) -----------------------------> Linker + linker script ------------------> final one ELF
                                │Contains sections but no MCU address           │ multiple object files as input            │ contains 
                                 .text                                                                                      .text  → Flash address
                                 .data                                                                                      .data  → RAM address
                                 .rodata                                                                                    .bss   → RAM address
                                 .bss                                                                                           

```

**What exactly is inside an object file (`.o`)?**

Because when we write:

```c
int global = 10; 

```

How does the compiler decide that this variable belongs to `.data`? And when we write:

```c
const int MAX = 100;

```

Why does it go to `.rodata`? It is based on the C language rules.

The compiler classifies variables based on: Scope, Initialization, and the `const` qualifier.

---

For this code:

```c
int global = 10;

const int MAX = 100;

int main(void)
{
}

```

The compiler creates three things:

1. Machine instructions for `main()`
2. Initial value `10`
3. Constant value `100`

---

The compiler creates the sections: `.text`, `.data`, `.bss`, `.rodata`. But the compiler does not know the final memory addresses.

### Example:

```c
int global1 = 10;

int global2;

const int MAX = 100;

int main(void)
{
    int local = 20;
}

```

The compiler analyzes this and creates an object file `main.o`. Inside this object file, it has sections (all without any address):

```text
main.o

.text
 │
 └── machine instructions of main()

.data
 │
 └── global1 = 10

.bss
 │
 └── global2

.rodata
 │
 └── MAX = 100

```

## Who Assigns Addresses?

**The Linker.**

The linker uses a special file: **The Linker Script**.
The linker script tells it:

* FLASH starts at `0x08000000`
* RAM starts at `0x20000000`

Then it places sections:

```text
FLASH
0x08000000
 │
 +----------------+
 | .text          |
 | main() code    |
 +----------------+
 |
 +----------------+
 | .rodata        |
 | MAX = 100      |
 +----------------+
 |
 +----------------+
 | .data image    |
 | global1 = 10   |
 +----------------+


RAM
0x20000000
 │
 +----------------+
 | .data          |
 | global1        |
 +----------------+
 |
 +----------------+
 | .bss           |
 | global2        |
 +----------------+
 |
 +----------------+
 | Stack          |
 +----------------+

```

---

## The Complete Picture

```text
                  C Compiler
                     │
                     ▼
              Creates sections

        +----------------------+
        | main.o               |
        +----------------------+
        | .text                |
        | .data                |
        | .bss                 |
        | .rodata              |
        +----------------------+

                 Linker
                     │
                     ▼
              Linker Script
                     │
                     ▼

        +----------------------+
        | Final Memory Map     |
        +----------------------+

FLASH
0x08000000
.text
.rodata
.data initial values

RAM
0x20000000
.data
.bss
stack
heap

```

### Notes Summary:

1. Compiler creates sections.
2. Compiler decides what belongs in each section.
3. Compiler does not assign addresses.
4. Linker assigns addresses using the linker script.
5. Startup code moves `.data` from Flash -> RAM and clears `.bss`.

---
