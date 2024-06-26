MCU = atmega328p
F_CPU = 16000000UL
CC = avr-gcc
OBJCOPY = avr-objcopy
CFLAGS = -std=c99 -Wall -g -Os -mmcu=$(MCU) -DF_CPU=$(F_CPU) -I.
PORT = usb
TARGET = ./dist/bootloader
SRCS = ./src/bootloader.c

all: 
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET).bin -Wl,-Ttext=0x7000
	$(OBJCOPY) -j .text -j .data -O ihex $(TARGET).bin $(TARGET).hex
	avr-size $(TARGET).hex
	avr-objdump -S $(TARGET).bin > $(TARGET).lss
flash:
	avrdude -p $(MCU) -c usbasp -P $(PORT) -U flash:w:$(TARGET).hex:i -V -F 

clean:
	rm -f ./dist/*.bin ./dist/*.hex