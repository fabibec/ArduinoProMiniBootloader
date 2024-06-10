#define F_CPU 16000000UL
#define BAUDRATE 9600

typedef enum {
    false = 0,
    true = 1
} bool;

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/boot.h>
#include <util/delay.h>

#include "uart.h"



// This functions resets everything to the default state and starts the program
void runProgram();

// This Timer runs 4sec in order to signal a the timeout
ISR(TIMER1_OVF_vect){
    runProgram();
}

int main(){
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

    // Reset Uart
    uart_deinit();

    // Disable interrupts
    cli();

    // Jump into the program
    goto *0;
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