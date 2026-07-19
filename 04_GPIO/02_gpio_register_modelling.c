
// Base address of GPIOA peripheral
#define GPIOA_BASE  0x40020000U

// Register offsets from GPIOA base address
#define GPIOA_ODR_OFFSET  0x14U
#define GPIOA_IDR_OFFSET  0x10U

// Macro mapping to hardware register addresses
#define GPIOA_ODR (*((volatile union ODR_PORT_A_REGISTER *)(GPIOA_BASE + GPIOA_ODR_OFFSET)))
#define GPIOA_IDR (*((volatile union IDR_PORT_A_REGISTER *)(GPIOA_BASE + GPIOA_IDR_OFFSET)))



/*
PORT A ODR (Output Data Register Port A) :
    - 32 pins (PA0,... PA31)
    - PA0 to PA15 not reserved (for user's use)
    - PA16 to PA31 (reserved)
*/

/*
Requirement:
    - Access the whole register value.
    - Access an individual pin such as PA5.
*/



// Complete 16 pins of ODR_PORTA : Structure
/*
    Base Type : unsigned int = 32 bits (4 bytes)
    1 bit is allocated for each pin 
*/
struct ODR_PORT_A_BITS
{
    unsigned int PA0 : 1;
    unsigned int PA1 : 1;
    unsigned int PA2 : 1;
    unsigned int PA3 : 1;
    unsigned int PA4 : 1;
    unsigned int PA5 : 1;
    unsigned int PA6 : 1;
    unsigned int PA7 : 1;
    unsigned int PA8 : 1;
    unsigned int PA9 : 1;
    unsigned int PA10 : 1;
    unsigned int PA11 : 1;
    unsigned int PA12 : 1;
    unsigned int PA13 : 1;
    unsigned int PA14 : 1;
    unsigned int PA15 : 1;
    
    // ODR_PORTA_PA16 to PA31 Reserved
    unsigned int Reserved : 16;
};

/*
Union : Size of union = size of largest member
32 bits (4 bytes) allocated
*/
union ODR_PORT_A_REGISTER
{
    unsigned int whole_register;  // 4 bytes or 32 bits
    struct ODR_PORT_A_BITS pin; // 4 bytes or 32 bits
};

// Complete 16 pins of IDR_PORT_A : Structure
/*
    Base Type : unsigned int = 32 bits (4 bytes)
    1 bit is allocated for each pin 
*/
struct IDR_PORT_A_BITS
{
    unsigned int PA0 : 1;
    unsigned int PA1 : 1;
    unsigned int PA2 : 1;
    unsigned int PA3 : 1;
    unsigned int PA4 : 1;
    unsigned int PA5 : 1;
    unsigned int PA6 : 1;
    unsigned int PA7 : 1;
    unsigned int PA8 : 1;
    unsigned int PA9 : 1;
    unsigned int PA10 : 1;
    unsigned int PA11 : 1;
    unsigned int PA12 : 1;
    unsigned int PA13 : 1;
    unsigned int PA14 : 1;
    unsigned int PA15 : 1;
    
    // IDR_PORT_A_PA16 to PA31 Reserved
    unsigned int Reserved : 16;
};

/*
Union : Size of union = size of largest member
32 bits (4 bytes) allocated
*/
union IDR_PORT_A_REGISTER
{
    unsigned int whole_register; // 32 bits or 4 bytes
    struct IDR_PORT_A_BITS pin; // 32 bits or 4 bytes
};

int main(void)
{
    // Conceptual register write: set PA5 high
    GPIOA_ODR.pin.PA5 = 1;
    return 0;
}



    