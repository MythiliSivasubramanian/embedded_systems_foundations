# Startup File / Code

Lets start by understanding why we need a startup Code. For example, when we write a program in C, the processor (Cortex M4) doesnt magically know that the program is in C and whenre the main() function is. The Cortex-M4 starts executing according to its reset architecture. The reset mechanism establishes the initial processor state, and the startup code then prepares the software/C runtime environment required before the application can safely execute.This process is done by startup code(commonly an assembly source file supplied by ST or generated as part of the project). Startup code is essentially the bridge between the processor's reset state and our C application.

Conceptually,
```text
                 HARDWARE WORLD
                       │
                       │ RESET
                       v
              Cortex-M4 processor
                       │
                       │
                       v
                Startup Code
                       │
              ┌────────┼─────────┐
              │        │         │
              v        v         v
             Stack   Memory    Runtime
                     setup      setup
                       │
                       v
                    main()
                       │
                       v
              C application

```
This Startup Code / Startup file is commonly an assembly source file supplied by ST or generated as part of the project. For example, something like `startup_stm32f407xx.s`. The exact filename can vary depending on the toolchain/project. Inside we'll typically find things such as,

```text
Vector table
Reset_Handler
Exception handlers
Stack definition
Weak interrupt handlers
```
And sometimes the startup file calls functions responsible for further initialization.

### Connection to Cortex M4 core Registers & Hardware Reset Architecture:

We just learnt from [M4_Cortex_Core_Registers](stm32-learning/docs/02_Cortex-M4 Core Registers/01_Cortex_M4_Core_Registers.md) that R13 is the Stack Pointer. And Cortex-M4 has MSP and PSP. Loading the initial MSP and Reset_Handler PC is done automatically by silicon logic (Hardware). And this is where the vector table becomes extremely important. The hardware performs some reset initialization before Reset_Handler runs. During reset, the Cortex-M4 hardware loads the initial MSP value from the first vector-table entry before executing Reset_Handler. Therefore, when Reset_Handler begins executing, MSP has already been initialized to the value specified by the vector table. So startup software does not initialize MSP by executing an instruction. The initial MSP setup happens as part of the processor's reset behavior.

The Cortex-M4 reset mechanism obtains two critical pieces of information from the beginning of the vector table:

```text
Vector Table

+0x00  -> Initial MSP value
+0x04  -> Reset_Handler address
```

So conceptually:

```text
Flash
0x08000000
     │
     ├──────────────────────┐
     │                      │
     v                      v
Initial MSP            Reset_Handler
value                  address
```

```text
             RESET
               │
               v
      Cortex-M4 reset hardware
               │
               │  reads Vector Table
               │
        ┌──────┴──────┐
        v             v
   Initial MSP    Reset_Handler
        │             │
        └──────┬──────┘
               v
         Startup Code
               │ (.data / .bss / runtime initialization)
               v
       Prepare C environment
               │
               v
             main()
```
This is our first bridge from Core Registers to Startup Code. Startup code is low-level code executed as part of the reset/startup path before the application's main() function. It provides the vector table and reset handler and participates in establishing the processor and C runtime environment, including .data and .bss initialization, before transferring control to the application.

### Why can't the processor simply jump to main()?

Because before main() executes, some important things needs to be established.

1. **The Stack must be ready :**
C functions use the stack.

```c
void function(void)
{
    int x = 10; // local initialised variable
}

```
Local variables, function calls, saved registers, etc. depend on a valid stack. So the processor needs a valid MSP.

2. **RAM variables need initialization :**

Consider `int counter = 10;`. The variable needs to exist in RAM with `counter = 10`.But the initial value is typically stored in the program image in Flash. Startup code is involved in getting that initialized data into RAM. The initial values of initialized global/static variables are typically stored in the Flash image, while their runtime storage is in RAM. Startup code copies the initial-value bytes from Flash to the .data section in RAM.

```text
                 Flash
             program image
                  │
                  │ initial value = 10
                  v
             10 stored here
                  │
                  │ startup copy
                  v
                  RAM
             .data section
                  │
                  v
             counter = 10
             
Flash -> initial-value storage
RAM   -> runtime storage

```

3. **.bss must be zeroed :**

```c
int counter; // global variable
static int flag; // global static variable
```
C requires these to start as `counter = 0`, `flag = 0`. The .bss section must be cleared to zero before the C program starts executing. Because .bss doesn't normally contain meaningful initial-value bytes in the Flash image. Startup code writes zeros into the corresponding RAM region.

4. **MCU/system initialization may be required :**
For STM32 projects, things such as early system/clock initialization may be performed through code called from the startup path, commonly SystemInit(). The startup path commonly calls SystemInit() for early MCU/system initialization, such as clock configuration. Then comes main(). The exact ordering depends on the startup/runtime implementation. In STM32 CMSIS-based projects, Reset_Handler commonly calls SystemInit() for early MCU/system initialization. This is a software convention provided by the STM32 software ecosystem, not a Cortex-M4 architectural requirement.

### Startup file vs Linker Script :

| Startup file | Linker script |
| --- | --- |
| Mainly code | Mainly memory/layout description |
| Usually assembly + symbols | Linker directives |
| Reset entry | Defines Flash/RAM regions |
| Vector table | Places sections |
| Reset_Handler | Determines `.text`, `.data`, `.bss` placement |
| Early initialization | Provides symbols used by startup code |

**The startup code relies on linker script symbols (e.g., _sidata, _sdata, _edata) to know the exact source memory addresses in Flash and destination memory addresses in RAM.**

```text
              Linker Script
                    │
                    │ defines layout
                    v
          Flash / RAM addresses
                    │
                    │ symbols
                    v
              Startup Code
                    │
                    v
             .data / .bss
                    │
                    v
                  main()               
```

```text
                                    +-----------------------+
                                    |     VECTOR TABLE      |
                                    | 0x08000000: Initial MSP|
                                    | 0x08000004: Reset_Hdr |
                                    +-----------+-----------+
                                                |
                         HARDWARE               | Reads entries 0 & 1
                         (Silicon Logic)        v
                                    +-----------------------+
                                    | Set SP (R13) = MSP    |
                                    | Set PC (R15) = Reset  |
                                    +-----------+-----------+
                                                |
                         SOFTWARE               | Executes instructions
                         (startup code)         v
                                    +-----------------------+
                                    | Copy Flash .data      |
                                    | Zero out RAM .bss     |
                                    | Call SystemInit()     |
                                    | Call main()           |
                                    +-----------------------+
```

#### Final Verification Summary

| Stage | Triggered By | Key Actions |
| --- | --- | --- |
| 1. Reset Vector Fetch | Hardware | Load initial MSP and Reset_Handler address |
| 2. Early System Initialization | Software | System/MCU initialization as required |
| 3. C Runtime Initialization | Software | Copy .data, clear .bss, runtime setup |
| 4. Application Entry | Software | Transfer control to main() |

**The Cortex-M4 architecture defines the reset behavior and exception/vector mechanism, but the startup file is largely implementation/toolchain/vendor-specific software that builds the environment needed by the C application.**

## The Vector Table
In the above lines, we came accross this word Vector Table many times, so what exactly is a vector table? What exactly is inside the Vector Table? **The vector table is a table of 32-bit vector values stored in memory. The first entry contains the initial MSP value, while subsequent entries contain the addresses of exception and interrupt handlers.**

For STM32F407, the vector table is normally located at the beginning of the Flash region `0x08000000` assuming the normal memory configuration where the vector table is mapped there.

### The First Two Entries in Vector Table :
The vector table starts something like this:
```text
Address        Contents
────────────────────────────────────
0x08000000     Initial MSP value
0x08000004     Reset_Handler address
0x08000008     NMI_Handler address
0x0800000C     HardFault_Handler address
etc
```
```text
Offset 0x00  -> Initial MSP
Offset 0x04  -> Reset vector
Offset 0x08  -> NMI
Offset 0x0C  -> HardFault
etc
```
The reset vector contains the address of Reset_Handler. Why does the second entry appear at +4? Because every entry is 32 bits, ie 4 bytes. Therefore, `0x08000000 + 4` = `0x08000004`. Imagine our STM32F407 program has, Initial stack address = `0x20020000`, Reset_Handler address = `0x08000101` Then memory might conceptually look like,
```text
             VECTOR TABLE

0x08000000 ┌──────────────────────┐
           │ 0x20020000           │
           │ Initial MSP value    │
           └──────────────────────┘

0x08000004 ┌──────────────────────┐
           │ 0x08000101           │
           │ Reset_Handler address│
           └──────────────────────┘

0x08000008 ┌──────────────────────┐
           │ NMI_Handler address  │
           └──────────────────────┘

0x0800000C ┌──────────────────────┐
           │ HardFault_Handler    │
           │ address              │
           └──────────────────────┘
```
***The vector table does not contain Reset_Handler as text. It contains a 32-bit address/value associated with Reset_Handler.*** 

### What Does the Hardware Do at RESET?
Now connect this with our [Core_Register_knowledge](/stm32-learning/docs/02_Cortex-M4 Core Registers/01_Cortex_M4_Core_Registers.md). When reset occurs, the Cortex-M4's reset mechanism obtains the initial stack pointer and reset vector from the vector table. Conceptually,
```text
                       RESET
                         │
                         v
                Cortex-M4 hardware
                         │
                         v
                  Vector Table
                         │
              ┌──────────┴──────────┐
              │                     │
              v                     v
       [0x08000000]           [0x08000004]
       Initial MSP            Reset vector
              │                     │
              v                     v
           MSP <- value           PC <- value
```
So if
```text
[0x08000000] = 0x20020000
[0x08000004] = 0x08000101

then conceptually:

MSP = 0x20020000
PC  = 0x08000101
```
before Reset_Handler begins executing. **The startup assembly does not need an instruction such as LDR SP, =0x20020000 to initialize the initial MSP. The reset mechanism obtains the initial MSP from the vector table (Hardfware - Silicon logic)**

### Why Is the Initial MSP at Address 0x08000000?
Because the Cortex-M architecture defines the reset vector-table mechanism. The processor needs to know where to begin finding its initial execution information. In the normal STM32F407 configuration, the vector table is located at the beginning of the Flash image `0x08000000`. So the first word is `0x08000000` -> initial MSP and the second word is `0x08000004` -> reset vector. The exact vector-table location can be changed later through the Cortex-M vector-table mechanism, but at reset we care about the reset mapping/configuration. We'll discuss VTOR later.


Noticed that the address of Reset_Handler is odd `0x08000101`istead of `0x08000100`. The short version is that Cortex-M4 executes Thumb instructions, and the reset vector has its least-significant bit set to indicate the required Thumb state. **`0x08000101`doesn't mean the instruction is physically located at an odd byte address.** The actual code address is aligned appropriately, while bit 0 carries state information. This connects directly to what we learned earlier about `EPSR.T`. We will deep dive into this later. 

### What else does Vector Table contain?
The first two entries are just the beginning. Conceptually,
```text
Vector Table
│
├── +0x00  -> Initial MSP
├── +0x04  -> Reset_Handler
├── +0x08  -> NMI_Handler
├── +0x0C  -> HardFault_Handler
├── +0x10  -> MemManage_Handler
├── +0x14  -> BusFault_Handler
├── +0x18  -> UsageFault_Handler
│
├── etc
│
└── External IRQ handlers
```
The table contains vectors for **Cortex-M system exceptions** such as,
```text
-   NMI
-   HardFault
-   MemManage
-   BusFault
-   UsageFault
-   SVCall
-   PendSV
-   SysTick
```
and then **External interrupts** such as STM32 peripherals,
```text
TIM2
TIM3
USART1
SPI1
I2C1
DMA
ADC
etx
```
The exact STM32F407 interrupt list comes from the STM32F407 interrupt/vector definitions, not from the generic Cortex-M4 architecture.

### Why Does the Timer Interrupt Need the Vector Table?
Suppose `TIM2 interrupt occurs`, the processor needs to know which address should it execute? and the vector table provides that association.
```text
TIM2 interrupt
      │
      v
Vector table
      │
      v
TIM2_IRQHandler address
      │
      v
TIM2_IRQHandler()
```
So the vector table is essentially the connection between an exception/interrupt number and its handler address.

| Hardware                            | Startup software          |
| ----------------------------------- | ------------------------- |
| Handles reset entry                 | Executes `Reset_Handler`  |
| Fetches initial MSP                 | Initializes `.data`       |
| Fetches reset vector                | Clears `.bss`             |
| Establishes initial execution state | May call `SystemInit()`   |
| Knows vector mechanism              | Eventually calls `main()` |

## Architecture to Actual code :
I am very much excited to build myself a minimal but real Cortex-M4 startup system for my STM32F407VG. 
Conceptually, 
```text
                    RESET
                      │
                      v
                VECTOR TABLE
                      │
          ┌───────────┴───────────┐
          v                       v
    Initial MSP              Reset_Handler
                                  │
                                  v
                         ┌─────────────────┐
                         │ Reset_Handler   │
                         └────────┬────────┘
                                  │
                                  v
                           SystemInit()
                                  │
                                  v
                       Copy .data Flash→RAM
                                  │
                                  v
                         Clear .bss → 0
                                  │
                                  v
                                main()
                                
Lets construct every box from scratch
```
### Understanding Requirements:
The first and foremost question is what our startup file must contain. So, what are the minimum information / Code required, so that the Cortex M4 can start executing our C Program/ application after Reset?

**Step 1: From Hardware`s Point of view :**
We already know what happens during reset,
```text
                 RESET
                   │
                   v
        -─────────────────────-
        │ Cortex-M4 hardware  │
        │ reads vector table  │
        -──────────┬──────────-
                   │
          ┌────────┴────────┐
          v                 v
       MSP value         Reset vector
          │                 │
          v                 v
      Initial SP       Initial PC
                            │
                            v
                     Reset_Handler
```
So the processor needs two things immediately,
1.    Where should the stack start?
The first vector-table entry must provide `Initial MSP`. For our STM32F407, from Reference manual, the starting address of SRAM mapped at address `0x2000 0000` and from Datasheet we know that SRAM is 192-Kbyte RAM in an LQFP100 package, which is our STM32F407.

Lets calculate the last addreess (exclusive last address and last usuable byte) for a memory block starting at the base address 0x20000000 with a size of 192 KB, 
1. **Convert the size to bytes :**
192 KB = 192 * 1024 = 196,608 bytes.
2. **[Convert the Size to hexadecimal:](#Convert-the-Size-to-hexadecimal):**
196,608 in hex = 0x30000
3. **Calculate the exclusive end address :**
0x20000000 + 0x30000 = 0x20030000
4. **Calculate the last usable byte (inclusive) :**
0x20030000 - 1 = 0x2002FFFF

So, `0x2002FFFF` is the inclusive last address, which is the final valid byte within the block.
`0x20030000` is the exclusive end address, which is typically used in linker scripts to define the upper bound of the segment. 

##### Convert the Size to hexadecimal:
Decimal 196,608 to Hex 0x30000
Method 1: Successive Division:
Divide the decimal number by 16 repeatedly, keeping track of the remainders. Read the remainders from the bottom to the top to get the final hex value.
1.    196,608 ÷ 16 = 12,288 with a remainder of 0 (Least Significant Digit)
2.    12,288 ÷ 16 = 768 with a remainder of 0
3.     768 ÷ 16 = 48 with a remainder of 0
4.     48 ÷ 16 = 3 with a remainder of 0
5.     3 ÷ 16 = 0 with a remainder of 3 (Most Significant Digit)

Reading bottom-to-top yields 30000. Adding the 0x radix prefix results in 0x30000.

Method 2: Positional Powers of 16 
To find the starting power of 16, look for the largest power of 16 that can fit inside your target number 
196,608 without exceeding it
1.    Its 16⁴. 16⁴ = 65,536.
2.    Divide the total byte size by this value 196,608 / 65,536 = 3.
3.    Because 65,536 fits into 196,608 exactly 3 times with a remainder of 0, the 16⁴ column is assigned a value of 3, and all lower column positions (16³, 16², 16¹, 16⁰) default to 0. Result is 0x30000.

```text
RAM:
0x20000000
     │
     │  192KB
     │
     │
0x2001FFFF    last final valid RAM byte
0x20020000   <- _estack
```
So, MSP = `0x20020000`

2.    Where should execution begin?
The second vector-table entry must provide the `Reset vector`which  contains the address of `Reset_Handler`. 
So Conceptually,
```text
0x08000000 -> initial MSP located at
0x08000004 -> Reset vector located at
```