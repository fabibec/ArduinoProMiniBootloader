#define F_CPU 16000000UL
#define BAUDRATE 9600

#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/io.h>
#include <stdbool.h>
#include <stdio.h>

#include "timer.h"
#include "uart.h"

/*
	LEDA1 AN PB0
	LEDA4 AN PB1
	LEDA7 AN PB2
*/

// LED Ports
#define LED0_MODE DDB0
#define LED0_VAL PORTB0
#define LED1_MODE DDB1
#define LED1_VAL PORTB1
#define LED2_MODE DDB1
#define LED2_VAL PORTB1

// Prototypes
inline void printMenu();
inline void sendCRLF();
inline void sendPGMString(const char *);
inline void getCommand();
void setLEDS();
void countdown();
bool isValidStartValue(uint8_t);

// Menu Text in FLASH
const char menuTextHeader[] PROGMEM = "Fabi\'s & Flo\'s Fancy Timer:\0";
const char menuTextDivider[] PROGMEM = "-----------------";
const char menuTextAction[]PROGMEM = "Choose an action (1-4)";
const char menuTextAction1[] PROGMEM = "1 - Start Timer";
const char menuTextAction2[] PROGMEM = "2 - Stop Timer";
const char menuTextAction3[] PROGMEM = "3 - Configure Starting Time";
const char menuTextAction4[] PROGMEM = "4 - Print Starting Time";
const char menuTextInput[] PROGMEM = "Input (1-4): ";
const char menuTextReturn[] PROGMEM = "Press RETURN to go back to Main Menu";
const char menuTextConfigStartVal[] PROGMEM = "Starting Value (1-7): ";
const char menuTextError[] PROGMEM = "Error! Please choose action between 1 and 4.";
const char menuTextWrongInput[] PROGMEM = "Invalid Input, please enter a number between 1 and 7";
const char menuTextRunningInfo[] PROGMEM = "The Timer is currently running, please stop the Timer to set starting value!";

const char* const menu[] PROGMEM = {menuTextHeader, menuTextDivider, menuTextAction, \
    menuTextAction1, menuTextAction2, menuTextAction3, menuTextAction4};
const uint8_t menuSize PROGMEM = 7;


// If the counter is running or not
volatile uint8_t running = 1;
// The current count
volatile uint8_t count;
// The starting count (reset value)
uint8_t startingCount EEMEM = 7;

char msg[30] = {0};


int main(void){
	// Activate Output and Input Pullups and init
	DDRB |= ((1 << LED0_MODE) | (1 << LED1_MODE) | (1 << LED2_MODE));
	
	timer_init();
    uart_init();

	// Declare 1s timer for counting down 
	declareTimer(TIMER1, 1000000, countdown);

	// Enable global interrupt
	sei();

    // Set LED to starting value
    count = eeprom_read_byte(&startingCount);
    setLEDS();

	while(1){
        printMenu();
        getCommand();
	}
}

void countdown(){
	if(running) {
		if(count) {
			count--;
		} else {
			count = startingCount;
		}
        setLEDS();
	}
}

inline void printMenu(){
    uint8_t end = pgm_read_byte(&menuSize);
    uint16_t adr;

    for(uint8_t i = 0; i < end; i++){
        adr = pgm_read_word(&menu[i]);
        sendPGMString((char *) adr);
        sendCRLF();
    }
}

inline void sendCRLF(){
    uart_send(0x0D);
    uart_send(0x0A);
}

inline void sendPGMString(const char * string){
    uint8_t index = 0, c;

    while((c = pgm_read_byte(string + index)) != 0){
        uart_send(c);
        index++;
    }   
}

inline void sendString(char * string){
    uint8_t index = 0;

    while(string[index] != 0){
        uart_send(string[index]);
        index++;
    }   
}

inline void getCommand(){
    sendPGMString(menuTextInput);
    uint8_t c;

    while((c = uart_receive()) == '\0') ;
    clearScreen();
    sendPGMString(menuTextInput);
    uart_send(c);
    sendCRLF();

    c -= '0';

    switch(c){
        case 1:
            // Start Timer
            count = eeprom_read_byte(&startingCount);
            setLEDS();
            startTimer(TIMER1);
            break;
        case 2:
            // Stop Timer
            cancelTimer(TIMER1);
            break;
        case 3:
            // Configure starting time
            sendPGMString(menuTextConfigStartVal);
            int8_t c;

            do{
                while((c = uart_receive()) == '\0') ;
                uart_send(c);
            } while(!isValidStartValue(c - '0'));

            // Set new count value
            if(isRunning(TIMER1)){
                sendCRLF();
                sendPGMString(menuTextRunningInfo);
            } else {
                uint16_t newStartingCount = c - '0';
                eeprom_write_byte(&startingCount, newStartingCount);
                count = eeprom_read_byte(&startingCount);
                setLEDS();
            }
            sendCRLF();
            break;
        case 4:
            // Return starting time
            snprintf(msg, 30, "Starting Value is set to %c", (char) (eeprom_read_byte(&startingCount) + '0'));
            sendString(msg);
            sendCRLF();
            break;
        default:
            sendCRLF();
            sendPGMString(menuTextError);
            sendCRLF();
            break;
    }

    sendPGMString(menuTextReturn);
    while((c = uart_receive()) != 13) ;
    clearScreen();
}

bool isValidStartValue(uint8_t c){
    if(c > 7 || c == 0){
        sendCRLF();
        sendPGMString(menuTextWrongInput);
        sendCRLF();
        sendPGMString(menuTextConfigStartVal);
        return false;
    }
    return true;
}

void setLEDS() {
	uint8_t tmp = PORTB;
	// 11111000 -> Clear last 3 bits
	tmp &= 0xF8;
	// 00000XXX -> Set count on tmp without modifying first 5 bits
	tmp |= (~count & 0x07);
	PORTB = tmp;
}