TARGET = libavr-lcd1602

# Chip info.
MCU = atmega328p
MCU_PROGRAMMER = m328p
F_CPU = 16000000

# Data direction register and port used by the display.
LCD_DDR = DDRC
LCD_PORT = PORTC

CFLAGS = -std=c99 -DF_CPU=$(F_CPU)L -DLCD_DDR=$(LCD_DDR) -DLCD_PORT=$(LCD_PORT) -mmcu=$(MCU) -Wall -Os

compile:
	# Compile the source code for the library.
	avr-gcc $(CFLAGS) -c lcd1602.c

	# Create the static library.
	ar rcs $(TARGET).a lcd1602.o

asm:
	avr-gcc -S $(CFLAGS) -c lcd1602.c

clean:
	rm -f *.o *.a

all:
	compile
