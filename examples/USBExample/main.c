/*
*Basic USB Example
*by Daniel J.  Gonzalez July 2012
*dgonz@mit.edu | http://yameb.blogspot.com
* 
* Useful:
* USB interrupts on pg30 of ATxMegaa4u datasheet
* 
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "main.h"

// --- main frame buffers
#ifndef __IMAGE_H
	UCHAR frame[FRAME_SIZE];
#endif
UCHAR unpacked_gs_packet[GS_PACKET_SIZE];

// --- functions that are almost-inline, except even faster when substituted
#define pulse_xlat() 			sbi(PORTD, XLAT); cbi(PORTD, XLAT);
#define blank() 				sbi(PORTD, BLANK);
#define unblank() 				cbi(PORTD, BLANK);
#define pulse_blank() 			sbi(PORTD, BLANK); cbi(PORTD, BLANK);
#define clock_sr() 				sbi(PORTC, SR_CLOCK); _delay_loop_1(10); cbi(PORTC, SR_CLOCK); cbi(PORTC, SR_DATA);
#define status_toggle1() 		PORTD ^= _BV(PD6);
#define status_toggle2() 		PORTD ^= _BV(PD7);
#define usb_assert_rd()			cbi(PORTC, PC7);
#define usb_disassert_rd()		sbi(PORTC, PC7);
#define usb_assert_wr()			sbi(PORTC, PC6);
#define usb_disassert_wr()		cbi(PORTC, PC6);

#ifdef USART_IN_DEBUG_MODE
void uart_send(char c) {
	while (!(UCSR0A & _BV(UDRE0))) { }
	UDR0 = c;
}
#endif

ISR(__vector_default) {
	status_toggle2();
}


void init() {
	UCHAR i;

	////////////////////////////////////////////////////////////
	// DDR INIT
	////////////////////////////////////////////////////////////
	
	// --- USB: wr/rd --> output
	DDRA = 0x00;
	DDRC |= _BV(PC6) | _BV(PC7);
	usb_disassert_rd();
	usb_disassert_wr();
	
	// --- FET drive
	// data and clock --> out
	DDRC |= _BV(PC0) | _BV(PC1);
	
	// --- LED drive
	// all outputs on
	DDRB |= _BV(VPRG);
	DDRD |= _BV(XLAT) | _BV(DCPRG) | _BV(GSCLK) | _BV(BLANK);
	blank();           // disable outputs
	sbi(PORTD, DCPRG); // DC from regs
	cbi(PORTB, VPRG);  // access GS regs
	
	// SPI 1+2 output mode
	DDRB |= _BV(PB0) | _BV(PB5) | _BV(PB7);
	DDRD |= _BV(PD1);
	
	// --- status LEDs
	DDRD |= _BV(PD6) | _BV(PD7);
	
	////////////////////////////////////////////////////////////
	// SPI INIT
	////////////////////////////////////////////////////////////
	
	// --- SPI unit: interrupt, master; clk/2
	SPCR0 = _BV(SPE0) | _BV(MSTR0);
	SPSR0 = _BV(SPI2X0);

	#ifdef USART_IN_DEBUG_MODE	
	// --- USART unit (serial port): tx, rx, rate = 56.7, 8n1
	UBRR0 = 21;
	UCSR0B = _BV(TXEN0) | _BV(RXEN0);
	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);
	#else
	// --- USART unit: transmit complete intr, tranmit mode; MSPIM mode; clk/4
	UCSR0B = _BV(TXEN0) | _BV(RXEN0);
	UCSR0C = _BV(UMSEL00) | _BV(UMSEL01);
	UBRR0 = 0x01;
	#endif
	
	////////////////////////////////////////////////////////////
	// ENABLE EXT INTERRUPT SENSE (BUT NOT MASK)
	////////////////////////////////////////////////////////////
	EICRA = _BV(ISC21);
	EIMSK = _BV(INT2);
	
	////////////////////////////////////////////////////////////
	// DISABLE FETS
	////////////////////////////////////////////////////////////
	cbi(PORTC, SR_DATA);
	for (i = 0; i < 33; i++) {
		sbi(PORTC, SR_CLOCK);
		cbi(PORTC, SR_CLOCK);
	}

	////////////////////////////////////////////////////////////
	// ENABLE GS CLOCK GEN ON OC1B @ 10 MHz
	////////////////////////////////////////////////////////////
	OCR1A = 0x00; // first tick, swich on
	ICR1 = 0x01; // every 2 ticks, switch off
	DDRD |= _BV(PD4); // enable pin
	// --- timer/counter 1: turn on @ compare match, clear @ overflow, fast PWM mode
	// --- ICR1 = top, OCR1a = match
	// --- clock = f_clk
	TCCR1A = _BV(COM1B0) | _BV(COM1B1) | _BV(WGM11);
	TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS10);

	////////////////////////////////////////////////////////////
	// DISABLE INTERRUPTS
	////////////////////////////////////////////////////////////
	cli();
}

void primary_scan_loop2() {
	UCHAR i, y;
	UCHAR a, b;
	UCHAR primary;
	UCHAR secondary;
	UINT f;
	UCHAR update_loss_counter = 0;
	UINT frame_update_position = 0xff;

	while (1) {
		sbi(PORTC, SR_DATA);
		f = 0;
		
		if (frame_update_position && update_loss_counter++ == UPDATE_LOSS_LIMIT) {
			update_loss_counter = 0;
			frame_update_position = 0;
			make_checkerboard(3);
		}
		
		status_toggle1();
		
		for (y = MAX_ROWS; y; y--) {
			// unpack next data
			// we handle 2 data at a time, converting to 3 bytes
			// we also try to read in 2 bytes at that point
			primary = 0;
			for (i = MAX_COLS/2; i; i--) {
				
				// check for data and read
				if (!(PINB & _BV(PB2))) {
					usb_assert_rd();
					frame[frame_update_position++] = PINA;
					usb_disassert_rd();
					if (frame_update_position == FRAME_SIZE) {
						frame_update_position = 0;
						update_loss_counter = 0;
						status_toggle2();
					} else {
						_delay_loop_1(3); // 11 tick comp
					}
				} else {
					_delay_loop_1(7); // 23 tick comp
				}
				
				// parse a frame data while we wait for the USB to catch up
				unpacked_gs_packet[primary++] = frame[f++];
				
				// check for data and read
				if (!(PINB & _BV(PB2))) {
					usb_assert_rd();
					frame[frame_update_position++] = PINA;
					usb_disassert_rd();
					if (frame_update_position == FRAME_SIZE) {
						frame_update_position = 0;
						update_loss_counter = 0;
						status_toggle2();
					} else {
						_delay_loop_1(3);
					}
				} else {
					_delay_loop_1(7);
				}
				
				// parse another frame data while waiting for USB to catch up
				b = frame[f++];
				unpacked_gs_packet[primary++] = (UCHAR)(b >> 4);
				unpacked_gs_packet[primary++] = (UCHAR)(b << 4);
			}
			
			// setup primary and secondary packet pointers
			primary = CH1_START;
			secondary = CH2_START;
			
			// prime SPI units with first data values
			SPDR0 = unpacked_gs_packet[primary++];
            #ifndef USART_IN_DEBUG_MODE
				UDR0 = unpacked_gs_packet[secondary++];
			#endif
			
			// clock out rest of data (one byte spent already)
			a = CH1_SIZE - 1;
			#ifdef USART_IN_DEBUG_MODE
				b = 0;
			#else
				b = CH2_SIZE - 1;
			#endif
			
			while (a+b) {
				if (a && (SPSR0 & _BV(SPIF0))) {
					SPDR0 = unpacked_gs_packet[primary++];
					a--;
				}
				if (b && (UCSR0A & _BV(TXC0))) {
					UCSR0A |= _BV(TXC0);
					UDR0 = unpacked_gs_packet[secondary++];
					b--;
				}
			}
			
			// clear the flags (requires reading for SPI one)
			while (!(SPSR0 & _BV(SPIF0))) ;
			#ifndef USART_IN_DEBUG_MODE
				while (!(UCSR0A & _BV(TXC0))) ;
				UCSR0A |= _BV(TXC0);
			#endif
			
			// blank display for changeover
			blank();
			
			// clock the shift reg and clear output data bit afterwards
			clock_sr();
			
			// latch data
			pulse_xlat();
			
			// restart greyscale clock
			unblank();
			
			// delay it to 60 fps2
			//_delay_loop_2(1260);
			
		}
	}
}

void make_checkerboard(UCHAR size) {
	UCHAR i, j;
	UINT f = 0;
	
	for (j = 0; j < 32; j++) {
		for (i = 0; i < 96                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               ; i++) {
			if ((((i/size) + (j/size)) & 1) == 1) {
				frame[f++] = 0xff;
			} else {
				frame[f++] = 0x00;
			}
		}
	}
	
}

void make_stripe(BOOL vertical, UCHAR width) {
	int j;
	
	if (vertical) {
		for (j = 0; j < FRAME_SIZE; j++) {
			frame[j] = ((j/width) & 1) ? 0xff : 0;
		}
	} else  {
		for (j = 0; j < FRAME_SIZE; j++) {
			frame[j] = ((j/(16*6*width)) & 1) ? 0xff : 0;
		}
	}	
}

void make_filled(UCHAR color) {
	int j;
	for (j = 0; j < FRAME_SIZE; j++) {
		frame[j] = color;
	}
}

int main() {
	
	//make_filled(0x00);
	//make_checkerboard(3);
	//make_stripe(TRUE, 3);

	init();
	
	primary_scan_loop2();

	return 0;
}
