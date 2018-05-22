#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include "ds18b20.h"
#include <avr/eeprom.h>
#include <avr/interrupt.h>

EEMEM uint8_t eeprom_address;
int flag = 0;

volatile unsigned int usart_bufor_ind;           
char usart_bufor[30];

int set_temp;
static const uint8_t PROGMEM numbers[13]={0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0x78,0x80,0x90,0xBF,0x98,0xFF};
	
volatile unsigned char received_value;
volatile unsigned char received_flag=0;
char usart_bufor_received[255];


void init_ports(void)												
{
	DDRB = 0xFF;
	PORTB = 0x00;
	DDRD = 0xC7;
	PORTD = 0xFF;
	DDRC = 0xF0;												
	PORTC = 0xFF;
	DDRA = 0xFF;
	PORTA = 0x00;
}

void tune(unsigned int frequency, unsigned int time)
{
	unsigned int i, t, n;
	t = 125000/frequency;
	n = (250UL*time)/t;

	PORTD |= 0x04;

	for(i=0; i < n; i++)
	{
		PORTD ^= 0x04;
		_delay_loop_2(t);
	}
}

void init_usart(void)  
{  
 	 #define BAUD 9600         
	 #include <util/setbaud.h>
   
	 UBRRH = UBRRH_VALUE;    
	 UBRRL = UBRRL_VALUE;  
	 #if USE_2X  
	   UCSRA |=  (1<<U2X);  
	 #else  
	   UCSRA &= ~(1<<U2X);  
	 #endif  
 
	 UCSRC = (1<<URSEL) | (1<<UCSZ1) | (1<<UCSZ0);    
  
	 UCSRB = (1<<TXEN) | (1<<RXEN) | (1<<RXCIE);   
} 

ISR(USART_RXC_vect){    
 received_flag=1;
 sprintf(usart_bufor_received, "%d", UDR);
}   

  
ISR(USART_UDRE_vect){    
	 if(usart_bufor[usart_bufor_ind]!= 0){
		 UDR = usart_bufor[usart_bufor_ind++];		 
	 }else{
		 UCSRB &= ~(1<<UDRIE);
	 } 
}  
  
  
void usart_send(void){
 
	 unsigned char n;  
	 for(n=0; n<30; n++){  
		  if(usart_bufor[n]==0){   
		   usart_bufor[n]   = 13;   
		   usart_bufor[n+2]  = 0;
		   break;  
		  }  
	 }  
	 while (!(UCSRA & (1<<UDRE))); 
	 usart_bufor_ind = 0;    
	 UCSRB |= (1<<UDRIE);   
}  


void display(uint8_t val,uint8_t rev)	
{
	uint8_t tmp=0xFF;
	uint8_t tmp_rev=0xff;
		
	if((val)<14) tmp = pgm_read_byte(&numbers[val]);
	if((rev)<14) tmp_rev = pgm_read_byte(&numbers[rev]);
	PORTB = tmp;
	PORTA = tmp_rev;	
}


int main(void)
{
	char j=0;
	char blocked1=1, blocked2=1, blocked3=1;
	
	init_usart(); 
	sei();      
	init_ports();	
	temperature();

	while(1)	
	{							
		if(!(PIND & 0x08) && blocked1)
		{
			_delay_ms(80);
			blocked1 = 0;
			j=(j+1)%2;
		}
		
		if(PIND & 0X08 && !blocked1)
		{
			_delay_ms(80);
			blocked1=1;
		}
		switch(j){
			case 0:		
					
				temperature();
				display(current_temp/10, current_temp%10);
				sprintf(usart_bufor, "%s%d", "t", current_temp);
				usart_send();												
				set_temp=eeprom_read_byte(&eeprom_address);									
				if(current_temp<set_temp-1) 
					PORTD |= 0x40;					
				else if(current_temp>set_temp) 
					PORTD &= 0xBF;					 
				if(received_flag){
					received_flag=0;
					eeprom_update_byte(&eeprom_address, atoi(usart_bufor_received));	
					sprintf(usart_bufor, "%s%d", "Graniczna: ", eeprom_read_byte(&eeprom_address));
					usart_send();											
				}
				break;					
				case 1:				
					set_temp=eeprom_read_byte(&eeprom_address);					
					if(!(PIND & 0x10) && blocked3)
					{
						_delay_ms(100);
						blocked3 =0;
						set_temp--;
						display(set_temp/10, set_temp%10);
					}
					if(PIND & 0X10 && !blocked3)
					{
						_delay_ms(150);
						blocked3=1;
					}	
					if(!(PIND & 0x20) && blocked2)
					{
						_delay_ms(100);
						blocked2=0;
						set_temp++;
						display(set_temp/10, set_temp%10);
					}
					if(PIND & 0X20 && !blocked2)
					{
						_delay_ms(150);
						blocked2=1;
					}
					if(set_temp>99){
						set_temp = 0;
					}
					eeprom_update_byte(&eeprom_address, set_temp);
					display(set_temp/10, set_temp%10);
					break;
			}
	}
}