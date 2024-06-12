#ifndef UART_H_
#define UART_H_
/*
	This file contains all the stuff we need for the serial communication
*/

#define BAUD_CONST ((F_CPU/(16UL*BAUDRATE))-1)
#define XOFF 0x13
#define XON	0x11

void uart_init();
void uart_deinit();
void uart_send(uint8_t data);
uint8_t uart_receive();
void send_xon();
void send_xoff();
void sendCRLF();
void sendString(char * string);

volatile uint8_t receiveBuffer[32] = {0};
volatile uint8_t lastReceived = 0;
volatile uint8_t lastRead = 0;
volatile uint8_t counter = 0;
volatile uint8_t sendBlock = 0;
volatile uint8_t x_status = XON; 

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
  clearScreen();
}

void uart_deinit() {
    clearScreen();
}

bool bufferEmpty() {
	return (!counter);
}

void uart_send(uint8_t data) {
    printf("%c", data);	
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
	x_status = XOFF;
	uart_send(XOFF);
}
void send_xon(){
	x_status = XON;
}

void sendCRLF(){
    uart_send(0x0D);
    uart_send(0x0A);
}

void sendString(char * string){
    uint8_t index = 0;

    while(string[index] != 0){
        uart_send(string[index]);
        index++;
    }   
}

#endif /* UART_H_ */
