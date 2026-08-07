# STM32 Startup Code & Reset Handler

## Index

- [What happens after MCU power ON?](#what-happens-after-mcu-power-on)
- [Reset Handler - Bridge between linker script and C program](#reset-handler---bridge-between-linker-script-and-c-program)
- [What happens to .bss during startup?](#what-happens-to-bss-during-startup)
- [How does the CPU know where Reset Handler is?](#How-does-the-CPU-know-where-Reset-Handler-is)


---
# What happens after MCU power ON?

After power ON, Flash contains data in various sections:
- `.text`
- `.rodata`
- `.data` initial image (LMA - Load Memory Address)

RAM does not contain useful information because its contents are lost after power OFF.

The CPU cannot directly jump to `main()` because before executing the application:
-   `.data` initial values from Flash must be copied to `.data` in RAM.
-   `.bss` section in RAM must be cleared to zero.
-   Stack must be initialized.

The Reset Handler performs these operations before calling `main()`. The Reset Handler is the first function that runs after MCU reset.

```text
Reset happens
      |
      |
Reset_Handler()
      |
      |
      ---- Initialize stack
      |
      ---- Copy .data
      |
      ---- Clear .bss
      |
      ---- Call main()
```
---

# Reset Handler - Bridge between linker script and C program

## .data section

Example:

```c
uint32_t global1 = 10;   // Initialized global variable (.data)

int main(void)
{
    global1 = 50;

    while(1);
}
```

Before Reset Handler runs, the initial value of `global1 = 10` is stored in the `.data` initial image in Flash (LMA). And the RAM does not contain any valuable information, since the content is lost after previous power off (Volatile))

Example:

```text
FLASH (LMA)
0x08004000

.data initial image
global1 = 10
```

RAM does not contain a guaranteed value:

```text
RAM (VMA)
.data
global1 = ?????
```
When the Reset Handler starts, it uses linker-generated symbols:

```c
extern unsigned int _sidata;
extern unsigned int _sdata;
extern unsigned int _edata;
```

These symbols provide:

```text
_sidata : FLASH Start address of .data initial image
_sdata : RAM Start address of runtime .data
_edata : RAM End address of runtime .data
```

The Reset Handler copies `.data` from Flash to RAM:

```c
unsigned int *source = &_sidata;
unsigned int *destination = &_sdata;


while(destination < &_edata)
{
    *destination = *source;

    destination++;
    source++;
}
```

It is: ```c destination < &_edata ``` and not ```c destination <= &_edata ``` because `_edata` points to the first address **after** the `.data` section.

Example:

```text
RAM

0x20000000   _sdata
     |
     |
 ----------------
| global1 = 10   | 4 bytes
 ----------------
| counter = 5    | 4 bytes
 ----------------
     |
     |
0x20000008   _edata
```

Therefore:

```text
_sdata = first address of .data
_edata = first address after .data
```

If we use ```c destination <= &_edata ``` then the loop will copy one extra word beyond `.data` and may corrupt the next RAM area. How many bytes does this copy? This line: ```c *destination = *source;```
does not get the size information from the linker script. The copy size is decided by the pointer data type. The pointer type is: ```c unsigned int ```

Here:

```c
unsigned int *source;
unsigned int *destination;
```

On STM32F407 (Cortex-M4): ```text unsigned int = 32 bits = 4 bytes ```. Therefore: ```c *destination = *source; ``` copies: 4 bytes from Flash to 4 bytes to RAM. 

Also ```c destination++; source++; ``` depends on the pointer type. Since the pointer is `unsigned int *`:
increment = sizeof(unsigned int) = 4 bytes. 

Example:

`destination = 0x20000000` and after ```c destination++; ``` becomes `destination = 0x20000004` But where is .data size defined? The linker calculates the size.

Example:

```c
int a = 10;
int b = 20;
int c = 30;
```
Each integer is 4 bytes. `.data` size = 12 bytes

Linker places:

```text
_sdata = 0x20000000

a -> 0x20000000
b -> 0x20000004
c -> 0x20000008

_edata = 0x2000000C
```

The Reset Handler only uses these addresses.

---

After copying:

```text
FLASH
.data initial image
global1 = 10

RAM
.data runtime
global1 = 10
```

Now `main()` starts.

If ```c global1 = 50; ``` executes, the new value 50 will be only in RAM and the Flash has only the initial value which is 10. (Flash is read only) and it stores only the initial image.
---

# What happens to .bss during startup?

Example:

```c
int global1 = 10;  // .data
int global2;       // .bss
```

The compiler places `global1 = 10`in .data section and  `global2`in .bss section. We would need a flash copy for .data section (from .data Flash to .data in RAM) since it has an initialised value, whereas .bss section doesnt need a copy as it doesnt contain any initialised value. 
As per C language rules, all uninitialized global variables must start with zero. The linker doesnt place `global2`in flash and assign with 0 which would waste the flash memory space. Rather linker places .bss section in RAM and Reset Handler assigns 0 to the entire .ss region before main().


The Reset Handler does:

```c
unsigned int *bss = &_sbss;
while(bss < &_ebss)
{
    *bss = 0;

    bss++;
}
```

---

The symbols `_sbss` and `_ebss` are created by the linker script:

```ld
.bss :
{
    _sbss = .;

    *(.bss)

    _ebss = .;

} > RAM
```

# How does the CPU know where Reset Handler is?

So far, we learned that, as soon as either MCU reset or MCU power on, the Reset Handler performs few operations (copy. data, clear .bss, call main()). But who tells the CPU the address of Reset Handler?

## Vector Table

For STM32 Cortex-M4, the very beginning of Flash contains a special table called as the Vector Table.

A simplified view:
```text
FLASH
0x08000000
|
|
 ---------------------- 
| Initial Stack Pointer |
 -----------------------
| Reset_Handler address |
 -----------------------
| NMI Handler address   |
 -----------------------
| HardFault address     |
 -----------------------
| ...                  |
 -----------------------

```
The CPU has a very fixed rule:

When reset happens:
-   Read the first word from Flash.
-   Load it into the Stack Pointer.
-   Read the second word from Flash.
-   Jump to that address.

Example:

```text
Imagine Flash contains:
Address          Value
--------------------------------

0x08000000       0x20020000

0x08000004       0x08000100
```

The CPU interprets this as:

First entry:

```text
0x08000000
        |

0x20020000   
This is the initial Stack Pointer.
```
So CPU does `SP = 0x20020000`. Now the stack is ready.

Second entry:

0x08000004
        |

0x08000100
This is the address of: Reset_Handler()
```
CPU does Pogram Counter `PC = 0x08000100`. Now execution starts from Reset_Handler.

**Visualization :**

```text

Power ON / Reset
        |
        |
CPU reads Vector Table from Flash
        |
        |
         ----------------------------
        |                            |
        |                            |

First entry                  Second entry

Initial SP                  Reset_Handler address

        |                            |
        |                            |

   Stack ready              Jump to Reset_Handler

                                     |
                                     |
                                     v

                              Copy .data
                              Flash ---> RAM

                                     |
                                     |
                              Clear .bss
                              RAM = 0
                                     |
                                     |

                              Call main()

```
The linker script places .text > FLASH.  The Reset Handler is a function, so its machine instructions are inside .text section and the address of the Reset Handler is stored in Vector table. The CPU reads the address from the Vector Table and jumps there.

```text

FLASH
Vector Table 
.text
   |
   +-- Reset_Handler() instructions
   +-- main() instruction
   +-- other functions instructions
.rodata
.data initial image

    
RAM
.data runtime
.bss
Stack
Heap
```

Now, we know that the compiler creates various sections (.data, .text, .rodata, .bss,..), the linker places these sections into the actual MCU memory and Reset Handler copies the .data sections and clears .bss before calling main(). The Vector Table in Flash contains the Stack Pointer address (1st entry) and the address of the Reset Handler as the second entry. But where does the Vector Table come from? I mean, who places this Vector table in Flash? It is usually defined in the startup file.

**Startup file**

```c
__attribute__((section(".isr_vector")))
const void *vector_table[] =
{
    &_estack,
    Reset_Handler,
    NMI_Handler,
    HardFault_Handler
};
```
Lets learn more about each line of above code later, but for now the important idea here is that the startup file creates a special section. called **`.isr_vector`**. 

The Vector Table is not normally part of .text. It has its own section `.isr_vector` created by the startup file.

The linker script places it at the very beginning of Flash:

```ld 
.isr_vector :
{
    KEEP(*(.isr_vector))
} > FLASH
```
** Flash Layout**

```text
FLASH
0x08000000
|
 --------------------
| Vector Table       |
|                    |
| Stack pointer      |
| Reset_Handler addr |
| Interrupt vectors  |
 --------------------
|
 --------------------
| .text              |
| Reset_Handler code |
| main()             |
| functions          |
 --------------------
|
 --------------------
| .rodata            |
 --------------------
|
 --------------------
| .data initial image|
 --------------------

```

**Linker Script with Vector info:**


```ld
SECTIONS
{
    .isr_vector :
    {
        KEEP(*(.isr_vector))
    } > FLASH


    .text :
    {
        *(.text)
    } > FLASH
    
    ....
}
```

```text
FLASH
0x08000000
 |
 |
  ---------------- 
 | .isr_vector    |
 | Vector Table   |
  ---------------- 
 |
  ---------------- 
 | .text          |
 | Reset_Handler  |
 | main()         |
  ---------------- 

```
