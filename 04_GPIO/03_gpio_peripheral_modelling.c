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
  
  2. Individual Pin Operations: (ODR)
    a. Write (Set) a single pin (Write 1) using ODR
    b. Clear a single pin (Write 0) using ODR
    c. Read a single pin
    d. Toggle a single pin
    
  3. Atomic Operations using BSRR:
    a. Set a single pin using BSRR
    b. Reset/Clear a single pin using BSRR
    c. Set multiple pins simultaneously using BSRR
    d. Reset/Clear multiple pins simultaneously using BSRR
    e. Simultaneously Set some pins and Reset other pins using BSRR
    f. Set and Reset the same pin simultaneously (conflict situation)
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
  
  // 3. Atomic Operations using BSRR (No Read-Modify-Write needed, single cycle):
  
  /* 3.a. Set a single pin using BSRR */
  // Set GPIOB Pin 5 HIGH (Writing to lower 16 bits: BS5)
  GPIOB->BSRR = (1U << 5);
  
  /* 3.b. Reset / Clear a single pin using BSRR */
  // Reset GPIOB Pin 5 LOW (Writing to upper 16 bits: BR5 -> Shift offset by 16)
  GPIOB->BSRR = (1U << (5 + 16));
  
  /* 3.c. Set multiple pins simultaneously using BSRR */
  // Set GPIOB Pin 0, Pin 2, and Pin 5 HIGH at the exact same instant
  GPIOB->BSRR = (1U << 0) | (1U << 2) | (1U << 5);
  
  /* 3.d. Reset / Clear multiple pins simultaneously using BSRR */
  // Reset GPIOB Pin 0, Pin 2, and Pin 5 LOW at the exact same instant
  GPIOB->BSRR = (1U << (0 + 16)) | (1U << (2 + 16)) | (1U << (5 + 16));
  
  /* 3.e. Simultaneously Set some pins and Reset other pins using BSRR */
  // Set Pin 1 and Pin 3 HIGH, while Resetting Pin 4 and Pin 6 LOW in one single instruction
  GPIOB->BSRR = (1U << 1) | (1U << 3) | (1U << (4 + 16)) | (1U << (6 + 16));
  
  /* 3.f. Conflict resolution: Set AND Reset the same pin in a single BSRR write */
  // Attempting to set Pin 5 HIGH (1U << 5) AND reset Pin 5 LOW (1U << (5 + 16)) simultaneously:
  // Hardware Priority: Set takes precedence over reset. The pin will be SET (HIGH).
  GPIOB->BSRR = (1U << 5) | (1U << (5 + 16));
  
  return 0;
}

