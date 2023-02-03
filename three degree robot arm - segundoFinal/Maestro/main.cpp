// Segundo examen final 
// Jose Carlos Rios Parquet
// CI: 4.336.205

/////////////////////////////////////////////////////////////////////////////////////////

// Inclusiones
#include <stdlib.h>
#include <math.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

//Macros

#define F_CPU 16000000 //16M
#define SCL_CLK 100000L //10k
#define BITRATE(TWSR)	((F_CPU/SCL_CLK)-16)/(2*pow(4,(TWSR&((1<<TWPS0)|(1<<TWPS1)))))


#define SlaveWriteAddressX 0x20
#define SlaveReadAddressX	 0x21
#define SlaveWriteAddressY 0x22
#define SlaveReadAddressY	 0x23
#define SlaveWriteAddressZ 0x24
#define SlaveReadAddressZ	 0x25
#define	count 10

#define LCD_INST 0 // Led master, inst y datos
#define LCD_DATA 1

#define LINE1 0x80  // las direcciones de instrucciones
#define LINE2 0xC0
#define LINE3 0x94
#define LINE4 0xD4
#define CLEAR 0x01

#define SCALE 0.2483f
#define OFFSET 127


int lectura1,lectura2,lectura3;
char String1[5],String2[5],String3[5];



// LCD start (es casi como un refresh)
void lcd_start() {
	PORTB &=~ (1<<PORTB0);
	_delay_us(1);
	PORTB |=  (1<<PORTB0);
}

// LCD write
void lcd_write(uint8_t type, uint8_t inst_data) {
	if (type)
	PORTB |=  (1<<PORTB1);
	else
	PORTB &=~ (1<<PORTB1);

	PORTD = (PIND & 0x0F) | (inst_data & 0xF0);
	lcd_start();
	PORTD = (PIND & 0x0F) | ((inst_data<<4) & 0xF0);
	lcd_start();
	_delay_ms(2);
}

// Inicializacion
void lcd_init() {
	DDRB  |=  (1<<DDB0) | (1<<DDB1);
	DDRD  |=  0xF0;
	
	PORTB &=~ ((1<<PORTB0) | (1<<PORTB0));
	
	PORTD  = (PIND & 0x0F) | 0x30;

	_delay_ms(100);
	lcd_start();
	_delay_ms(5);
	lcd_start();
	_delay_us(100);
	lcd_start();
	_delay_us(100);

	PORTD = (PIND & 0x0F) | 0x20;
	
	lcd_start();
	_delay_us(100);

	lcd_write(LCD_INST, 0x2C);
	lcd_write(LCD_INST, 0x08);
	lcd_write(LCD_INST, 0x01);
	lcd_write(LCD_INST, 0x06);
	
	lcd_write(LCD_INST, 0x0C);
}

// El print del LCD
void lcd_print(char *s) {
	while(*s)
	{
		lcd_write(LCD_DATA, *s);
		s++;
	}
}

//I2C

//Inicializacion en 0
void I2C_Init() {
	TWBR = BITRATE(TWSR=0x00);
}

// I2C start, segun el estado retorna el valor int en el que se encuentra
uint8_t I2C_Start(char write_address) {
	uint8_t estado;
	TWCR = (1<<TWSTA) | (1<<TWEN) | (1<<TWINT);
	while ( !( TWCR & (1<<TWINT) ) );
	estado = TWSR&0xF8;
	if (estado != 0x08)
	return 0;
	TWDR = write_address;
	TWCR = (1<<TWEN) | (1<<TWINT);
	while (!(TWCR & (1<<TWINT)));
	estado = TWSR&0xF8;
	if (estado == 0x18)
	return 1;
	if (estado == 0x20)
	return 2;
	else
	return 3;
}

// se repite pero para la direcc de lectura
uint8_t I2C_Repeated_Start(char read_address) {
	uint8_t estado;
	TWCR=(1<<TWSTA)|(1<<TWEN)|(1<<TWINT);
	while(!(TWCR&(1<<TWINT)));
	estado=TWSR&0xF8;
	if(estado!=0x10)
	return 0;
	TWDR=read_address;
	TWCR=(1<<TWEN)|(1<<TWINT);
	while(!(TWCR&(1<<TWINT)));
	estado=TWSR&0xF8;
	if(estado==0x40)
	return 1;
	if(estado==0x48)
	return 2;
	else
	return 3;
}

//escribe y retorna el estado
uint8_t I2C_Write(char data) {
	uint8_t estado;
	TWDR = data;
	TWCR = (1<<TWEN) | (1<<TWINT);
	while (!(TWCR & (1<<TWINT)));
	estado = TWSR & 0xF8;
	if (estado == 0x28)
	return 0;
	if (estado == 0x30)
	return 1;
	else
	return 2;
}

//Acknowledge y NAcknowledge
char I2C_Read_Ack() {
	TWCR = (1<<TWEN) | (1<<TWINT) | (1<<TWEA);
	while (!(TWCR & (1<<TWINT)));
	return TWDR;
}
char I2C_Read_Nack() {
	TWCR = (1<<TWEN) | (1<<TWINT);
	while(!(TWCR & (1<<TWINT)));
	return TWDR;
}

// Stop
void I2C_Stop() {
	TWCR = (1<<TWSTO) | (1<<TWINT) | (1<<TWEN);
	while (TWCR & (1<<TWSTO));
}

// ADC
void ADC_init() {
	ADMUX &=~ (1<<ADLAR);
	ADMUX |=  (1<<REFS0);
	ADMUX &=~ (1<<REFS1);
	ADCSRA |= (1<<ADPS0);
	ADCSRA |= (1<<ADPS1);
	ADCSRA |= (1<<ADPS2);
}

int ADC_GetData(int channel) {

	ADMUX &=~  0x0F; //Multiplexor
	ADMUX |=  channel;
	
	// Se enciende el ADC y espera que convierta con un delay.
	ADCSRA |= (1<<ADEN);
	_delay_us(10);
	
	// Se manda la muestra.
	ADCSRA |= (1<<ADSC);

	// Se espera a que muestree, leyendo la bandera para luego reiniciarla.
	while(!(ADCSRA & (1<<ADIF)));
	ADCSRA |= (1<<ADIF);

	// Se apaga el ADC.
	ADCSRA &=~ (1<<ADEN);

	return ADC;
}


int main(void) {
	
	DDRC |= (1<<DDC2); //salidas c2 y c3
	DDRC |= (1<<DDC3);
	
	
	PORTC |= (1 << 3); //led encendido
	
	DDRD &=~ (1<<DDD3); //d3 como in
	
	
	MCUCR = 1<<ISC10 | 1<<ISC11; //interrupciones
	GICR = 1<<INT1;
	
	
	cli(); // no hay interrupciones
	lcd_init();
	I2C_Init();
	ADC_init();
	sei(); //hay interrupciones
	_delay_ms(10);
	
	// while true
	while (1) {
		lcd_write(LCD_INST,CLEAR);	//limpieza
		
        //El adc funciona con resolucion de 8 bits con signo
		lectura1 = ADC_GetData(0) * SCALE - OFFSET;
		itoa(lectura1, String1, 10); //integer to string
		
        lcd_write(LCD_INST, LINE1);
		lcd_print("E1:");
		
		lcd_write(LCD_INST, LINE1 + 3);
		lcd_print(String1);
		
		lectura2 = ADC_GetData(1) * SCALE - OFFSET;
		itoa(lectura2, String2, 10);

		lcd_write(LCD_INST, LINE1 + 8);
		lcd_print("E2:");
		
		lcd_write(LCD_INST,LINE1 + 11);
		lcd_print(String2);
		
		lectura3 = ADC_GetData(2) * SCALE - OFFSET;
		itoa(lectura3, String3, 10);

		lcd_write(LCD_INST, LINE2);
		lcd_print("E3:");

		lcd_write(LCD_INST, LINE2 + 3);
		lcd_print(String3);
		
	}
}

// Interruption, operacion
ISR(INT1_vect) {
	// Flanco
	PORTC |= (1<<PORTC2);
	_delay_ms(500);
	PORTC &=~ (1<<PORTC2);
	_delay_ms(10);

	//Escribir para el esclavo del eje X
	I2C_Start();
	_delay_ms(5);
	I2C_Write(lectura1);
	_delay_ms(500);
	I2C_Read_Ack();
	
	// Escribir para el esclavo del eje Y
	I2C_Repeated_Start();
	_delay_ms(5);
	I2C_Write(lectura2);
	_delay_ms(500);
	I2C_Read_Ack();

	// Escribir para el esclavo del eje Z
	I2C_Repeated_Start();
	_delay_ms(5);
	I2C_Write(lectura3);
	_delay_ms(500);
	I2C_Read_Nack();
	_delay_ms(50);
	
	I2C_Stop(); //Deteniendo la comunicacion

}
