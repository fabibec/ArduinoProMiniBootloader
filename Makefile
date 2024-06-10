MCU = atmega328p
F_CPU = 16000000UL
CC = avr-gcc
OBJCOPY = avr-objcopy
CFLAGS = -std=c99 -Wall -g -Os -mmcu=$(MCU) -DF_CPU=$(F_CPU) -I.
PORT = usb
TARGET = bootloader
SRCS = bootloader.c

all: 
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET).bin 
	$(OBJCOPY) -j .text -j .data -O ihex $(TARGET).bin $(TARGET).hex

flash:
	avrdude -p $(MCU) -c usbasp -P $(PORT) -U flash:w:$(TARGET).hex:i -F -v

clean:
	rm -f *.bin *.hex