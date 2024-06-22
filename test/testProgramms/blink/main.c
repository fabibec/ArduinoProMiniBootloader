#include <avr/io.h>
#include <avr/common.h>
#include <avr/interrupt.h>

#define F_CPU 16000000UL
#include <util/delay.h>

#define LED0_MODE DDB1
#define LED0_VAL PORTB1
#define LED1_MODE DDB0
#define LED1_VAL PORTB0
#define LED2_MODE DDD7
#define LED2_VAL PORTD7

int main(void){
	uint8_t in = SREG & (1 << SREG_I);
	uint8_t ivec = MCUCR & (1 << IVSEL);

	DDRB |= ((1 << LED0_MODE) | (1 << LED1_MODE));
	DDRD |= (1 << LED2_MODE);
	PORTB &= ~(1 << LED0_VAL);

	if (!in){
		PORTD |= (1 << LED2_VAL);
	}
	
	if (!ivec){
		PORTB |= (1 << LED1_VAL);
	}

	while(1){
		_delay_ms(1000);
		PORTB ^= (1 << LED0_VAL);
	}
}
