#ifndef __MAIN_H

#define __MAIN_H

#include <avr/io.h>

// --- some defines to make programming cleaner
#define UCHAR unsigned char
#define UINT unsigned int
#define BOOL unsigned char

#define TRUE  1
#define FALSE 0

// --- grid size info
#define MAX_ROWS    32
#define MAX_COLS    (16*6)
#define FRAME_SIZE  (MAX_ROWS * MAX_COLS)

// --- LED control pin mnemonics
#define VPRG	PB4
#define XLAT	PD2
#define DCPRG	PD3
#define GSCLK	PD4
#define BLANK	PD5

// --- shift reg mnemonics
#define SR_DATA  PC1
#define SR_CLOCK PC0

// --- unpacked greyscale control packet sizes (per-chip = 24 bytes of SPI from 16 bytes of frame data)
#define CH1_START       0
#define CH2_START       (24 * 4)

#define CH1_SIZE		(24 * 4)
#define CH2_SIZE		(24 * 2)
#define GS_PACKET_SIZE  (24 * 6)

#define GS_DATA_SIZE    	16 * 6
#define GS_DATA_CH2_OFFSET	16 * 2
// make the 2 a 4 if it gets fucked up. 

// --- wastes exactly one cycle...
#define nop() asm("nop": : )

// --- set and clear using assembly instuction <= PORTF (SFR 0..63)
#define cbi(port, bit) asm("cbi %0, %1" : : "I" (_SFR_IO_ADDR(port)), "I" (bit) )
#define sbi(port, bit) asm("sbi %0, %1" : : "I" (_SFR_IO_ADDR(port)), "I" (bit) )

// --- set and clear for registers above PORTF (#64 and above)
#define cbi_far(port, bit) port &= (unsigned char)~_BV(bit)
#define sbi_far(port, bit) port |= (unsigned char)_BV(bit)

// --- number of frames to wait before resetting
#define UPDATE_LOSS_LIMIT 64

// --- define this to switch the UART from SPI to debug serial mode
//#define USART_IN_DEBUG_MODE

void init(void);
int main(void);
void primary_scan_loop2(void);
void make_checkerboard(UCHAR size);
void make_filled(UCHAR color);
void make_stripe(BOOL vertical, UCHAR width);
void uart_send(char c);

#endif
