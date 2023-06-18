/*
 * ATMega16_I2C.c
 *
 * Created: 2023-06-15 8:54:12 AM
 * Author : a_shi
 */ 

#define F_CPU 8000000UL

#define lcd PORTA

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

//Functions

char decimal_to_bcd(unsigned char value)
{
	return (((value/10)<<4) | (value%10));
}

void hex_to_ASCII(unsigned char value)
{
	unsigned char bcd;
	bcd = value;
	bcd = bcd & 0xf0;
	bcd = bcd >> 4;
	bcd = bcd | 0x30;
	
	lcd_data(bcd);
	
	bcd = value;
	bcd = bcd & 0x0f;
	bcd = bcd | 0x30;
	
	lcd_data(bcd);
	
}


//Initialize I2C
void i2c_Init()
{
	TWSR =0;
	TWBR = ((F_CPU/100000)-16)/(2*pow(4,(TWSR & (1<<TWPS0)|(1<<TWPS1))));	//Get bit rate register value by formula, and set prescaler 64.
}

//I2C Start function

uint8_t i2c_Start(char write_Address)
{
	TWCR = (1<<TWSTA) | (1<<TWEN) | (1<<TWINT);		//Enable TWI, generate Start condition, and clear interrupt flag.
	while (!(TWCR & (1<<TWINT)));					//Wait for TWINT Flag set. This indicates that the START condition has been transmitted.
	
	TWDR = write_Address;							//Load SLA_W into TWDR Register.
	
	TWCR = (1<<TWINT) | (1<<TWEN);					//Enable TWI and clear interrupt flag.
													
	while (!(TWCR & (1<<TWINT)));					//Wait until TWI finish its current job (Write Operation)
	
}

//I2C Repeated STart Function

uint8_t i2c_RepeatedStart(char read_Address)
{
	TWCR = (1<<TWSTA) | (1<<TWEN) | (1<<TWINT);		//Enable TWI, generate Start condition, and clear interrupt flag.
	while (!(TWCR & (1<<TWINT)));					//Wait for TWINT Flag set. This indicates that the START condition has been transmitted.
	
	TWDR = read_Address;							//Load SLA_W into TWDR Register.
	
	TWCR = (1<<TWINT) | (1<<TWEN);					//Enable TWI and clear interrupt flag.
	
	while (!(TWCR & (1<<TWINT)));		//Wait for TWINT Flag set. This indicates that the SLA+W has been transmitted, and ACK/NACK has been received.
	
}

//I2C Stop Function

void i2c_Stop()
{
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);		//Transmit STOP condition.
	while(!TWCR & (1<<TWSTO));						//Wait until Stop condition execute.
}

//I2C Write Function

uint8_t i2c_Write(char data)
{
	TWDR = decimal_to_bcd(data);				//Copy data in TWI data register.
	TWCR = (1<<TWINT) | (1<<TWEN);				//Enable TWI and clear interrupt flag.
	while (!(TWCR & (1<<TWINT)));				//Wait until TWI finish its current job (Write Operation)
}

//I2C Read acknowledge function

char i2c_ReadAck()
{
	TWCR = (1<<TWEN) | (1<<TWINT) | (1<<TWEA);		//Enable TWI, generation of ack and clear interrupt flag.
	
	//Wait for TWINT Flag set. This indicates that the DATA has been transmitted, and ACK has been received.
	
	while((!TWCR & (1<<TWINT)));						//Wait until TWI finish its current job (Write Operation)
	return TWDR;
}

//I2C Read Nack Function

char i2c_ReadNack()
{
	TWCR = (1<<TWEN) | (1<<TWINT);					//Enable TWI and clear interrupt flag.
	
	//Wait for TWINT Flag set. This indicates that the DATA has been transmitted, and NACK has been received.
	
	while((!TWCR & (1<<TWINT)));						//Wait until TWI finish its current job (Write Operation)
	return TWDR;
}

//LCD*********************************************************LCD*********

void lcd_cmd(unsigned char c)
{
	lcd=((c&0xF0) + 0x04);
	_delay_ms(1);
	lcd=lcd-0x04;
	lcd=((c<<4)& 0xF0) +0x04;
	_delay_ms(1);
	lcd=lcd-0x04;
}

void lcd_Init()
{
	lcd_cmd(0x02); //to set the home position
	lcd_cmd(0x28); // 4bit mode, 0x38 8 bit mode
	lcd_cmd(0x06); // increment
	lcd_cmd(0x01); //clear lcd
	lcd_cmd(0x0C); // set cursor OFF
}

void lcd_data(unsigned char d)
{
	lcd=((d&0xF0) + 0x05);
	_delay_ms(1);
	lcd=lcd-0x04;
	lcd=((d<<4) & 0xF0) + 0x05;
	_delay_ms(1);
	lcd=lcd-0x04;
}

void lcd_str(char namo[])
{
	int i=0;
	while(namo[i]!='\0')
	{
		lcd_data(namo[i]);
		i++;
	}
}


//******************************************************

uint8_t data=0;

int main(void)
{
    sei();
	DDRA = 0xFF;			//PORTA as output.
	
	//DDRB |= (1<<DDB0) | (1<<DDB1) | (1<<DDB2);
	DDRC = 0x00;
	PORTC = 0xFF;
	
	lcd_Init();
	lcd_cmd(0x80);
	lcd_str("ASHISH");
	lcd_cmd(0xC0);			//Start at 2nd line. 
	
	i2c_Init();
	i2c_Start(0xA0);
	i2c_Write(32);
	i2c_Write(45);
	i2c_Write(98);
	i2c_Stop();
	_delay_ms(1000);
	
	i2c_Start(0xA0);
	i2c_Write(32);
	i2c_Write(45);
	i2c_RepeatedStart(0xA1);
	data = i2c_ReadAck();
	i2c_Stop();
	
	lcd_cmd(0xC0);
	hex_to_ASCII(data);
	
	
    while (1) 
    {
    }
}

