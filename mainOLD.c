/*
*
*
*
*/

#define F_CPU 32000000UL /*32 MHz Internal Oscillator*/

#include <avr/io.h>
#include <util/delay.h>
#include <math.h>

/* function for long delay */
void myDelay_ms(uint16_t ms) {
        while ( ms )
        {
                _delay_ms(1);
                ms--;
        }
}



int main (void)
{
        /* PC0 is digital output */
        //DDRD = _BV (PD4);    
        PORTD.DIRSET = _BV(4);  
        PORTD.DIRSET = _BV(5);     

        /* loop forever */
        while (1)
        {
                /* clear PD4 on PORTD (digital high) and delay for 1 Second */
                
                //PORTD &= ~_BV(PD4);
                PORTD.OUTSET = _BV(4);
                PORTD.OUTCLR = _BV(5);
                myDelay_ms(20);

                /* set PD4 on PORTD (digital low) and delay for 1 Second */
                //PORTD |= _BV(PD4);
                PORTD.OUTCLR = _BV(4);
                PORTD.OUTSET = _BV(5);
                myDelay_ms(20);
        }
}
