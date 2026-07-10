## My own imaginary tiny MCU

Let me design my own MCU first without using HAL, so that I'll better understand why STM32 is designed the way it is.
Here my goal is to 
    1. Read the whole port.
    2. Read one pin.
    3. Write one pin.
    4. Toggle one pin.
    5. Select the pin at runtime.

### My imaginary MCU :

Suppose I manufacture my own MCU. It has: PORTA with only 8 pins.

Pin0
Pin1
Pin2
Pin3
Pin4
Pin5
Pin6
Pin7

The question is how can I store the state of these 8 pins?
Each pin can be either  1 or 0 and hence each pin requires 1 bit. So therefere PORTA requires 8 bits, which is 1 byte. Each bit controls one pin.

If my port has 8 pins, each needing 1 bit, then what C data type could I choose to store the entire port? Unsigned Char because 8 pins -> 8 bits -> 1 byte -> unsigned char. For ex, if unsigned char PORT = 10110010, the how do I read a particular pin say pin 5? We know it's inside this byte. But accessing PORT gives the whole 10110010. Sometimes I need the whole port, and sometime just a particular pin. So, the value is same 10110010 is same. Just the view differs. Sometimes 10110010 as whole and sometimes one specific pin. So Which C feature lets us look at the same memory in different ways?

Union

 ├── View #1
 │      Whole byte 10110010
 │
 └── View #2
        Structure of bit-fields 
        
But Structure is simply a neat way to name the individual bits.

Something conceptually like:

Structure

Pin0 : 1 bit

Pin1 : 1 bit

Pin2 : 1 bit

...

Pin7 : 1 bit

The structure describes the layout.  The union lets that layout share memory with the whole byte.

We already know the whole port is: unsigned char PORT;

Now imagine we want another view:

Pin0

Pin1

Pin2

...

Pin7

Without worrying about unions yet, how would I write a struct that represents eight 1-bit pins? May be using bit-fields.
struct pins
{
    unsigned int pin0 : 1;
    unsigned int pin1 : 1;
    unsigned int pin2 : 1;
    unsigned int pin3 : 1;
    unsigned int pin4 : 1;
    unsigned int pin5 : 1;
    unsigned int pin6 : 1;
    unsigned int pin7 : 1;
} var;

Every member occupies 1 bit. But the sizeof (struct pins) = 4 bytes (Only 8 bits used!) since unsigned int requires 4 bytes. Isn't it Structure size = sum of all members ? why isn't the structure 32 bytes? Its because te compiler does not allocate memory member by member like it does for ordinary structure members. When its with bit fields, if members use the same underlying unsigned int storage, it creates one 32-bit (4 byte)container. They're all packed into the same 32-bit storage unit. So: sizeof(struct) = 4 bytes

Bit-fields are packed into the underlying type until there is no more room. 

#### Note : The exact memory layout of bit-fields is implementation-defined.
The C standard does not fully guarantee:

    whether bits are allocated from LSB to MSB,
    whether they cross storage boundaries,
    exact padding behavior.

The compiler decides.

For example:

Compiler A: bit0 → least significant bit

Compiler B: bit0 → most significant bit

That's why embedded code often uses:

GPIOA->MODER |= (1U << 10); with masks and shifts.
Bit-fields are convenient, but direct hardware register mapping with bit-fields has portability concerns.


Now, can we read the complete 32-bit/8-bit register directly? No. Because our structure only gives us individual fields: we can read as var.pino, var.pin1 etc. We need two views of the same memory: View 1: Whole port 10110010 View 2: pin0
pin1
pin2... So, structure vćan give only the individual pin view, and union can help with both views

Let me design the union that combines:

    Whole register access
    Bit-by-bit access


union pins
{
    unsigned int pin0 : 1;
    unsigned int pin1 : 1;
    unsigned int pin2 : 1;
    unsigned int pin3 : 1;
    unsigned int pin4 : 1;
    unsigned int pin5 : 1;
    unsigned int pin6 : 1;
    unsigned int pin7 : 1;
    
    unsigned int PORTA;
} var;
    
My hardware idea:

PORTA Register (1 byte)

Bit:

7   6   5   4   3   2   1   0

P7  P6  P5  P4  P3  P2  P1  P0

We want two ways to access the same byte.

View 1: Complete port
10101010

Access:

PORTA = 0xAA;

View 2: Individual pins
P7 = 1
P6 = 0
P5 = 1
...

Access:

P0 = 1;
P5 = 1;


Now the design idea:

                 UNION
              +----------+
              |          |
              | PORTA    | ---> 8 bits
              |          |
              +----------+
              |          |
              | pin bits | ---> same 8 bits
              |          |
              +----------+
              

struct PORTA_pins
{
    unsigned int PA0 : 1;
    unsigned int PA1 : 1;
    unsigned int PA2 : 1;
    unsigned int PA3 : 1;
    unsigned int PA4 : 1;
    unsigned int PA5 : 1;
    unsigned int PA6 : 1;
    unsigned int PA7 : 1;
} porta_pins; // No variable name, explanation is below. 
This is not yet connected to the hardware. And no memory exists yet.

Conceptually represents:

Bit position

7       6       5       4       3       2       1       0

PA7     PA6     PA5     PA4     PA3     PA2     PA1     PA0

So if memory contains:

10100110

then conceptually:

PA7 = 1
PA6 = 0
PA5 = 1
PA4 = 0
PA3 = 0
PA2 = 1
PA1 = 1
PA0 = 0

In real embedded code, we usually don't create a separate variable:

} porta_pins;

because we don't want a separate copy of the bits.

Remember our goal:

We want:

ONE MEMORY LOCATION
        |
        |
   +----+----+
   |         |
PORT byte   PA0-PA7

The structure is only a description of the layout.

So usually we write:
struct PORTA_pins
{
    ...
};

without: porta_pins;
Why? Because: } porta_pins; actually creates a variable.

Example:

struct PORTA_pins
{
    unsigned int PA0 : 1;
} porta_pins;

means: "Create a variable called porta_pins." Memory is allocated. But we only want to define a type/layout.

So our thinking becomes:

1. Define the bit layout:
struct PORTA_pins
{
    PA0 : 1;
    ...
    PA7 : 1;
};

2. Put this layout inside a union with:
    unsigned char PORTA;
    
    
struct PORTA_pins
{
    // bit fields
};


union GPIOA
{

    unsigned char PORTA;

    struct PORTA_pins pins; // Create a union member called pins whose type is struct PORTA_pins.

};

Visual memory:

union GPIOA

Same memory area

+----------------+
|                |
| unsigned char  |
| PORTA          |
|                |
+----------------+

+----------------+
|                |
| struct         |
| PORTA_pins     |
| pins           |
|                |
+----------------+

Both are looking at the same bytes.

If we write:

struct PORTA_pins portA;

Now memory is allocated.

Conceptually:

portA

Memory:

bit7 bit6 bit5 bit4 bit3 bit2 bit1 bit0
 ?    ?    ?    ?    ?    ?    ?    ?

PA7  PA6  PA5  PA4  PA3  PA2  PA1  PA0

Now we can do:

portA.PA0 = 1;

The compiler knows:

"PA0 is the first bit."

So internally it does something similar to:

Read byte

00000000


Set bit0


00000001

But here comes the embedded problem

Imagine the real MCU register:

GPIOA_ODR register

Address: 0x40020014


Memory:

Address        Value

0x40020014     10101010

This is the actual hardware.

We need our structure to sit exactly there.

Something like:

Our C structure

        ↓

0x40020014

bit7 bit6 bit5 bit4 bit3 bit2 bit1 bit0
 PA7  PA6  PA5  PA4  PA3  PA2  PA1  PA0
 
 Then:

portA.PA3 = 1;

would physically change the GPIO pin.

Currently:

struct PORTA_pins
        |
        |
        ↓
     RAM somewhere

But we need:

struct PORTA_pins
        |
        |
        ↓
GPIO hardware address
0x40020014

How?

Two concepts:

    1. Pointer
    2. Union

Suppose:

struct PORTA_pins portA;

and:

portA.PA3 = 1;

The compiler changes one bit. But what if you also want: portA = 0b10101010;

or:

read the complete 8-bit PORTA value

The bit-field gives you:

PA0
PA1
PA2
...

but not an easy:

whole 8 bits together

So the question:

How can we have BOTH views of the same memory?

View 1:

PA7 PA6 PA5 PA4 PA3 PA2 PA1 PA0

View 2:

10101010  (whole byte)

What C feature allows two different variables to share the exact same memory location? Union#

struct → gives separate memory for each member
union → all members share the same memory

1. Structure alone

Imagine:

struct PORTA_pins
{
    unsigned int PA0 : 1;
    unsigned int PA1 : 1;
    unsigned int PA2 : 1;
    unsigned int PA3 : 1;
    unsigned int PA4 : 1;
    unsigned int PA5 : 1;
    unsigned int PA6 : 1;
    unsigned int PA7 : 1;
};

Memory:

PORTA_pins

bit7 bit6 bit5 bit4 bit3 bit2 bit1 bit0

PA7  PA6  PA5  PA4  PA3  PA2  PA1  PA0


But we cannot easily say:

PORTA = 0xAA;

because there is no single 8-bit variable. Each member is 1 bit and not 8 bit.

2. Add a normal byte view

We create another way to look at the same memory:

unsigned char whole_port;

We want:

              SAME MEMORY

        +----------------+
        |                |
        |  10101010      |
        |                |
        +----------------+

          ↑          ↑

     whole_port   bit-fields
     
Both should point to the same 8 bits. That is exactly what union does.

3. Our first union

Conceptually:

union PORTA_register
{
    unsigned char whole_port;

    struct PORTA_pins pins;
};

Now memory:

union PORTA_register


Address
   |
   v

+----------------+
|                |
| 8 bits         |
|                |
+----------------+

    ↑        ↑
    |        |
whole_port  pins

Both members occupy the same location.

Example

Suppose:

PORTA.whole_port = 0b10101010;

Memory becomes:

bit7 bit6 bit5 bit4 bit3 bit2 bit1 bit0

 1    0    1    0    1    0    1    0

Now if we access:

PORTA.pins.PA7

we see:

PA7 = 1

because PA7 is bit 7.

And:

PORTA.pins.PA6

gives:

PA6 = 0

Same memory. Different view.