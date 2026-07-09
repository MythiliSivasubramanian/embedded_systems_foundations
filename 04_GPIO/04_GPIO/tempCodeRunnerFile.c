#include <stdio.h>

struct pin_s
{
    unsigned int pin0 : 1;
    unsigned int pin1 : 1;
    unsigned int pin2 : 1;
    unsigned int pin3 : 1;
    unsigned int pin4 : 1;
    unsigned int pin5 : 1;
    unsigned int pin6 : 1;
    unsigned int pin7 : 1;
};

union port
{
    unsigned int PORTA;
    struct pin_s pins;
};

int main(void)
{
    struct pin_s var_s;
    union port var_u;

    var_s.pin0 = 1;
    var_s.pin3 = 1;

    printf("\n\nReading the pins from Structure\n");
    printf("\nPin 0 is %u\n", var_s.pin0); // 1
    printf("\nPin 1 is %u\n", var_s.pin1); // 0
    printf("\nPin 2 is %u\n", var_s.pin2); // 0
    printf("\nPin 3 is %u\n", var_s.pin3); // 1
    printf("\nPin 4 is %u\n", var_s.pin4); // 0

    // Assigning values for union members
    var_u.pins.pin2 = 1;

    printf("\n\nReading the pins from Union\n");
    printf("\nPin 0 is %u\n", var_u.pins.pin0); // 1
    printf("\nPin 1 is %u\n", var_u.pins.pin1); // 0
    printf("\nPin 2 is %u\n", var_u.pins.pin2); // 0
    printf("\nPin 3 is %u\n", var_u.pins.pin3); // 1
    printf("\nPin 4 is %u\n", var_u.pins.pin4); // 0

    return 0;
}