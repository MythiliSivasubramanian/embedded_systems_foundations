/*
  03_gpio_peripheral_modelling.c
  Modelling GPIO pheripheral using C Structures
*/

#include <stdint.h>

/*
  GPIO peripheral register layout Offset as per STM32F407VG reference manaual
  MODER   -> 0x00
  OTYPER  -> 0x04
  OSPEEDR -> 0x08
  PUPDR   -> 0x0C
  IDR     -> 0x10
  ODR     -> 0x14 
  BSRR    -> 0x18
  LCKR    -> 0x1C
  AFRL    -> 0x20
  AFRH    -> 0x24
 */

// The structure represents the Hadware register layout
typedef struct
{
    volatile uint32_t MODER;      
    volatile uint32_t OTYPER;     
    volatile uint32_t OSPEEDR;    
    volatile uint32_t PUPDR;      
    volatile uint32_t IDR;        
    volatile uint32_t ODR;        
    volatile uint32_t BSRR;       
    volatile uint32_t LCKR;       
    volatile uint32_t AFRL;       
    volatile uint32_t AFRH;       
    
} GPIO_TypeDef;

/*
  GPIO peripheral base addresses
  STM32F407VG Reference Manual:
*/

#define GPIOA ((volatile GPIO_TypeDef *) 0x40020000U)
#define GPIOB ((volatile GPIO_TypeDef *) 0x40020400U)
#define GPIOC ((volatile GPIO_TypeDef *) 0x40020800U)
#define GPIOD ((volatile GPIO_TypeDef *) 0x40020C00U)
#define GPIOE ((volatile GPIO_TypeDef *) 0x40021000U)
#define GPIOF ((volatile GPIO_TypeDef *) 0x40021400U)
#define GPIOG ((volatile GPIO_TypeDef *) 0x40021800U)
#define GPIOH ((volatile GPIO_TypeDef *) 0x40021C00U)
#define GPIOI ((volatile GPIO_TypeDef *) 0x40022000U)


int main(void)
{

  /*
  1. Whole Resgiter operations:
    a. Write a whole register
    b. Read a whole register
    c. Toggle a whole register (dangerous)
  
  2. Individual Pin Operations:
    a. Write (Set) a single pin (Write 1)
    b. Clear a single pin (Write 0)
    c. Read a single pin
    d. Toggle a single pin
    
  */
 
  //  1. Whole Resgiter operations:
  /* 1.a. Write whole register*/
  
  // Write 0x05 to GPIOB_ODR (Write 0x05 at 0x40020414 (GPIOB address. + ODR Offset))
  GPIOB -> ODR = 0x05;       // Same as (*GPIOB).ODR = 0x05
  
  /* 1.b. Read whole regsiter*/
  // A variable to store the register state
  uint32_t register_state;
  register_state = GPIOB -> IDR;
  
  
  /* 1.c. Toggle whole register (XOR) */
  /* Not recommended as it flips all bits, whcih may
  accidently change connections to other hardware
  */
 
  GPIOB->ODR ^= 0xFFFFFFFFU;  // Same as (*GPIOB).ODR ^= 0xFFFFFFFFU;
  // Same as (*GPIOB).ODR = (*GPIOB).ODR ^ 0xFFFFFFFFU;
  // Same as GPIOB->ODR = GPIOB->ODR ^ 0xFFFFFFFFU;
  
  
  //2. Individual Pin Operations:
  
  /* 2.a. Write / Set a single pin (write 1) */
  // Write GPIOB Pin5 1
  GPIOB->ODR |= (1U << 5);
  
  /* 2.b. Clear a single pin (write 0)*/
  // Clear a single pin GPIOB_ODR PIN 5
  GPIOB->ODR &=  ~(1U << 5);
 
  /* 2.c. Read a single pin */
  // Read a single pin GPIOC_IDR PIN 5
  if(GPIOC->IDR & (1U << 5))
    // or if ((GPIOC->IDR & (1U << 5)) != 0)
    {
      // pin5 is 1
    } 
  else
    {
      // pin5 is 0
    }
  
  
  /* 2.d. Toggle a single pin */
  // Toggle a single pin GPIOB_ODR PIN 2
  GPIOB->ODR ^= (1U << 2);
  
  
  
  return 0;
}

