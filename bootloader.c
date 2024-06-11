#define F_CPU 16000000UL
#define BAUDRATE 9600

typedef enum {
    false,
    true,
} bool;

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/boot.h>
#include <util/delay.h>

#include "uart.h"
#include "parser.h"
#include <stdio.h>

#define WAIT_FOR_START 1
#define GET_DATA_LENGTH 2
#define GET_ADDRESS 3
#define GET_RECORD_TYPE 4
#define GET_DATA 5
#define GET_CHECKSUM 6
#define CHECKSUM_ERROR 7

#define DATA_RECORD 0
#define EOF_RECORD 1

// This functions resets everything to the default state and starts the program
void runProgram();

// This Timer runs 4sec in order to signal a the timeout
ISR(TIMER1_OVF_vect){
    runProgram();
}

/*
    INTEL HEX RECORD
    1 Byte - Starting Code is omitted
    1 Byte (2 Hex Characters but we receive ASCII from UART) - Record Length | Index[0]
    2 Bytes (4 Hex Characters) - Address | Index[1..3]
    2 Bytes (2 Hex Characters) - Record Type | Index[6..7]
    16 Bytes - Data | Index[8..23]
    2 Bytes (2 Hex Characters) - Checksum | Index[24..25]
    -> 26 Bytes without Starting Code
*/
uint8_t dataLength;
uint16_t pageAddress;
uint8_t recordType;
uint8_t data[16];
uint8_t dataIndex;
uint8_t checksum;

uint8_t bytesReceived = 0;
uint8_t hexBuffer[4] = {0};
uint8_t byteSum;
uint8_t state = WAIT_FOR_START;

int main(){
    
    cli();
    
    // Activate the Bootloader IV
    //uint8_t temp = MCUCR;
    //MCUCR = temp | (1 << IVCE);
    //MCUCR = temp | (1 << IVSEL);

    // Setup UART
    uart_init();

    sendString("<p> -> flashing mode | <any other key> -> continue to the application");
    sendCRLF();

    // Setup Timer1 to run (4 secs.) -> prescaler 1024
    TCCR1B |= ((1 << CS12) | (1 << CS10));
    // 65536 - 62500
    TCNT1 = 3036;
    TIMSK1 = (1 << TOIE1);

    sei();

    // Mode selection 
    uint8_t c;
    while((c = uart_receive()) == '\0') ;
    
    if(c != 'p'){
        runProgram();
    }

    // Disable Timer
    TIMSK1 = 0x0;

    // Receive program data from serial
    sendString("Please enter .hex code, new version");
    sendCRLF();
    
    // Wait for starting character
    while(1){
        while(!(c = uart_receive())) ;

        switch(state){
            case WAIT_FOR_START:
                if(c == ':'){
                    bytesReceived = 0;
                    byteSum = 0;
                    state = GET_DATA_LENGTH;
                }    
            break;
            case GET_DATA_LENGTH:
                hexBuffer[bytesReceived++] = c;
                if(bytesReceived == 2){
                    dataLength = hexDec8(hexBuffer);
                    char msg[100] = {0};
                    snprintf(msg, 100, "Length %u", dataLength);
                    sendString(msg);
                    sendCRLF();

                    // Add up Bytes for checksum
                    byteSum += dataLength;
                    bytesReceived = 0;
                    state = GET_ADDRESS;
                }
            break;
            case GET_ADDRESS:
                hexBuffer[bytesReceived++] = c;
                if(bytesReceived == 4){
                    pageAddress = hexDec16(hexBuffer);
                    sendCRLF();
                    byteSum += pageAddress;
                    
                    
                    // Calculate address
                    pageAddress = pageAddress % SPM_PAGESIZE;
                    char msg[100] = {0};
                    snprintf(msg, 100, "Page %d", pageAddress);
                    sendString(msg);
                    sendCRLF();
                    
                    bytesReceived = 0;
                    state = GET_RECORD_TYPE;
                }
            break;
            case GET_RECORD_TYPE:
                hexBuffer[bytesReceived++] = c;
                if(bytesReceived == 2){
                    recordType = hexDec8(hexBuffer);
                    char msg[100] = {0};
                    snprintf(msg, 100, "Type %u", recordType);
                    sendString(msg);
                    sendCRLF();
                    byteSum += recordType;
                    bytesReceived = 0;
                    if(!dataLength){
                        state = GET_CHECKSUM;
                    } else {
                        state = GET_DATA;
                    }                   
                }
            break;
            case GET_DATA:
                hexBuffer[bytesReceived++] = c;
                if(dataIndex == dataLength){
                    state = GET_CHECKSUM;
                    bytesReceived = 0;
                } else if (bytesReceived && bytesReceived % 2 == 0){
                    data[dataIndex] = hexDec8(hexBuffer);
                    char msg[100] = {0};
                    snprintf(msg, 100, "Index %u, Limit %u", dataIndex, dataLength);
                    sendString(msg);
                    sendCRLF();
                    byteSum += data[dataIndex];
                    dataIndex++;
                }
            break;
            case GET_CHECKSUM:
                hexBuffer[bytesReceived++] = c;
                if(bytesReceived == 2){
                    sendString("checksum");
                    checksum = hexDec8(hexBuffer);
                    
                    // Calculate checksum -> build 2th's complement and check for equality
                    byteSum = ~byteSum;
                    byteSum += 1;
                    char msg[70] = {0};
                    sendCRLF();
                    snprintf(msg, 70, "calculated %u, received %u", (uint16_t) byteSum, (uint16_t) checksum);
                    sendString(msg);

                    // 
                    //programFlash();
                    sendString("TODO: Program Flash");

                    
                    bytesReceived = 0;
                    state = WAIT_FOR_START;
                    sendCRLF();
                }
        }
    }
}

void runProgram(){
    // Move back to the normal IV
    //uint8_t temp = MCUCR;
    //MCUCR = temp | (1 << IVCE);
    //MCUCR = temp & ~(1 << IVSEL);
    sendString("Starting program...");
    
    _delay_ms(100);

    // Reset Timer
    TCCR1B = 0x0;
    TCNT1 = 0x0;
    TIMSK1 = 0x0;

    /*
    // Reset Uart
    uart_deinit();

    // Disable interrupts
    cli();

    // Jump into the program
    goto *(0x0);
    */
}

void boot_program_page(uint16_t page, uint8_t *buf){
    uint16_t i;
    uint8_t sreg;
    // Disable interrupts.
    sreg = SREG;
    cli();
    eeprom_busy_wait();
    boot_page_erase(page);
    boot_spm_busy_wait();      // Wait until the memory is erased.
    for(i = 0 ; i < SPM_PAGESIZE; i += 2){
        // Set up little-endian word.
        uint16_t w = *buf++;
        w += (*buf++) << 8;
    
        boot_page_fill (page + i, w);
    }
    boot_page_write(page);     // Store buffer in flash page.
    boot_spm_busy_wait();       // Wait until the memory is written.
    // Reenable RWW-section again. We need this if we want to jump back
    // to the application after bootloading.
    boot_rww_enable();
    // Re-enable interrupts (if they were ever enabled).
    SREG = sreg;
}