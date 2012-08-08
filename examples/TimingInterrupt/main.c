/* Interrupt Example
 * by Daniel J. Gonzalez
 * July 2012, yameb.blogspot.com
 */

#define F_CPU 	32000000UL //32 MHz Internal Oscillator
#define LOW 0
#define HIGH 1

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "requiredFiles/avr_compiler.h"
#include "requiredFiles/clksys_driver.h"
#include "requiredFiles/clksys_driver.c"

#define bit_get(p,m) ((p) & (m)) 
#define bit_set(p,m) ((p) |= (m)) 
#define bit_clear(p,m) ((p) &= ~(m)) 
#define bit_flip(p,m) ((p) ^= (m)) 
#define BIT(x) (0x01 << (x))
#define LONGBIT(x) ((unsigned long)0x00000001 << (x))


volatile unsigned long elapsedMillis = 0;
volatile unsigned char flag = 0;


ISR(TCC0_OVF_vect){
	PORTE.OUTSET = _BV(0);
	elapsedMillis++;
}

// My own Delay function, because the built-in _delay_ms() can delay for only a limited time
void delay_ms(int ms) {
	_delay_us(1000);
} 

// Set Clock Speed to 32MHz (XMega)
void Config32MHzClock(void)
{ 
	CCP = CCP_IOREG_gc; //Security Signature to modify clock 
	// initialize clock source to be 32MHz internal oscillator (no PLL) 
	OSC.CTRL = OSC_RC32MEN_bm; // enable internal 32MHz oscillator 
	while(!(OSC.STATUS & OSC_RC32MRDY_bm)); // wait for oscillator ready 
	CCP = CCP_IOREG_gc; //Security Signature to modify clock 
	CLK.CTRL = 0x01; //select sysclock 32MHz osc 
}

//Get time elapsed in millis 
long millis(void){
	return elapsedMillis;
}

void blink(int myVal){
		PORTE.OUTSET = _BV(0);
        delay_ms(myVal);
        PORTE.OUTCLR = _BV(0);
        delay_ms(myVal);
}

int main (void)
{
	Config32MHzClock();// Set Clock Speed to 32MHz	
	PORTE.DIRSET = _BV(0); // DEBUG: Set pin E0 as OUTPUT
	PORTE.OUTCLR = _BV(0);

	/* Timer for keeping track of milliseconds: 
	 * 
	 * The result of this triggers the interrupt once per millisecond.
	 */
	TCC0.PER = 4000;// Set HIGH to 4000 and LOW to 0
	TCC0.CTRLA = ( TCC0.CTRLA & ~TC0_CLKSEL_gm ) | TC_CLKSEL_DIV1024_gc; // Work from CPUCLK/8
	TCC0.INTCTRLA = ( TCC0.INTCTRLA & ~TC0_OVFINTLVL_gm ) | TC_OVFINTLVL_HI_gc; // Enable overflow interrupt.--
	// Enable low interrupt level in PMIC and enable global interrupts.
	PMIC.CTRL |= PMIC_HILVLEN_bm;
	sei();
	
	
	while(1){
		if(millis()>1){
			PORTE.OUTSET = _BV(0);
		}
	}
}
