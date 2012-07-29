/* TinyArmTroller Firmware v0.02
 * by Daniel J. Gonzalez and Azureviolin, modified from Arduino Stepper.cpp V.04
 * July 2012, yameb.blogspot.com
 * 
 */

#define F_CPU 	32000000UL //32 MHz Internal Oscillator
#define LOW 0
#define HIGH 1

#define bit_get(p,m) ((p) & (m)) 
#define bit_set(p,m) ((p) |= (m)) 
#define bit_clear(p,m) ((p) &= ~(m)) 
#define bit_flip(p,m) ((p) ^= (m)) 
#define bit_write(c<Plug>PeepOpen,m) (c ? bit_set(p,m) : bit_clear(p,m)) 
#define BIT(x) (0x01 << (x)) 
#define LONGBIT(x) ((unsigned long)0x00000001 << (x))

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

void mainLoop(void){
	while(1){
		
	}
}

void init(void){
		Config32MHzClock(); //Set Clock Speed to 32MHz
}

int main (void)
{
	init();	
	delay_ms(1000);
	mainLoop();
	return 0;
}
