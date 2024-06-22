#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 16000000UL
#include <util/delay.h>

#define LED0_MODE DDB1
#define LED0_VAL PORTB1

/* This Timer runs 4sec in order to signal a the timeout*/
ISR(TIMER1_OVF_vect){
    PORTB ^= (1 << LED0_VAL);
    TCNT1 = 60000;
}

int main(void){
    DDRD |= (1 << DDD7);
	DDRB |= ((1 << LED0_MODE) | (1 << DDB0));
	PORTB &= ~(1 << LED0_VAL);

    // Setup Timer1 to run (1 secs.) -> prescaler 1024
    TCCR1B |= ((1 << CS12) | (1 << CS10));
    TCNT1 = 60000;
    TIMSK1 = (1 << TOIE1);

    sei();
	while(1){
	}
}