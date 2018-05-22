#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "ds18b20.h"
volatile unsigned char i=100;
char current_temp;

uint8_t reset_pulse(void)
{
	
	uint8_t err;
	uint8_t sreg;
	
	OUT_LOW();	//ustawiamy port w stani niski
	DIR_OUT();	//ustawiamy pin na wyjscie
	_delay_us(480);	//odczekanie min 480s
	
	sreg=SREG;
	DIR_IN();	//ustawiamy pin na wiejscie
	
	_delay_us(66);
	err = GET_IN();	
		
	SREG=sreg; 
	
	_delay_us(480-66);
	
	if(GET_IN() == 0 )		
		err = 1;
		
	return err;
	
}


uint8_t send_bit(uint8_t b)	//wys³anie pojedynczego bitu na magistrale
{
	uint8_t sreg;
	
	sreg=SREG;
	
	DIR_OUT();	//kierunek pinu na wyjscie
	_delay_us(1); 
	
	if (b==1) 
		DIR_IN();	//zwalaniamy magistrale, pin bedize na wejscie
		
	_delay_us(15-1);
	
	if( OW_GET_IN() == 0 )		//jezeli w ospowiednim czasie nie nastapi podciagniecie magistrali to uznaje sie to za wyslanie zera logicznego
		b = 0;  
		
	_delay_us(60-15);
	DIR_IN();
	
	SREG=sreg; 
	
	return b;
	
}

uint8_t send_byte(uint8_t b)
{
	uint8_t i = 8, j;
	
	do {
		j = send_bit(b & 1);
		b >>= 1;
		if(j) b |= 0x80;
	} while( --i );
	
	return b;
}


uint8_t get_byte( void )
{
  return send_byte(0xFF); 
}


void convert_t(void)
{
	reset_pulse();	
	send_byte(0xCC);	//skip ROM
	send_byte(0x44);	//CONVERT T
	_delay_ms(6);
}

void temperature(void)
{
	int i=0; 					
 	char tmp[2];	//nas interasuj¹ 2 pierwsze bajty-temperatura 	

	convert_t();
    reset_pulse();			
	send_byte(0xCC);	//SKIP ROM
	send_byte(0xBE);	//READ SCRATCHPAD
	
	for (i=0;i<2;i++)
		tmp[i]=get_byte(); 
		
	current_temp = tmp[1]<<4 | tmp[0]>>4;
	
}
