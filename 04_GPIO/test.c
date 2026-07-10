#include <stdio.h>


int selection(void);
void read_pin(unsigned char);


int main()
{
    unsigned char porta = 0b10100110;
    int choice = selection();
    
    // Switch case based on menu selection 
    
    // Read a particular pin
    read_pin(porta);
        
    
    return 0;
}

// Menu selection for the user
int selection(void)
{
    int option_selected;
    printf("\n\nChoose the number from below options :\n"
        "1. Read whole PortA\n"
        "2. Read a particular Pin\n"
        "3. Write a pin\n"
        "4. Toggle a pin\n"
        "5.Select the pin at runtime.\n\n");
    
    scanf("%d", &option_selected);
    return option_selected;
}

void read_pin(unsigned char porta, uint8_t pin)
{
    unsigned char pin; 
    printf("\nEnter the particular pin to read (0 - 7): ");
    scanf("%hhu", &pin);
    while (pin > 7)
    {
        printf("\nIts a 8 bit port, please enter a valid pin (0 -7) : ");
        scanf("%hhu", &pin);
    
    }
    
    unsigned char mask = (1U << pin);
    if ((porta & mask) == 0)
    {
        printf("\nThe pin %hhu is 0/not set\n", pin);
    }

    else
    {
        printf("\nThe pin %hhu is 1 / set\n", pin);
    }
    
}