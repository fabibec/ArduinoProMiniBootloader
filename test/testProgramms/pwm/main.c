#define F_CPU 16000000UL
#define BAUDRATE 9600

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "adc.h"
#include "uart-utils.h"
#include "uart.h"

#define POTIPIN A2

typedef enum {
	FALLING,
	RISING,
} edge;

const char menuLine1[] PROGMEM = "Duty Cycle: ";
volatile edge currentEdge = RISING;
volatile uint16_t fallingCapture = 0;
volatile uint16_t risingCapture = 0;
	
ISR(TIMER1_CAPT_vect){
	uint16_t capture = ICR1;
	if(currentEdge == RISING) {
		// React to Falling edge
		TCCR1B &= ~(1 << ICES1);
		// Clear capture flag
		TIFR1 &= ~(1 << ICF1);
		risingCapture = capture;
		currentEdge = FALLING;
	} else {
		// React to Rising edge
		TCCR1B |= (1 << ICES1);
		// Clear capture flag
		TIFR1 &= ~(1 << ICF1);
		fallingCapture = capture;
		currentEdge = RISING;
	}
}

inline void writeDutyCycle(uint8_t dutyCycle){
	char msgVal[20] = {0};
	sendPGMString(menuLine1);
	snprintf(msgVal, 20, "%d %%", dutyCycle);
	sendString(msgVal);
}

inline uint8_t calculateDutyCycle(){
	// Detect Timer overflow
	if (fallingCapture < risingCapture) {
		uint32_t tmp;
		tmp = 65536 - (risingCapture - fallingCapture);
		tmp *= 100;
		tmp /= 250;
		tmp = 100 - tmp;
		tmp += 2;
		return tmp;
	}
	// Standard calculation
	else {
		// 100 - because we use inverted PWM mode
		uint32_t tmp;
		tmp = fallingCapture - risingCapture;
		tmp *= 100;
		tmp /= 250;
		tmp = 100 - tmp;
		return tmp;
	}
}



int main(){

	// Store the ADC value
	uint16_t adcVal = 0;

	uart_init();
	adcInit();

	// Init PWM Timer
	TCNT0 = 0;
	
	// Set OCR0A | Inverted mode because LED is wired to 5V
	OCR0A = 250;
	OCR0B = 125;
	TCCR0A |= ((1 << COM0B1) | (1 << COM0B0));
	
	// OCR0B is on PD5
	DDRD |= (1 << PD5);
	
	// Fast PWM (WGM2:0 = 111) 1 kHz | 64 Prescaler (CS2:0 = 011)
	TCCR0A |= ((1 << WGM01) | (1 << WGM00));
	TCCR0B |= ((1 << WGM02) | (1 << CS01) | (1 << CS00));
	
	// Init PWM Input Capture Timer | 64 Prescaler 
	TCCR1B |= ((1 << ICNC1) | (1 << CS11) | (1 << CS10));
	TIMSK1 |= (1 << ICIE1);
	

	sei();
	while (1){                                                                                                                              ^
		adcRead(POTIPIN, &adcVal);

		// Convert ADC to mV
		// 250 / (2^10) = ~ 1/4
		adcVal /= 4;
		OCR0B = (uint8_t) adcVal;
		
		
		clearScreen();
		// Disable interrupts to make sure that fallingCapture/risingCapture does not change durring calculation
		cli();
		uint16_t dutyCycle = calculateDutyCycle();
		sei();
		writeDutyCycle(dutyCycle);
		
		_delay_ms(500);
	}
}

