/*
  03_gpio_peripheral_modelling.c
  Modelling GPIO pheripheral using C Structures
 
  - A peripheral is a collection of registers.
  - The structure represents the register layout and the pointer connects the structure 
    to the actual hardware address.
 */

#include <stdint.h>
/*
  GPIO peripheral register layout
  Each member represents one hardware register.
  The compiler automatically creates offsets:
  MODER   -> 0x00
  OTYPER  -> 0x04
  OSPEEDR -> 0x08
  PUPDR   -> 0x0C
  IDR     -> 0x10
  ODR     -> 0x14 
 */

typedef struct
{
    volatile uint32_t MODER;      // Offset 0x00
    volatile uint32_t OTYPER;     // Offset 0x04
    volatile uint32_t OSPEEDR;    // Offset 0x08
    volatile uint32_t PUPDR;      // Offset 0x0C
    volatile uint32_t IDR;        // Offset 0x10
    volatile uint32_t ODR;        // Offset 0x14

} GPIO_TypeDef;

/*
  GPIO peripheral base addresses
  STM32F407VG Reference Manual:
  GPIOA base = 0x40020000
  GPIOB base = 0x40020400 
 */

#define GPIOA_BASE   0x40020000UL
#define GPIOB_BASE   0x40020400UL

/*
  Creating peripheral instances
  Same structure.
  Different base address.
 
  GPIOA pointer points to GPIOA hardware registers.
  GPIOB pointer points to GPIOB hardware registers.
 */

GPIO_TypeDef GPIOA = (GPIO_TypeDef )GPIOA_BASE;
GPIO_TypeDef GPIOB = (GPIO_TypeDef )GPIOB_BASE;

int main(void)
{

    /*
      Accessing registers using structure members.
      GPIOA->ODR means: GPIOA base address + ODR offset
      0x40020000 + 0x14 = 0x40020014
    */

    GPIOA->ODR = 0x01;

    // GPIOA->IDR means: 0x40020000 + 0x10 = 0x40020010
    
    uint32_t pin_state;
    pin_state = GPIOA->IDR;

    /*
      GPIOB uses the same structure. Only the base address changes.
     GPIOB->ODR is 0x40020400 + 0x14 = 0x40020414
    */

    GPIOB->ODR = 0x01;
    while(1)
    {

    }

}