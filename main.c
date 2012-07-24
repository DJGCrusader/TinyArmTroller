#define F_CPU 2000000UL /*32 MHz Internal Oscillator*/

#include <avr/io.h>
#include <util/delay.h>
//#include <math.h>

void doItFast(){
	int i;
	for (i=0;i<10;i++){
		PORTD.OUTSET = _BV(4);
        PORTD.OUTCLR = _BV(5);
        _delay_ms(100);

        PORTD.OUTCLR = _BV(4);
        PORTD.OUTSET = _BV(5);
        _delay_ms(100);
	}
}

void doItSlow(){
	int i;
	for (i=0;i<10;i++){
		PORTD.OUTSET = _BV(4);
        PORTD.OUTCLR = _BV(5);
        _delay_ms(1000);

        PORTD.OUTCLR = _BV(4);
        PORTD.OUTSET = _BV(5);
        _delay_ms(1000);
	}
}

int main (void)
{
		
        PORTD.DIRSET = _BV(4);  
        PORTD.DIRSET = _BV(5); 
//		doItFast();
		doItSlow();
		while(1){
		}
}
