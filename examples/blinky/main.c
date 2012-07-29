/* Blinky: an atxmega16a4 example
 * Daniel J. Gonzalez July 2012
 * dgonz@mit.edu
 * yameb.blogspot.com
 * 
 * MCU: atxmega16a4u
 * Board: TinyArmTroller REV2
 * Setup: Button between C3 and GND, Resistor and LED between E0 and GND
 * 
 * The purpose of this program has been to learn how to do the following:
 * -Set the CPU Speed to 32MHz
 * -Implement robust delay
 * -Implement outputs and set outputs to HIGH and LOW
 * -Implement inputs and detect of an input pin is driven LOW
 * 
 * Specifically, this program blinks an LED on port E0 in a certain pattern 
 * if a button between port C3 and GND is pressed. 
 */

#define F_CPU 	32000000UL //32 MHz Internal Oscillator

#include <avr/io.h>
#include <util/delay.h>
#include "requiredFiles/avr_compiler.h"
#include "requiredFiles/clksys_driver.h"
#include "requiredFiles/clksys_driver.c"


void delay_ms(int ms) //My own Delay function, because the built-in _delay_ms() can delay for only a limited time
{ 
    while (ms--) 
        _delay_us(1000);  
} 

void Config32MHzClock(void) //Set Clock Speed to 32MHz (XMega Only)
{ 
	CCP = CCP_IOREG_gc; //Security Signature to modify clock 
	// initialize clock source to be 32MHz internal oscillator (no PLL) 
	OSC.CTRL = OSC_RC32MEN_bm; // enable internal 32MHz oscillator 
	while(!(OSC.STATUS & OSC_RC32MRDY_bm)); // wait for oscillator ready 
	CCP = CCP_IOREG_gc; //Security Signature to modify clock 
	CLK.CTRL = 0x01; //select sysclock 32MHz osc 
}

void blink(int myVal){
		PORTE.OUTSET = _BV(0);
        delay_ms(myVal);
        PORTE.OUTCLR = _BV(0);
        delay_ms(myVal);
}

void blinkQuick(void){
	blink(500);
}

void blinkTen(void){
	int i;
	for (i=0;i<10;i++){
		blink(50);
	}
}

void mainLoop(void){
		while(1){
			if(!(PORTC.IN & _BV(3))){ //if pin C3 goes LOW
				blinkQuick();
				blinkTen();
			}
		}
}

/* See Atmel doc8050.pdf : AVR1313: Using the XMEGA IO Pins and External Interrupts
 * 
 * 
 */
void init(void){
		Config32MHzClock(); //Set Clock Speed to 32MHz
		PORTE.DIRSET |= _BV(0); //Set pin E0 as OUTPUT
		PORTC.PIN3CTRL = PORT_OPC_PULLUP_gc; //Set pin C3 as INPUT and as a pullup (Detect when pulled LOW)
}

int main (void)
{
	init();	
	delay_ms(1000);
	mainLoop();
	return 0;
}
