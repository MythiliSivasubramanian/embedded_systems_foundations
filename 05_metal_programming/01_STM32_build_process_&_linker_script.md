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