#define F_CPU 2000000UL /*2 MHz Internal Oscillator*/

#include <avr/io.h>
#include <util/delay.h>
//#include <math.h>
void doItFast(void);
void doItSlow(void);
int main(void);
void mainLoop(void);

void doItFast(void){
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

void doItSlow(void){
		PORTD.OUTSET = _BV(4);
        PORTD.OUTCLR = _BV(5);
        _delay_ms(1000);

        PORTD.OUTCLR = _BV(4);
        PORTD.OUTSET = _BV(5);
        _delay_ms(1000);
}

void mainLoop(void){
		while(1){
			doItFast();
			doItSlow();
		}
}



int main (void)
{
        PORTD.DIRSET = _BV(4);  
        PORTD.DIRSET = _BV(5); 
        mainLoop();
        return 0;
}
