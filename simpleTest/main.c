#include <avr/io.h>

#define F_CPU 16000000UL
#include <util/delay.h>

#define LED0_MODE DDD0
#define LED0_VAL PORTD0

int main(void){
	DDRD |= (1 << LED0_MODE);
	PORTD &= ~(1 << LED0_VAL);
	while(1){
	  _delay_ms(1000);
      PORTD ^= (1 << LED0_VAL);
	}
}
