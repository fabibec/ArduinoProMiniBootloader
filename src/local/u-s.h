#ifndef UART_H_
#define UART_H_
/*
	This file contains all the stuff we need for the serial communication
*/

#define BAUD_CONST ((F_CPU/(16UL*BAUDRATE))-1)
#define XOFF 0x13
#define XON	0x11
#include <stdio.h>
#include <stdint.h>

void uart_init();
void uart_deinit();
void uart_send(uint8_t data);
uint8_t uart_receive();
void send_xon();
void send_xoff();
void sendCRLF();
void sendString(char * string);
void clearScreen();

volatile uint8_t counter = 0;
volatile uint8_t sendBlock = 0;
volatile uint8_t x_status = XON; 
volatile uint8_t endOfFile = 0;

FILE *out;
FILE *in;

void clearScreen(){
    /*
	// Clear terminal
	uart_send(27); // ESC
	uart_send('[');
	uart_send('2');
	uart_send('J');

	// Move cursor to start
	uart_send(27);
	uart_send('[');
	uart_send('H');
    */
    // send newline for document
    uart_send('\n');
}

void uart_init() {
    out = fopen("out.txt", "w");
    in = fopen("in.hex", "r");
    clearScreen();
}

void uart_deinit() {
    fclose(out);
    clearScreen();
}

uint8_t bufferEmpty() {
	return (!counter);
}

void uart_send(uint8_t data) {
  fputc(data, out);
}

uint8_t uart_receive(){
    uint8_t returnVal = fgetc(in);    
    if (returnVal == EOF) {
        endOfFile = 1;
    }
    if (endOfFile) {
        return '\0';
    }
	return returnVal;
}

void send_xoff(){
	x_status = XOFF;
}
void send_xon(){
	x_status = XON;
}

void sendCRLF(){
  // send newline for document 
  uart_send('\n');
}

void sendString(char * string){
    uint8_t index = 0;

    while(string[index] != 0){
        uart_send(string[index]);
        index++;
    }   
}

#endif /* UART_H_ */
