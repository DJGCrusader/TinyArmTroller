# AVR-GCC Makefile
PROJECT=tutorial_1
SOURCES=main.c
CC=avr-gcc
OBJCOPY=avr-objcopy
#This is what the atxmega16a4 uses according to http://www.nongnu.org/avr-libc/user-manual/using_tools.html
MMCU=atxmega32a4

CFLAGS=-mmcu=$(MMCU) -Wall -O1

$(PROJECT).hex: $(PROJECT).out
	$(OBJCOPY) -j .text -O ihex $(PROJECT).out $(PROJECT).hex

$(PROJECT).out: $(SOURCES)
	$(CC) $(CFLAGS) -I./ -o $(PROJECT).out $(SOURCES)

program: $(PROJECT).hex
	avrdude -p x16a4 -c avrispmkII -e -U flash:w:$(PROJECT).hex
clean:
	rm -f $(PROJECT).out
	rm -f $(PROJECT).hex
