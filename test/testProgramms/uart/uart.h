#ifndef UART_H_
#define UART_H_

#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/common.h>

#define BAUD_CONST ((F_CPU/(16UL*BAUDRATE))-1)
#define XOFF 0x13
#define XON	0x11

void uart_send(uint8_t data);
uint8_t uart_receive();
void send_xon();
void send_xoff();

volatile uint8_t receiveBuffer[32] = {0};
volatile uint8_t lastReceived = 0;
volatile uint8_t lastRead = 0;
volatile uint8_t counter = 0;
volatile uint8_t sendBlock = 0;
volatile uint8_t x_status = XON; 

ISR(USART_RX_vect){
	uint8_t tmp = UDR0;
	
	// If XOFF is received block sending
	if (tmp == XOFF) {
		sendBlock = 1;
	} 
	// if XON is received allow sending
	else if(tmp == XON) {
		sendBlock = 0;
	}
	else {
		receiveBuffer[lastReceived] = tmp;
		lastReceived = ((lastReceived + 1) % 32);
		counter++;
		// If ring buffer has 26 elements stop receiving
		if(counter >= 22 && x_status==XON) {
			send_xoff();
		}
	}
	 
	
}

void clearScreen(){
	// Clear terminal
	uart_send(27); // ESC
	uart_send('[');
	uart_send('2');
	uart_send('J');

	// Move cursor to start
	uart_send(27);
	uart_send('[');
	uart_send('H');
}

void uart_init() {
	// Configure baud rate
	UBRR0H = (BAUD_CONST >> 8);
	UBRR0L = BAUD_CONST;
	
	// Enable uart send and receive and uart receive complete interrupt
	UCSR0B |= ((1<<RXEN0) | (1<<TXEN0) | (1<<RXCIE0));

    clearScreen();
}



bool bufferEmpty() {
	return (!counter);
}

void uart_send(uint8_t data) {
	
	// If send register empty and no XOFF
	while(!(UCSR0A & (1<<UDRE0)) && !sendBlock) {
		;
	}
	UDR0 = data;
}

uint8_t uart_receive(){
	if(bufferEmpty()) {
		return '\0';
	}
	
	uint8_t returnVal = receiveBuffer[lastRead];	
	counter--;
	lastRead = (lastRead + 1) % 32;
	
	// If ring buffer elements are less than or equal to 10 allow sending
	if (counter <= 10 && x_status == XOFF) {
		send_xon();
	}

	return returnVal;
}

void send_xoff(){
	uint8_t sreg = SREG;
	cli();
	x_status = XOFF;
	uart_send(XOFF);
	SREG = sreg;
	
}
void send_xon(){
	uint8_t sreg = SREG;
	cli();
	x_status = XON;
	uart_send(XON);
	SREG = sreg;
}

#endif /* UART_H_ */