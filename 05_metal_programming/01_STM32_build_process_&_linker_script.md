# STM32 Build Process & Linker Scripts

## Table of Contents
* [The Complete Build Process](#the-complete-build-process)
* [The Memory Map](#the-memory-map)
* [Building Our Own Linker Script](#building-our-own-linker-script)
* [Connect Everything Together](#connect-everything-together)
* [Key Topics Overview](#key-topics-overview)
* [The Linker](#the-linker)
* [Writing Our Own Memory Layout for STM32](#writing-our-own-memory-layout-for-stm32)
  * [1. The SECTIONS Block](#1-the-sections-block)
    * [`.text` Placement](#text-placement)
  * [2. `.data` Placement](#2-data-placement)
    * [Load Address vs. Virtual Address](#load-address-vs-virtual-address)
    * [What Happens During Startup & Execution?](#what-happens-during-startup--execution)
  * [3. `.bss` Placement](#3-bss-placement)
  * [Complete Memory Map Visualization](#complete-memory-map-visualization)
  * [Linker Symbols: The Bridge to Startup Code](#linker-symbols-the-bridge-to-startup-code)
  * [Visualizing the Complete Execution Flow](#visualizing-the-complete-execution-flow)
  
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

# Writing Our Own Memory Layout for STM32

```text
Power ON
    │
    ▼
Flash and RAM exist
    │
    ▼
Compiler creates sections
    │
    ▼
Linker places sections into memory
    │
    ▼
Reset Handler runs
    │
    ▼
.data copied to RAM
.bss cleared
    │
    ▼
main()

```

Imagine **STM32F407VG**. We already know:

* **Flash** starts at `0x08000000`
* **RAM** starts at `0x20000000`

The linker script does not create `.text`. The compiler already did that. The linker script simply says where to place these sections.

---

## Declaring MCU Memory to the Linker Script

How does the linker script know how much Flash and RAM the MCU has?

For example, our STM32F407VG has:

* **Flash:** 1 MB
* **RAM:** 128 KB

How does the linker know this? Inside the linker script, the very first thing we usually write is the memory layout of the MCU:

```ld
MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 1024K
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
}

```

`FLASH(rx)`. This is simply a name. It could technically be `PROGRAM_MEMORY` or `ABC`—the linker doesn't care. But everyone calls it `FLASH` because that's what it represents. `ORIGIN = 0x08000000` represents that flash begins here: `0x08000000`. Exactly the same address from the STM32 reference manual. `LENGTH = 1024K` meaning Flash size is 1024 KB. Exactly what the datasheet tells us. Similarly for RAM.

---

It now builds a mental picture like this:

```text
                STM32F407VG Memory

Flash
0x08000000
+----------------------------------+
|                                  |
|                                  |
|                                  |
|                                  |
+----------------------------------+
          1024 KB


RAM
0x20000000
+----------------------------------+
|                                  |
|                                  |
|                                  |
|                                  |
+----------------------------------+
           128 KB

```

We still haven't placed `.text` or `.data`. We've only told the linker about the available memories. Only after this does the linker know where it is allowed to place sections.

---

## The SECTIONS Block in Linker Script

Think of the linker script as having two major parts:

```text
Linker Script
 │
 ├── MEMORY
 │     │
 │     └── What memories exist?
 │
 └── SECTIONS
       │
       └── Which section goes into which memory?

```
Suppose we write:

* `.text` -> FLASH
* `.rodata` -> FLASH
* `.data` -> RAM
* `.bss` -> RAM

Now it can build the final executable.

---

## The Complete Flow

This is one of the most important diagrams in embedded systems:

```text
                 C Source Files
                        │
                        ▼
                  Compiler
                        │
                        ▼
              Object Files (.o)

        .text
        .data
        .bss
        .rodata

                        │
                        ▼
               Linker + Linker Script
                        │
          ┌─────────────┴─────────────┐
          │                           │
     MEMORY Block              SECTIONS Block
  What memory exists?       Where does each section go?
          │                           │
          └─────────────┬─────────────┘
                        │
                        ▼
                 Final ELF File
                        │
                        ▼
              Ready to be programmed
              into the STM32 Flash

```

---

## A Real Linker Script

A simplified STM32 linker script looks like this:

```ld
MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 1024K
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
}

SECTIONS
{
    .text :
    {
        *(.text)
    } > FLASH

    .rodata :
    {
        *(.rodata)
    } > FLASH

    .data :
    {
        *(.data)
    } > RAM

    .bss :
    {
        *(.bss)
    } > RAM
}

```

### What is SECTIONS?

Think of it like this:

```text
MEMORY
========
Available memories
- FLASH
- RAM


SECTIONS
========
- .text    → FLASH
- .rodata  → FLASH
- .data    → RAM
- .bss     → RAM

```

Notice the difference:

* **`MEMORY`** = What memories exist?
* **`SECTIONS`** = Where should each section go?

The linker script is referring to the compiler's `.text` section.

---

#### What does `*(.text)` mean?

Imagine our project has three C files: `main.c`, `gpio.c`, and `uart.c`.

After compilation we get: `main.o`, `gpio.o`, and `uart.o`.

Each object file contains its own `.text` section:

```text
main.o  -----\
              \
gpio.o  -------> Final .text
              /
uart.o  -----/

```

So `*(.text)` is basically telling the linker to take every `.text` section from EVERY object file (`*`) and put them together here.

What does `> FLASH` mean? `FLASH` is simply the memory name we defined earlier in the `MEMORY` block.

So:

```ld
.text :
{
    *(.text)
} > FLASH

```
Place the final combined `.text` section into the memory region called `FLASH`.

---
The linker never opens our C source files. It never looks at:

```c
int main(void)
{
}

```

or

```c
void GPIO_Init(void)
{
}

```

It only works with object files: `main.o`, `gpio.o`, `uart.o`, `timer.o`.

### Summary of Compiler Output vs. Linker Script Responsibilities

So the compiler creates sections:

* `.text` ->  Machine code instructions
* `.rodata`  ->  Constants / read-only data
* `.data`  ->  Initialized global & static variables (initial values)
* `.bss`  ->  Uninitialized global & static variables

The Linker Script:

* Defines available memories (`MEMORY`)
* Decides where sections should finally live (`SECTIONS`)
* Combines all input sections from multiple `.o` files into final output sections

---

## 1. The SECTIONS Block

A simplified linker script:

```ld
SECTIONS
{
    .text :
    {
        *(.text)
    } > FLASH

    .data :
    {
        *(.data)
    } > RAM

    .bss :
    {
        *(.bss)
    } > RAM
}

```

Now we have given explicit instructions to the linker.

### `.text` Placement

```ld
.text :
{
    *(.text)
} > FLASH

```

This tells the linker to take all input `.text` sections from all compiled object files and combine them into the final `.text` section located in FLASH.

#### Before linking:

Multiple `.o` files, each containing individual sections:

```text
main.o
.text
 │
 ├── main()
 └── function1()

gpio.o
.text
 │
 └── gpio_init()

uart.o
.text
 │
 └── uart_send()

```

#### After linking:

Only one unified `.text` section placed in Flash:

```text
FLASH
0x08000000
 │
 ▼
.text
 ├── main()
 ├── function1()
 ├── gpio_init()
 └── uart_send()

```

---

## 2. `.data` Placement

Example:

```c
int global = 10;

```

The compiler places `global = 10` inside the `.data` section.

However, the value `10` cannot exist only in RAM at startup, because RAM contents are volatile and undefined after a power reset. How would `global` hold the value `10` immediately after reset if RAM starts empty?

To solve this, the linker assigns two different addresses to `.data`:

1. **Load Address (LMA / Flash):** Where the initial value is permanently stored.
2. **Virtual / Execution Address (VMA / RAM):** Where the variable lives and mutates during runtime.

### Load Address vs. Virtual Address

**Load Address (LMA in Flash):**

```text
FLASH
+---------------------+
| .data initial image |
| global = 10         |
+---------------------+

```

**Virtual Address (VMA in RAM):**

```text
RAM
+---------------------+
| .data runtime section|
| global = 10         |
+---------------------+

```

Our simplified linker script previously looked like:

```ld
.data :
{
    *(.data)
} > RAM

```

This only tells the linker that the runtime location (VMA) is RAM. It says nothing about where the initial values are stored in Flash.

### Real STM32 Linker Script Syntax: `> RAM AT > FLASH`

Real STM32 linker scripts use this syntax:

```ld
.data :
{
    *(.data)
} > RAM AT > FLASH

```

The clause `> RAM AT > FLASH` explicitly tells the linker:

* **Runtime location (VMA):** `> RAM` (Run the `.data` section from RAM)
* **Initial storage location (LMA):** `AT > FLASH` (Store its initial contents in FLASH)

Now the linker structures memory like this:

```text
FLASH
----------------------
.text
.rodata
.data initial image
----------------------

RAM
----------------------
.data (runtime)
----------------------

```

### What Happens During Startup & Execution?

1. **Before MCU runs:** Flash stores the initial values.

```text
FLASH (LMA)
.text
.rodata
.data initial image
-------------------
global = 10
counter = 5

```

2. **During Reset:** The Reset Handler copies the initial image from Flash to RAM before `main()` executes:

```text
FLASH (.data initial image)
           │
           ▼ (Reset Handler copies)
RAM (.data runtime section)

```

3. **After Copying:**

```text
RAM (VMA)
global = 10
counter = 5

```

Now `main()` starts.

4. **Runtime Changes:**
If your program later executes:

```c
global = 99;

```

RAM becomes:

```text
RAM
global = 99

```

Flash remains unchanged:

```text
FLASH
global = 10

```

Nothing writes back to Flash. The `.data` section executes from RAM, but its initial contents are preserved in Flash.

---

## 3. `.bss` Placement

Example:

```c
int counter;

```

The compiler puts uninitialized variables inside `.bss`. Because uninitialized variables have no custom initial value, they do not need storage space in Flash. The linker script simply places `.bss` in RAM:

```ld
.bss :
{
    *(.bss)
} > RAM

```

```text
RAM
.bss
 ├── counter
 ├── flag
 └── buffer

```

During startup, the Reset Handler iterates through the `.bss` region and writes `0` to every byte. After that, `counter == 0`.

---

## Complete Memory Map Visualization

Consider this complete program:

```c
int global1 = 10;
int global2;
const int MAX = 100;

int main(void)
{
    int local;
    return 0;
}

```

### Compiler Output (`main.o`):

```text
main.o
.text   → machine instructions for main()
.data   → global1 = 10
.bss    → global2
.rodata → MAX = 100

```

### After Linker Processing:

```text
FLASH
0x08000000
 │
 +----------------------+
 | .text                |
 | main()               |
 +----------------------+
 | .rodata              |
 | MAX = 100            |
 +----------------------+
 | .data initial image  |
 | global1 = 10         |
 +----------------------+

RAM
0x20000000
 │
 +----------------------+
 | .data (runtime)      |
 | global1 = 10         |
 +----------------------+
 | .bss                 |
 | global2 = 0          |
 +----------------------+
 |                      |
 | Stack (grows down)   |
 | local variable       |

```

---

## Linker Symbols: The Bridge to Startup Code

The linker script specifies `.text > FLASH`, `.data > RAM AT > FLASH`, and `.bss > RAM`. But how does the Reset Handler actually know:

1. Where `.data` starts in Flash?
2. Where `.data` starts in RAM?
3. Where `.data` ends in RAM?

Linker symbols helps with these information. Scroll further down for Linker symbols. 

### Defining Symbols in the Linker Script

```ld
.text
{
    *(.text)
} > FLASH

.rodata
{
    *(.rodata)
} > FLASH

.data :
{
    _sdata = .;
    *(.data)
} > RAM AT > FLASH

_sidata = LOADADDR(.data);

```
In the above linker script, .text section instructs to take all .text section for all files and to place as one .text section in FLASH. Similary for .rodata. 
But .data section instructs to place the .data section in two locations RAM and in FLASH aswell. One is Load memory Access (LMA) and the other is Vitual Memory Access (VMA).

**Load Memory Address (LMA)** is where the initial value is stored before startup. Usually in FLASH.

Example:

FLASH
.data initial image
global = 10

**Virtual Memory Address (VMA)** is where the variable lives during normal execution. Usually in RAM

After Reset Handler:
RAM
.data
global = 10

Later when this variable global is changed to 50, then it is updated only in RAM and not in FLASH.

So the linker script needs to express both. Hence we write as the below meaning Runtime location : RAM
Initial copy : FLASH

```ld 
.data :
{
    *(.data)
} > RAM AT > FLASH
```
Here AT is the linker script keyword, meaning Load address at. 

Now, the next line `*(.data)` means take all .data sections from all object files and place them here.


---

## How Startup Code Uses These Symbols

In C startup code (`startup.c`), the Reset Handler accesses these linker-generated symbols using the `extern` keyword:

```c
extern unsigned int _sidata;
extern unsigned int _sdata;
extern unsigned int _edata;

void Reset_Handler(void)
{
    unsigned int *source = &_sidata;
    unsigned int *destination = &_sdata;

    // Copy .data initial values from Flash to RAM
    while (destination < &_edata)
    {
        *destination = *source;
        destination++;
        source++;
    }

    // Call main application
    main();
}

```

---

## Visualizing the Complete Execution Flow

```text
Power ON ⚡
   │
   ▼
FLASH (0x08000000)
 ├── .text (Reset_Handler, main)
 ├── .rodata
 └── .data initial image (global = 25)
   │
   ▼
RAM (0x20000000)
 ├── .data (global = ?)
 └── .bss
   │
   ▼
Reset Handler executes:
 ├── _sidata points to Flash (global = 25)
 ├── _sdata points to RAM (destination start)
 ├── Copies bytes from Flash to RAM until _edata
 └── Zero-fills .bss section
   │
   ▼
RAM becomes initialized:
 └── global = 25
   │
   ▼
main() executes

```

---

## Crucial Embedded Concept

The linker script **does not copy anything by itself**.

The linker script only states:

> *"The initial data starts here in Flash, and the runtime data belongs there in RAM."*

The actual memory transfer (`FLASH`  ->  `RAM`) is performed at runtime by the **Reset Handler code**.

```text
Linker Script
     │
     ▼ (creates symbols)
 _sidata, _sdata, _edata
     │
     ▼ (used by)
 Reset Handler
     │
     ▼ (copies)
 .data from Flash to RAM
     │
     ▼
   main()

```

## .bss section:

Example : ```c int counter; ``` (without an initial value / uninitialized global variable.) belongs to the .bss section, and it only needs RAM.

So does Flash need to store an initial value? No. Because there is no initial value. The C rule says, the uninitialized variables must start with 0. Hence the startup code simply creates zero in RAM.

Linker script for the .bss section:

```ld
.bss
{
    *(.bss)
} > RAM
```
1.  Collect all .bss sections from object files.
2.  Combine them into one final .bss.
3.  Reserve RAM space for them.

### Startup difference between .data and .bss:

For .data the startup operation is to copy the data from Flash to RAM where as the startup operation in .bss section is to clear .bss section in RAM(no copy from Flash to RAM). So During startup, the Reset Handler has to set each byte in .bss to 0.

## How does the Reset Handler know where .data starts, where it ends, and where to copy it?

We just wrote the below script, but the Startup code needs addresses like source address, destionation address and length. The linker provides these information using linker symbols.

```ld
.data
{
    *(.data)
} > RAM AT > FLASH
```

### Linker script symbols

 

