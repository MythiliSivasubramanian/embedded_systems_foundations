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

