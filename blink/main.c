/*
 * File:   main.c
 * Author: Caleb Trepowski
 *
 */


#pragma config FOSC = XT
#pragma config WDTE = OFF
#pragma config PWRTE = OFF
#pragma config CP = OFf

#include <xc.h>

#define _XTAL_FREQ 8000000UL

void main(void) {
    TRISA = 0b00000000;
    while (1)
    {
        PORTA = 0b00000000;
        __delay_ms(1000);
        PORTA = 0b00000001;
        __delay_ms(1000);
    }
    return;
}
