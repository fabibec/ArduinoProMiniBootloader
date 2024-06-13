// Linkerflag für Bootloader Software
// -Ttext=0x7600

// IVSEL für Interupt Vektor (sofort beim starten, dann nach Ende wieder zurücksetzen)
uint8_t temp = MCUCR;
MCUCR = temp | (1 << IVCE);
MCUCR = temp | (1 << IVSEL);

// pages sind beim Atmega 328p nur 128 Bytes lang also nur unit16_t hier
#include <inttypes.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
void boot_program_page (uint32_t page, uint8_t *buf)
{
    uint16_t i;
    uint8_t sreg;
    // Disable interrupts.
    sreg = SREG;
    cli();
    eeprom_busy_wait ();
    boot_page_erase (page);
    boot_spm_busy_wait ();      // Wait until the memory is erased.
    for (i=0; i<SPM_PAGESIZE; i+=2)
    {
        // Set up little-endian word.
        uint16_t w = *buf++;
        w += (*buf++) << 8;
    
        boot_page_fill (page + i, w);
    }
    boot_page_write (page);     // Store buffer in flash page.
    boot_spm_busy_wait();       // Wait until the memory is written.
    // Reenable RWW-section again. We need this if we want to jump back
    // to the application after bootloading.
    boot_rww_enable ();
    // Re-enable interrupts (if they were ever enabled).
    SREG = sreg;
}