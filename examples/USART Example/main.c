/* UART: an atxmega16a4u example
 * Daniel J. Gonzalez December 2012
 * dgonz@mit.edu
 * yameb.blogspot.com
 * 
 * MCU: atxmega16a4u
 * Board: TinyArmTroller REV2
 * Setup: Button between C3 and GND, Resistor and LED between E0 and GND
 */

#define F_CPU 	32000000UL //32 MHz Internal Oscillator

#include <stdio.h> 
#include <avr/io.h>
#include <util/delay.h>
#include "requiredFiles/avr_compiler.h"
#include "requiredFiles/clksys_driver.h"
#include "requiredFiles/clksys_driver.c"

static int uart_putchar(char c, FILE *stream); 
static void uart_init (void); 

static FILE mystdout = FDEV_SETUP_STREAM (uart_putchar, NULL, _FDEV_SETUP_WRITE);
//static FILE mystdin = FDEV_SETUP_STREAM (uart_putchar, NULL, _FDEV_SETUP_READ);

int vals[6] = {0,0,0,0,0,0};

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

void blinkN(int n){
    int i;
	for (i=0;i<n;i++){
		blink(50);
	}
}

void update(void){
    int c;
    int i;
    for(i = 0; i < 6; i++){
        vals[i] = 0; 
        while(1){
            c = uart_getchar();
            if(c == ','){
                break;
            }else if(c == '\n' && i == 5){
                break;
            }else{
                vals[i] = vals[i]*10+(c - '0');
            }
        }
    }
    return 0;
}

void mainLoop(void){
    stdout = &mystdout; 
    blink(1000);
    while (1){
        printf("Hello, world!\n");
        update();
        int i;
        for(i = 0; i < 6; i++){
            blinkN(vals[i]);
            delay_ms(1000);
        }
    }
}

/* See Atmel doc8050.pdf : AVR1313: Using the XMEGA IO Pins and External Interrupts
 * 
 * 
 */
void init(void){
		Config32MHzClock(); //Set Clock Speed to 32MHz	
		PORTD.PIN4CTRL = PORT_OPC_PULLUP_gc; //Set pin D4 as INPUT and as a pullup (Detect when pulled LOW)
		PORTE.DIRSET |= _BV(0); //Set pin E0 as OUTPUT
        uart_init(); 
}

int main (void)
{
	init();	
	delay_ms(1000);
	mainLoop();
	return 0;
}


static int uart_putchar (char c, FILE *stream) { 
    if (c == '\n') 
        uart_putchar('\r', stream); 

    // Wait for the transmit buffer to be empty 
    while ( !( USARTD1.STATUS & USART_DREIF_bm) ); 

    // Put our character into the transmit buffer 
    USARTD1.DATA = c; 

    return 0; 
} 


int uart_get(void){
    int output;
    while(1){
        if(uart_getchar() == '\n')
            break;
        else
            output = uart_getchar();
    }
    return output;
}

int uart_getchar (void) { 
    while( !(USARTD1_STATUS & USART_RXCIF_bm) ){
        PORTE.OUTSET = _BV(0);
    }
    PORTE.OUTCLR = _BV(0);
    return USARTD1.DATA;
} 

// Init USART.  Transmit only (we're not receiving anything) 
// We use USARTD1, transmit pin on PC7. 
// Want 115200 baud. Have a 32 MHz clock. BSCALE = 0 
// BSEL = ( 32000000 / (2^0 * 16*115200)) -1 = 16.361111111
// Fbaud = 32000000 / (2^0 * 16 * (16+1))  = 117647 bits/sec 
static void uart_init (void) { 
    // Set the TxD pin high - set PORTD DIR register bit 7 to 1 
    PORTD.OUTSET = PIN7_bm; 

    // Set the TxD pin as an output - set PORTC OUT register bit 3 to 1 
    PORTD.DIRSET = PIN7_bm; 

    // Set baud rate & frame format 
    
    USARTD1.BAUDCTRLB = 0;                      // BSCALE = 0 as well 
    USARTD1.BAUDCTRLA = 51; //16 for 32MHz at 115200, 12 for 2MHz at 9600, 51 for  38400 at 32MHz

    // Set mode of operation 
    USARTD1.CTRLA = 0;                          // no interrupts please 
    USARTD1.CTRLC = 0x03;                       // async, no parity, 8 bit data, 1 stop bit 

    // Enable transmitter only 
    USARTD1.CTRLB =USART_RXEN_bm|USART_TXEN_bm; 
     
} 
