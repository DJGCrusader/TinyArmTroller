#define F_CPU 	32000000UL /*32 MHz Internal Oscillator*/

#include <avr/io.h>
#include <util/delay.h>
#include "requiredFiles/avr_compiler.h"
#include "requiredFiles/clksys_driver.h"
#include "requiredFiles/clksys_driver.c"
//#include <math.h>

void delay_ms(int ms);
void blink(int val);
void blinkFast(void);
void blinkSlow(void);
void setClock(void);
void init(void);
void mainLoop(void);
int main(void);

void delay_ms(int ms) 
{ 
    while (ms--) 
        _delay_us(1000); 
} 

void init(void){
		setClock();
		PORTD.DIRSET = _BV(4);  
        PORTD.DIRSET = _BV(5);
}

void blink(int myVal){
		PORTD.OUTSET = _BV(4);
        PORTD.OUTCLR = _BV(5);
        delay_ms(myVal);

        PORTD.OUTCLR = _BV(4);
        PORTD.OUTSET = _BV(5);
        delay_ms(myVal);
}

void blinkSlow(void){
	blink(2000);
}

void blinkFast(void){
	int i;
	for (i=0;i<10;i++){
		
		blink(100);
	}
	
}

void mainLoop(void){
		while(1){
			
			blinkSlow();
			blinkFast();
			blink(1000);
		}
}

void setClock(void){
    // Configure Clock -- Based on Application Notes & Examples on AVRFreaks
	CLKSYS_Enable( OSC_RC32MEN_bm );
//	do {} while ( CLKSYS_IsReady( OSC_RC32MRDY_bm ) == 0 );
	CLKSYS_Main_ClockSource_Select( CLK_SCLKSEL_RC32M_gc );
}



int main (void)
{	

		init();
        mainLoop();
        return 0;
}
