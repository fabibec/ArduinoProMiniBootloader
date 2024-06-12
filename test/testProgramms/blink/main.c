#include <avr/io.h>

#define F_CPU 16000000UL
#include <util/delay.h>

#define LED0_MODE DDB1
#define LED0_VAL PORTB1

int main(void){

	DDRB |= (1 << LED0_MODE);
	PORTB &= ~(1 << LED0_VAL);

	while(1){
		_delay_ms(1000);
		PORTB ^= (1 << LED0_VAL);
	}
}
