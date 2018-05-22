#ifndef DS18B20
#define DS18B20

#define PIN_1W 	7	
#define PORT_1W PINA		

#define OUT_LOW  ( PORT_1W &= (~(1 << PIN_1W)))
#define OUT_HIGH ( PORT_1W |= (1 << PIN_1W) )

#define DIR_IN()   (DDRA &= (~(1 << PIN_1W )) )
#define DIR_OUT()  (DDRA |= (1 << PIN_1W) )

#define GET_IN()   (PINA & (1<<PIN_1W))

extern char current_temp;
void temperature(void);
void convert_t(void);
uint8_t get_byte(void);
uint8_t send_byte(uint8_t b);
uint8_t send_bit(uint8_t b);
uint8_t reset_pulse(void);

#endif