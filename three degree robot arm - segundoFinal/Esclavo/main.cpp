//Segundo examen final
//Esclavo
//Jose Carlos Rios Parquet
//CI: 4.336.205
/////////////////////////////////////////////////////////////////////////////////////

// importaciones
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


//Estas deficiones son iguales a las del master, no necesita mucha explicacion.

#define SLAVE_ADDRESS 0x20
#define F_CPU 16000000 //16M

#define LCD_INST 0
#define LCD_DATA 1

#define LINE1 0x80
#define LINE2 0xC0
#define LINE3 0x94
#define LINE4 0xD4
#define CLEAR 0x01

//Constantes del PID
#define KP 1f
#define KI 3f
#define KD 0.01f







//Inicializamos variables a usar
int8_t count = 0;
int medicion = 0, i = 0, j = 0, k = 0, l = 0;
char stringLCD[5]="0000";

float error, up, ud; //se trabaja con errores y vector u para cada aporte (proporcional, integral, derivador)
float ui = 0.0, ui_ant = 0.0, error_ant = 0.0, ut = 0.0, d_error = 0.0;
float setpoint = 17;
int _freq = 0;

char SP1[10]; //slave setpoint char


// Funcion del PID
void pid(float pid_input) {
	
	const float max_x=100.0; //se definen limites de porcentaje
	const float min_x=-100.0;
	const float T=0.000001;

	error = setpoint - pid_input;
	up = KP * error;
	ui = ui_ant + (KI * T * error);

	d_error = error - error_ant;
	ud = KD * (d_error) / T;

	ut= up + ui + ud;

	if (ut > max_x) ut = max_x;
	if (ut < min_x) ut = min_x;

	ui_ant = ui;
	error_ant = error;
}

// PWM Init
void PWM_init(int freq) {
	TCCR0 = (1<<WGM00) | (1<<WGM01);
	_freq = freq;
	OCR0 = (F_CPU/1024/_freq) - 1;
}

//PWM On
void PWM_on() {
	TCNT0 = 0x0000;
	TCCR0 |=  (1<<CS00);
	TCCR0 &=~ (1<<CS01);
	TCCR0 |=  (1<<CS02);
}

// PWM Off
void PWM_off() {
	TCCR0 &=~ (1<<CS00);
	TCCR0 &=~ (1<<CS01);
	TCCR0 &=~ (1<<CS02);
}

// Set del PWM
void PWM_setDuty(int duty) {
	DDRB |= (1<<DDB3);
	TCCR0 &=~ (1<<COM00);
	TCCR0 |=  (1<<COM01);
	OCR0 = (((F_CPU / 1024 / _freq) - 1) * duty) / 100;
}

// LCD start
void lcd_start() {
    PORTB &=~ (1<<PORTB0);
    _delay_us(1);
    PORTB |=  (1<<PORTB0);
}
 
// LCD Write
void lcd_write(uint8_t type, uint8_t inst_data) {
    if (type)
    PORTB |=  (1<<PORTB1);
    else
    PORTB &=~ (1<<PORTB1);
    PORTC = (PINC & 0x0F) | (inst_data & 0xF0);
    lcd_start();
    PORTC = (PINC & 0x0F) | ((inst_data<<4) & 0xF0);
    lcd_start();
    _delay_ms(2);
}
 
// LCD Init
void lcd_init() {
    DDRB  |=  (1<<DDB0) | (1<<DDB1);
    DDRC  |=  0xF0;
    PORTB &=~ ((1<<PORTB0) | (1<<PORTB0));
    PORTC  = (PINC & 0x0F) | 0x30;
    _delay_ms(100);
    lcd_start();
    _delay_ms(5);
    lcd_start();
    _delay_us(100);
    lcd_start();
    _delay_us(100);
    PORTC = (PINC & 0x0F) | 0x20;
    lcd_start();
    _delay_us(100);

    lcd_write(LCD_INST, 0x2C);
    lcd_write(LCD_INST, 0x08);
    lcd_write(LCD_INST, 0x01);
    lcd_write(LCD_INST, 0x06);
    
    lcd_write(LCD_INST, 0x0C);
}  

//LCD Print
void lcd_print(char *s) {
    while(*s) {
        lcd_write(LCD_DATA, *s);
        s++;
    }
}
 
//Slave Init
void I2C_Slave_Init(uint8_t SLAVE_ADDRESS) {
    TWAR = SLAVE_ADDRESS;
    TWCR = (1<<TWEN) | (1<<TWEA) | (1<<TWINT);
}

//slave listen
int8_t I2C_Slave_Listen() {
    while(1) {
        uint8_t status;
        while (!(TWCR & (1<<TWINT)));
        status = TWSR & 0xF8;
        if (status == 0x60 || status == 0x68)
        return 0;
        if (status == 0xA8 || status == 0xB0)
        return 1;
        if (status == 0x70 || status == 0x78)
        return 2;
        else
        continue;
    }
}

//slave transmit
int8_t I2C_Slave_Transmit(char data) {
    uint8_t status;
    TWDR = data;
    TWCR = (1<<TWEN) | (1<<TWINT) | (1<<TWEA);
    while (!(TWCR & (1<<TWINT)));
    status = TWSR&0xF8;
    if (status == 0xA0) {
        TWCR |= (1<<TWINT);
        return -1;
    }
    if (status == 0xB8)
    return 0;
    if (status == 0xC0) {
        TWCR |= (1<<TWINT);
        return -2;
    }
    if(status == 0xC8)
    return -3;
    else
    return -4;
}

//slave receive
char I2C_Slave_Receive() {
    uint8_t status;
    TWCR=(1<<TWEN) | (1<<TWEA) | (1<<TWINT);
    while(!(TWCR & (1<<TWINT)));
    status = TWSR & 0xF8;
    if (status == 0x80 || status == 0x90)
    return TWDR;
    if(status == 0x88 || status == 0x98)
    return TWDR;
    if(status == 0xA0) {
        TWCR |= (1<<TWINT);
        return -1;
    }
    else
    return -2;
}

//main
int main(void) {
    cli();
    I2C_Slave_Init(SLAVE_ADDRESS);

    DDRD &=~ (1<<DDD1); //D1 encoder
    DDRD &=~ (1<<DDD2); //D2 encoder
    DDRD &=~ (1<<DDD3); //D3 interrupt

    DDRB |= (1<<DDB3); //B3 PWM

    DDRB |= (1<<DDB6); //B6 Out para IN1
    DDRB |= (1<<DDB7); //B7 Out para IN2

    PORTB |= (1<<PORTB6); //B6 en alto
    PORTB &=~ (1<<PORTB7); //B7 en bajo
    TCCR0 &=~ (1<<COM00);
    TCCR0 |= (1<<COM01);

    //Interrupciones
    MCUCR |= 1<<ISC00 | 1<<ISC01;
    GICR |= 1<<INT0;
    MCUCR |= 1<<ISC10 | 1<<ISC11; 
    GICR |= 1<<INT1;

    // se setea el PWM y se habilitan las interrupciones
    PWM_init(200);
    PWM_setDuty(50);
    lcd_init();

    sei();
    PWM_on();

    //Manejo de velocidad
    while (1) { //while true
        pid(medicion);
        if(ut<0) {
            ut = -ut;
            PORTB |= (1<<PORTB7);
            PORTB &=~ (1<<PORTB6);
        }
        else {
            PORTB |= (1<<PORTB6);
            PORTB &=~ (1<<PORTB7);
        }
        PWM_setDuty(ut);	 
    }
}

// Instrucciones en interrupcion
ISR(INT0_vect) {
	if (PIND & (1<<PIND1)) { //entonces se aumenta la medicion y el contador i
		medicion++;
		i++;		
	}
	else{
		medicion--;	
	}
	

    //impresion LCD
	lcd_write(LCD_INST, CLEAR);
	itoa(medicion,cadenaLCD,10);

	lcd_write(LCD_INST,);
	lcd_print("SetPoint:");

	itoa(setpoint,SP1,10);
	lcd_write(LCD_INST,+10);
	lcd_print(SP1);

	lcd_write(LCD_INST,);
	lcd_print("Pulsos:");

	lcd_write(LCD_INST,+8);
	lcd_print(cadenaLCD);

	lcd_write(LCD_INST,+14);
	lcd_print("E3");
	
}

ISR(INT1_vect) {
	char String1[10];
	int exit = 0;
	I2C_Slave_Init(SLAVE_ADDRESS);
	while (1) {
		switch (I2C_Slave_Listen()) {
			case 0: {
				lcd_write(LCD_INST, CLEAR);
				lcd_write(LCD_INST, LINE1);
				lcd_print("Nuevo SetPoint:");

				do {	
					lcd_write(LCD_INST, LINE2);
					itoa(count, String1, 10);
					lcd_print(String1);
					setpoint = count;
					count = I2C_Slave_Receive();
					
				} while (count != -1);

				if(count == -1) {
					exit = 1;
				}
				count = 0;
				break;
			}
			
			case 1: { //Escritura
				int8_t ackStatus;
				lcd_write(LCD_INST, LINE1);
				lcd_print("Se envia:  ");
				do {
					ackStatus = I2C_Slave_Transmit(count);
					lcd_write(LCD_INST, LINE2);
					lcd_write(LCD_DATA, count + '0');
					count++;
				} while (ackStatus == 0);
				break;
			}
			default:
			break;
		}
		if(exit == 1) {
			exit = 0;
			break;
		}
	}
}