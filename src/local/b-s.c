#define F_CPU 16000000UL
#define BAUDRATE 9600

#include "u-s.h"
#include <stdio.h>
#include <stdint.h>

#define SPM_PAGESIZE 128

#define WAIT_FOR_START 1
#define GET_DATA_LENGTH 2
#define GET_ADDRESS 3
#define GET_RECORD_TYPE 4
#define GET_DATA 5
#define GET_CHECKSUM 6
#define CHECKSUM_ERROR 7

#define DATA_RECORD 0
#define EOF_RECORD 1

/* This functions resets everything to the default state and starts the program */
void runProgram();
/* Converts 4-Byte Hex String into uint16_t and 2-Byte Hex String into uint8_t */
uint16_t hexDec(uint8_t *bytes, uint8_t num);
/* Write page into program flash */
void boot_program_page(uint16_t page, uint8_t *buf);

/*
    INTEL HEX RECORD -> https://hexavik.github.io/posts/intel-hex-format/
    1 Byte - Starting Code is omitted
    1 Byte (2 Hex Characters but we receive ASCII from UART) - Record Length
    2 Bytes (4 Hex Characters) - Address
    2 Bytes (2 Hex Characters) - Record Type
    16 Bytes - Data 
    2 Bytes (2 Hex Characters) - Checksum
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

    // Setup UART
    uart_init();
    sendString("<p> -> flashing mode | <any other key> -> continue to the application");
    sendCRLF();

    uint8_t c;
    /*
    while((c = uart_receive()) == '\0') ;
    
    if(c != 'p'){
        runProgram();
    }
    */

    // Receive program data from serial
    sendString("Please enter .hex code");
    sendCRLF();
    
    // Wait for starting character
    while(1){
        while(!(c = uart_receive())) ;

        switch(state){
            case WAIT_FOR_START:
                if(c == ':'){
                    uart_send(':');
                    // Reset for next state
                    bytesReceived = 0;
                    byteSum = 0;
                    state = GET_DATA_LENGTH;
                }    
            break;
            case GET_DATA_LENGTH:
                hexBuffer[bytesReceived++] = c;
                uart_send(c);
                if(bytesReceived == 2){
                    // Decode data length
                    dataLength = (uint8_t) hexDec(hexBuffer, 2); 
                    char msg[100];
                    snprintf(msg, 100, "dataLength %u", (uint8_t) dataLength);
                    sendString(msg);
                    sendCRLF();

                    // Add up Bytes for checksum
                    byteSum += dataLength;

                    // Reset for next state
                    bytesReceived = 0;
                    state = GET_ADDRESS;
                }
            break;
            case GET_ADDRESS:
                hexBuffer[bytesReceived++] = c;
                uart_send(c);
                if(bytesReceived == 4){
                    // Decode absolute page address
                    pageAddress = hexDec(hexBuffer, 16);
                    char msg[100];
                    snprintf(msg, 100, "Address %hu", (uint16_t) pageAddress);
                    sendString(msg);
                    sendCRLF();
                    
                    // Update Checksum
                    byteSum += (uint8_t) pageAddress;
                    byteSum += (uint8_t) (pageAddress >> 8);
                    
                    // Calculate relative page address
                    pageAddress = pageAddress - (pageAddress % SPM_PAGESIZE);
                    snprintf(msg, 100, "Address %d", pageAddress);
                    sendString(msg);
                    sendCRLF();

                    // Reset for next state
                    bytesReceived = 0;
                    state = GET_RECORD_TYPE;
                }
            break;
            case GET_RECORD_TYPE:
                hexBuffer[bytesReceived++] = c;
                uart_send(c);
                if(bytesReceived == 2){
                    // Decode record type
                    recordType = (uint8_t) hexDec(hexBuffer, 2);

                    // Reset for next state
                    byteSum += recordType;
                    bytesReceived = 0;
                    dataIndex = 0;

                    // Skip GET_DATA state for EOF Records (Type 01)
                    state = (recordType == 1) ? GET_CHECKSUM : GET_DATA;               
                }
            break;
            case GET_DATA:
                switch (recordType){
                case DATA_RECORD:
                    hexBuffer[bytesReceived++] = c;
                    uart_send(c);
                    if(bytesReceived == 2){
                        // Collect data and add up checksum
                        data[dataIndex] = (uint8_t) hexDec(hexBuffer, 2);
                        byteSum += data[dataIndex];
                        dataIndex++;
                        bytesReceived = 0;
                        
                        // Check if finished
                        if(dataIndex == dataLength){
                            // Fill the rest of the buffer if needed
                            for(uint8_t i = dataLength; i < SPM_PAGESIZE; i++){
                                data[i] = 0xFF;
                            }

                            // Go to the next state
                            state = GET_CHECKSUM;
                        }
                    }
                    break;
                /* Maybe check out how to make eeprom write work */
                default:
                    break;
                }
            break;
            case GET_CHECKSUM:
                hexBuffer[bytesReceived++] = c;
                uart_send(c);
                if(bytesReceived == 2){
                    checksum = (uint8_t) hexDec(hexBuffer, 2);
                    
                    // Calculate checksum -> build 2th's complement and check for equality
                    byteSum = ~byteSum + 1;

                    //sendString("TODO: Program Flash");
                    /*
                        z.B.
                        send_XOFF();
                        _delay_ms(5);
                        boot_program_page(pageAddress, data)
                        send_XON();
                    */

                    /* Debug printf */
                    char msg[100];
                    snprintf(msg, 100, "Checksum %u", byteSum);
                    sendString(msg);
                    sendCRLF();
                    
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
    //endString("Starting program...");
    
  /*
    _delay_ms(100);

    // Reset Timer
    TCCR1B = 0x0;
    TCNT1 = 0x0;
    TIMSK1 = 0x0;
    */

    /*
    // Reset Uart
    uart_deinit();

    // Disable interrupts
    cli();

    // Jump into the program
    goto *(0x0);
    */
  printf("Program started\n");
  return;
}

void boot_program_page(uint16_t page, uint8_t *buf){
    for(int i = 0 ; i < SPM_PAGESIZE; i += 2){
        // Set up little-endian word.
        uint16_t w = *buf++;
        w += (*buf++) << 8;
    }
}

/* Converts 4-Byte Hex String into uint16_t and 2-Byte Hex String into uint8_t */
uint16_t hexDec(uint8_t *bytes, uint8_t num){
    uint16_t ret = 0;
    uint8_t c;
    for(uint8_t i = 0; i < num; i++){
        c = bytes[i];
        if(c >= '0' && c <= '9'){
            c -= '0';
        } else if (c >= 'A' && c <= 'F'){
            c -= ('A' - 10);
        } else if (c >= 'a' && c <= 'f'){
            c -= ('a' - 10);
        }
        ret = (ret << 4) | c;
    } 
    return ret;
}
