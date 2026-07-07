
## GPIO General Purpose Input Output

STM32 MCU looks something like this:

```text
                     STM32 MCU

          +----------------------+ 
          |                      |
          |       CPU            |
          |                      |
          |                      |
          |   GPIO Peripheral    |
          |                      |
          +----------------------+ 

       PA0 PA1 PA2 PA3 ... PB0 PB1 ...
               │
               │
          Physical Pins
```

Those little metal legs around the MCU are the pins. They are how the MCU communicates with the outside world.

For example: LED, Button, Sensor, Motor, LCD, UART, SPI — everything eventually connects to pins.

Question 1

Suppose you connect an LED:

```text
      VCC
       │
       │
      LED
       │
       │
PA5 -------- STM32
```

How does the LED turn ON?

Can the CPU physically send electricity to the pin?

No.

The CPU doesn't have wires connected directly to the pins. Instead, the CPU talks to another hardware block called GPIO. GPIO is hardware — inside the STM32 silicon there is digital hardware dedicated to controlling pins.

Think of GPIO as the bridge between the CPU and the physical pins:

```text
CPU
 ↓
GPIO Peripheral
 ↓
Physical Pin
 ↓
LED
```

Suppose we execute: `GPIOA->ODR |= (1U << 5);`

Who actually changes the voltage on pin PA5? The GPIO hardware changes the voltage after receiving instructions from the CPU. The CPU modifies a GPIO register, and the GPIO hardware reacts to that register value.

How does the CPU talk to GPIO?

```text
CPU
   |
   |  writes a value
   v
GPIO Register
   |
   v
GPIO Hardware
   |
   v
Physical Pin
```

The CPU does not say "Hey pin PA5, become HIGH." Instead it writes to a GPIO register and the GPIO hardware continuously watches its registers.

But where are these registers?

- RAM has addresses.
- Variables have addresses.
- Pointers store addresses.

Hardware peripherals also have addresses.

Example (simplified):

```text
Memory Address

0x40020000  ---> GPIOA MODER Register
0x40020014  ---> GPIOA ODR Register
```

The GPIO registers are not normal RAM variables. They are special memory locations connected to hardware. This concept is called Memory-Mapped I/O.

Think of this:

Normally:

```c
int x;
x = 10;
```

means:

```text
CPU
   v
RAM location
   v
store 10
```

Nothing else happens.

But:

```c
GPIOA->ODR = 0x20;
```

means:

```text
CPU
   v
GPIOA ODR address
   v
Hardware sees the change
   v
Pin voltage changes
```

The same writing operation causes a physical event.

Now connect this with your pointer knowledge.

Earlier:

```c
int temperature = 35;
```

The compiler gives it an address. Imagine:

```text
RAM

Address       Value
5000          35
```

Then:

```c
int *p = &temperature;
```

Memory:

```text
RAM
5000          35
7000          5000   <- p
```

For GPIO, conceptually:

```text
Memory Map

0x40020014  → GPIOA ODR Register
                     |
                     v
               Physical pins
```

The address is fixed by the MCU design.

Let's imagine:

```c
/* preferred: include stdint.h for fixed-width types */
#include <stdint.h>

/* register macro using fixed-width type */
#define GPIOA_ODR (*(volatile uint32_t *)0x40020014)
```

Focus on the pointer. Break it down:

```c
(volatile uint32_t *)0x40020014
```

Treat the address `0x40020014` as a pointer to a `uint32_t`.

Notes:

- Use a fixed-width type (`uint32_t`) for clarity and portability across platforms.
- Use `volatile` for hardware registers so the compiler does not optimize away reads/writes that have side effects.

The `volatile` keyword tells the compiler the value at that address can change at any time (for example, due to hardware), so it must perform every read and write exactly as written in the source.

Many vendor SDKs expose registers via peripheral structs (e.g. `GPIOA->ODR`). That is semantically equivalent to a macro or pointer to the same underlying address. For example:

```c
/* struct-style access (provided by CMSIS/device headers) */
GPIOA->ODR |= (1U << 5);

/* explicit macro access to same address */
GPIOA_ODR |= (1U << 5);
```

Both lines write the same bit to the same hardware register when the addresses match.

## How does C code control a physical pin on an MCU? / How does the CPU communicate with hardware using C?

Eventually we want to understand this: `GPIOA->ODR |= (1U << 5);`

But before this line, we need to understand the chain:

```text
CPU
 |
 Memory address
 |
 Hardware register
 |
 Pin
```

So we are currently learning:

```c
int temperature = 35;
```

The compiler gives it an address. Imagine:

```text
RAM

Address       Value
5000          35
```

Then:

```c
int *p = &temperature;
```

Memory:

```text
RAM
5000          35
7000          5000   <- p
```

Now imagine a different situation.

Instead of creating `int temperature;` imagine the MCU designers already reserved a location:

```text
Memory

5000        ?????
```

They say: This address controls a motor. The address already exists. No C variable was created. No malloc. No RAM allocation. It is just a hardware location.

Now we want to access address 5000.

In normal C:

```c
int *p;
p = (int *)5000;
```

`p` can store an address where an integer exists.

Then:

```c
*p
```

means: go to the address stored inside `p` and access the value there.

Now the same idea without creating `p`:

```c
*(int *)5000 = 25;
```

Let's decode slowly:

Part 1: `5000` — just a number representing an address.

Part 2: `(int *)5000` — treat `5000` as an address of an integer (a pointer).

Part 3: `*(int *)5000` — go to address `5000` and access the integer stored there.

Part 4: `*(int *)5000 = 25;` — go to address `5000` and write `25` there.

Memory becomes:

Before:

```text
5000     ?????
```

After:

```text
5000     25
```

Now connect to hardware.

The MCU manufacturer says: Address `0x40020014` is connected to the GPIO output register.

So:

```c
*(unsigned int *)0x40020014 = value;
```

means: write `value` into the GPIO hardware register. The hardware sees that change and changes the pin.

The most important sentence:

Normal variable:

You create memory first:

```c
int x;
```

Then access it.

Hardware register:

Memory already exists:
The MCU manufacturer says:

Address 0x40020014

is connected to GPIO output register.

So:

*(unsigned int *)0x40020014 = value;

means:

Write value into the GPIO hardware register.

The hardware sees that change and changes the pin.

The most important sentence:
Normal variable:

You create memory first:

int x;

Then access it.

create x
   |
   v
memory exists
Hardware register:

Memory already exists:

hardware creates the address
             |
             v
you access it using C

You are not creating anything.

You are only telling C:

"Please treat this existing address as an integer."

Let's check your understanding with a simpler question.

Forget casting.

Imagine:

Address

5000 → 100

and:

int *p = 5000;

Question:

What does:

*p

give? 100
