#ifndef _UARTUTILS_H_
#define _UARTUTILS_H_

#include "uart.h"
#include <avr/pgmspace.h>

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

#endif