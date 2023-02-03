/*
 * File:   newmain.c
 * Author: joseca
 *
 * Created on November 13, 2022, 2:43 PM
 */



// PIC16F1825 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FOSC = INTOSC       // Oscillator Selection (ECH, External Clock, High Power Mode (4-32 MHz): device clock supplied to CLKIN pin)
#pragma config WDTE = OFF        // Watchdog Timer Enable (WDT enabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable (PWRT disabled)
#pragma config MCLRE = OFF       // MCLR Pin Function Select (MCLR/VPP pin function is MCLR)
#pragma config CP = OFF         // Flash Program Memory Code Protection (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Memory Code Protection (Data memory code protection is disabled)
#pragma config BOREN = OFF       // Brown-out Reset Enable (Brown-out Reset enabled)
#pragma config CLKOUTEN = OFF   // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)
#pragma config IESO = ON        // Internal/External Switchover (Internal/External Switchover mode is enabled)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is enabled)

// CONFIG2
#pragma config WRT = OFF        // Flash Memory Self-Write Protection (Write protection off)
#pragma config PLLEN = OFF       // PLL Enable (4x PLL enabled)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)
#pragma config LVP = ON         // Low-Voltage Programming Enable (Low-voltage programming enabled)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <pic16f1825.h>
#include <stdio.h>
#include <stdlib.h>

#define _XTAL_FREQ 8000000UL

void putch(unsigned char byte){
    while(!TXIF){
        continue;
    }
    TXREG = byte;
}


void main(void) {
    
    OSCCON = 0b01110010; //pll off; 8 Mhz interno
    
    APFCON0 = 0b10000100; //Rx
    APFCON1 = 0; //Tx
    
    //puerto A
    PORTA = 0;
    TRISA = 0b00000110;//entadas 1 y 2
    ANSELA = 0b00000100; //2 es analogico
    
    ADCON1 = 0b01010000; //adc
    
    //del mismo modo configuramos puerto c
    PORTC = 0;
    TRISC = 0;
    ANSELC = 0;
    
    DACCON0 = 0;
    
    TXSTA = 0b00100000; //8 bit, con transmision asincrona a baja velocidad
    RCSTA = 0b10010000; //puerto serial activado, de 8 bits, con recepcion
    BAUDCON = 0b00000000;//baud rate <3>
    SPBRGL = 12; // POSC / baud * 64 -1
    SPBRGH = 0;
    
    
    //configurar el PWM
    CCP1CON = 0b00001100; //un solo canal
    CCPTMRS = 0; //timer 
    CCPR1L = 0;
    
    T2CON = 0b01111011; //relacion 1:16
    PR2 = 255; //registro
    
    printf("hola\r");
    
    
    // modbus
    
    // Interrupp de recepcion uart
    // Interrup por overflow,silencio
    
    PIR1 = 0;
    PIE1 = 0b00100001; // Cuales dispositivos van a generar interrupciones.
                       // Bit 5: Recepcion de un byte por UART.
                       // Bit 0: Timer 1 (Interrupción por Overflow)
    INTCONbits.GIE  = 1; // Global interrupt
    INTCONbits.PEIE = 1; // Peripheric interrupt
    
    // Timer 1
    TMR1   = 0;
    T1GCON = 0b10110001;
    T1CON  = 0b00110101;
    
    putch('@');
    T2CONbits.TMR2ON = 1;
    while(!PIR1bits.TMR2IF);
    
    while(1){
        RA4 = 1;
        __delay_ms(100);
        RA4 = 0;
        __delay_ms(100);
        
        CCPR1L++; //va aumentando hasta el maximo
        
        ADCON0 = 0b00001000; //canal 2
        ADON = 1;
        __delay_us(50);
        GO = 1;
        while(GO); //hasta que convierta todo
        ADON = 0;   
        
        /*if (PIR1bits.RCIF){ //pregunta si recibe byte
            if (RCREG == 'a'){
                while(1);
            }
        }*/
        TXREG = ADRESH; //transmision
    }
    
    
    
    
    return;
}

#define MODBUS_DIR 1
#define MAX_INDEX 8
unsigned char trama_modbus[] = {0,0,0,0,0,0,0,0};
unsigned char tm_index = 0;

unsigned int CRC16 (const unsigned char *nData, unsigned int wLength)
{
static const unsigned int wCRCTable[] = {
0X0000, 0XC0C1, 0XC181, 0X0140, 0XC301, 0X03C0, 0X0280, 0XC241,
0XC601, 0X06C0, 0X0780, 0XC741, 0X0500, 0XC5C1, 0XC481, 0X0440,
0XCC01, 0X0CC0, 0X0D80, 0XCD41, 0X0F00, 0XCFC1, 0XCE81, 0X0E40,
0X0A00, 0XCAC1, 0XCB81, 0X0B40, 0XC901, 0X09C0, 0X0880, 0XC841,
0XD801, 0X18C0, 0X1980, 0XD941, 0X1B00, 0XDBC1, 0XDA81, 0X1A40,
0X1E00, 0XDEC1, 0XDF81, 0X1F40, 0XDD01, 0X1DC0, 0X1C80, 0XDC41,
0X1400, 0XD4C1, 0XD581, 0X1540, 0XD701, 0X17C0, 0X1680, 0XD641,
0XD201, 0X12C0, 0X1380, 0XD341, 0X1100, 0XD1C1, 0XD081, 0X1040,
0XF001, 0X30C0, 0X3180, 0XF141, 0X3300, 0XF3C1, 0XF281, 0X3240,
0X3600, 0XF6C1, 0XF781, 0X3740, 0XF501, 0X35C0, 0X3480, 0XF441,
0X3C00, 0XFCC1, 0XFD81, 0X3D40, 0XFF01, 0X3FC0, 0X3E80, 0XFE41,
0XFA01, 0X3AC0, 0X3B80, 0XFB41, 0X3900, 0XF9C1, 0XF881, 0X3840,
0X2800, 0XE8C1, 0XE981, 0X2940, 0XEB01, 0X2BC0, 0X2A80, 0XEA41,
0XEE01, 0X2EC0, 0X2F80, 0XEF41, 0X2D00, 0XEDC1, 0XEC81, 0X2C40,
0XE401, 0X24C0, 0X2580, 0XE541, 0X2700, 0XE7C1, 0XE681, 0X2640,
0X2200, 0XE2C1, 0XE381, 0X2340, 0XE101, 0X21C0, 0X2080, 0XE041,
0XA001, 0X60C0, 0X6180, 0XA141, 0X6300, 0XA3C1, 0XA281, 0X6240,
0X6600, 0XA6C1, 0XA781, 0X6740, 0XA501, 0X65C0, 0X6480, 0XA441,
0X6C00, 0XACC1, 0XAD81, 0X6D40, 0XAF01, 0X6FC0, 0X6E80, 0XAE41,
0XAA01, 0X6AC0, 0X6B80, 0XAB41, 0X6900, 0XA9C1, 0XA881, 0X6840,
0X7800, 0XB8C1, 0XB981, 0X7940, 0XBB01, 0X7BC0, 0X7A80, 0XBA41,
0XBE01, 0X7EC0, 0X7F80, 0XBF41, 0X7D00, 0XBDC1, 0XBC81, 0X7C40,
0XB401, 0X74C0, 0X7580, 0XB541, 0X7700, 0XB7C1, 0XB681, 0X7640,
0X7200, 0XB2C1, 0XB381, 0X7340, 0XB101, 0X71C0, 0X7080, 0XB041,
0X5000, 0X90C1, 0X9181, 0X5140, 0X9301, 0X53C0, 0X5280, 0X9241,
0X9601, 0X56C0, 0X5780, 0X9741, 0X5500, 0X95C1, 0X9481, 0X5440,
0X9C01, 0X5CC0, 0X5D80, 0X9D41, 0X5F00, 0X9FC1, 0X9E81, 0X5E40,
0X5A00, 0X9AC1, 0X9B81, 0X5B40, 0X9901, 0X59C0, 0X5880, 0X9841,
0X8801, 0X48C0, 0X4980, 0X8941, 0X4B00, 0X8BC1, 0X8A81, 0X4A40,
0X4E00, 0X8EC1, 0X8F81, 0X4F40, 0X8D01, 0X4DC0, 0X4C80, 0X8C41,
0X4400, 0X84C1, 0X8581, 0X4540, 0X8701, 0X47C0, 0X4680, 0X8641,
0X8201, 0X42C0, 0X4380, 0X8341, 0X4100, 0X81C1, 0X8081, 0X4040 };

unsigned char nTemp;
unsigned int wCRCWord = 0xFFFF;

   while (wLength--)
   {
      nTemp = *nData++ ^ wCRCWord;
      wCRCWord >>= 8;
      wCRCWord ^= wCRCTable[nTemp];
   }
   return wCRCWord;

}
void procesar_trama(void)
{
    unsigned int crc16 = CRC16(trama_modbus, tm_index-1);
    unsigned char crc161o = crc16 & 0x00ff;
    if(trama_modbus[tm_index-1] == crc161o && trama_modbus[0] == MODBUS_DIR)
    {
        if(trama_modbus[1] == 6)
        {
            if(trama_modbus[2] == 0 && trama_modbus[3] == 0)
            {
                CCPR1L = trama_modbus[5];
                for(char i = 0; i<tm_index; ++i)
                    putch(trama_modbus[i]);
            }
        }
        else
            for(char i = 0; i<tm_index; ++i)
            {
                putch(trama_modbus[i]);
            }
    }
}
#define SILENCE_DELAY 73
void __interrupt() uart_in(void)
{
    GIE = 0;
    static unsigned char flag_capture = 0;
    static unsigned char silence_timer = 0;
    
    // Interrupción por desborde del Timer 1
    if(PIR1bits.TMR1IF)
    {
        PIR1bits.TMR1IF = 0;
        
        if(silence_timer < SILENCE_DELAY)
            silence_timer++;
        
        else if (tm_index)
        {
            procesar_trama();
            tm_index = flag_capture = 0;
        }
        
        TMR1 = 63035;
    }
    
    if(PIR1bits.RCIF)
    {
        PIR1bits.RCIF = 0;
        if(silence_timer >= SILENCE_DELAY && !tm_index)
            flag_capture = 1;
        silence_timer = 0;
        unsigned char b = RCREG;
        
        if (flag_capture && tm_index < MAX_INDEX)
            trama_modbus[tm_index++] = b;
    }
    GIE = 1;
}
