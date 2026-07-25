/*
  GPIO Single Register Modelling
  MCU: STM32F407VG
 
  Goal:
  Read a single pin
  Write a single pin
  Toggle a single pin
  
  Read a register
  Write a register
  Toggle a register
 
  Registers used:
 
  GPIOA_IDR : Input Data Register
  GPIOA_ODR : Output Data Register
 
 */


#include <stdint.h>

/*

GPIOA Peripheral Base Address:
STM32F407 Reference Manual:
GPIOA base address = 0x40020000
GPIOA_ODR Address = GPIOA Base + ODR Offset

*/

#define GPIOA_BASE          0x40020000U


/*
Register Offsets
GPIOA_IDR: Input Data Register
Offset = 0x10

GPIOA_ODR: Output Data Register
Offset = 0x14
*/

#define GPIOA_IDR_OFFSET    0x10U
#define GPIOA_ODR_OFFSET    0x14U



/*
ODR Register Bit-field Model
GPIOA_ODR is a 32-bit register.

Bits:
Bits 0-15: PA0 - PA15 output bits
Bits 16-31: Reserved
*/


struct GPIO_ODR_BITS
{
    unsigned int PA0  : 1;
    unsigned int PA1  : 1;
    unsigned int PA2  : 1;
    unsigned int PA3  : 1;
    unsigned int PA4  : 1;
    unsigned int PA5  : 1;
    unsigned int PA6  : 1;
    unsigned int PA7  : 1;

    unsigned int PA8  : 1;
    unsigned int PA9  : 1;
    unsigned int PA10 : 1;
    unsigned int PA11 : 1;
    unsigned int PA12 : 1;
    unsigned int PA13 : 1;
    unsigned int PA14 : 1;
    unsigned int PA15 : 1;


    /*
    Reserved bits: Bit 16 - Bit 31
    */

    unsigned int reserved : 16;
};



/*

ODR Union
Provides two views of the same register:
1. whole_register
    Access complete 32-bit register
2. pin
   Access individual GPIO pins
Both share the same memory.
*/


union GPIO_ODR_REGISTER
{
    unsigned int whole_register;

    struct GPIO_ODR_BITS pin;
};

/*
IDR Register Bit-field Model
GPIOA_IDR:
Bits 0-15:  Input state of PA0-PA15
Bits 16-31: Reserved
*/


struct GPIO_IDR_BITS
{
    unsigned int PA0  : 1;
    unsigned int PA1  : 1;
    unsigned int PA2  : 1;
    unsigned int PA3  : 1;
    unsigned int PA4  : 1;
    unsigned int PA5  : 1;
    unsigned int PA6  : 1;
    unsigned int PA7  : 1;

    unsigned int PA8  : 1;
    unsigned int PA9  : 1;
    unsigned int PA10 : 1;
    unsigned int PA11 : 1;
    unsigned int PA12 : 1;
    unsigned int PA13 : 1;
    unsigned int PA14 : 1;
    unsigned int PA15 : 1;


    unsigned int reserved : 16;
};



/*
IDR Union
Provides:
1. Complete register access
2. Individual pin access

*/


union GPIO_IDR_REGISTER
{
    unsigned int whole_register;

    struct GPIO_IDR_BITS pin;
};




/*
Hardware Register Mapping
The address calculation:
GPIOA_ODR:
GPIOA Base + ODR Offset
0x40020000 + 0x14 = 0x40020014

GPIOA_IDR: 0x40020000 + 0x10 = 0x40020010

volatile:
The hardware can change these registers independently.
Therefore compiler must always access the real address.
*/


#define GPIOA_ODR \
(*((volatile union GPIO_ODR_REGISTER *)(GPIOA_BASE + GPIOA_ODR_OFFSET)))


#define GPIOA_IDR \
(*((volatile union GPIO_IDR_REGISTER *)(GPIOA_BASE + GPIOA_IDR_OFFSET)))

int main(void)
{


    /*
    WRITE A PIN

    Requirement:
    Set PA5 HIGH
    Hardware effect:
    Bit 5 changes from 0 -> 1
    */


    GPIOA_ODR.pin.PA5 = 1;



    /*
    READ A PIN
    Check whether PA0 input is HIGH
    The hardware changes IDR based on the external voltage.
    */


    unsigned int button_state;


    button_state = GPIOA_IDR.pin.PA0;



    /*
    TOGGLE A PIN

    Toggle PA5

    XOR operation:

    0 becomes 1
    1 becomes 0
    */


    GPIOA_ODR.whole_register ^= (1U << 5);




    /*

    WRITE COMPLETE REGISTER
    Write complete 32-bit value.
    Example:
    PA5 and PA3 become HIGH.
    Binary:
    0000 0000 0010 1000
    */


    GPIOA_ODR.whole_register = 0x00000028;



    /*

    READ COMPLETE REGISTER
    Read complete GPIOA_ODR value.
    */


    unsigned int odr_value;


    odr_value = GPIOA_ODR.whole_register;




    /*
    
    TOGGLE COMPLETE REGISTER
    XOR with mask.
    Every bit set in the mask will toggle.
    Example:
    Mask:
    0000 0000 0010 1000
    Toggles PA5 and PA3 only.

    */


    GPIOA_ODR.whole_register ^= 0x00000028;



    while(1)
    {

    }


    return 0;
}

/*
Concepts covered:
 
  - Memory mapped registers
  - Register address calculation
  - volatile keyword
  - Bit-field structure
  - Union for multiple views
  - Read/write/toggle pin
  - Read/write/toggle complete register
  */
 