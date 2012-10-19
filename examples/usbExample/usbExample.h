
/** \file
 *
 *  Header file for usbExample.c.
 */

#ifndef _USBEXAMPLE_H_
#define _USBEXAMPLE_H_

	/* Includes: */
		#include <avr/io.h>
		#include <avr/wdt.h>
		#include <avr/power.h>
		#include <avr/interrupt.h>
		#include <string.h>
		#include <stdio.h>

		#include "Descriptors.h"

		#include <LUFA/Version.h>
		#include <LUFA/Platform/XMEGA/ClockManagement.h>
		#include <LUFA/Drivers/USB/USB.h>
		
		#include <util/delay.h>
		#include "requiredFiles/avr_compiler.h"
		#include "requiredFiles/clksys_driver.h"

	/* Macros: */
	
	
	#define F_CPU 	32000000UL //32 MHz Internal Oscillator
	#define set_it(p,m) ((p) = (m))
	#define bit_get(p,m) ((p) & (m)) 
	#define bit_set(p,m) ((p) |= (m)) 
	#define bit_clear(p,m) ((p) &= ~(m)) 
	#define bit_flip(p,m) ((p) ^= (m)) 
	#define BIT(x) (0x01 << (x))
	#define LONGBIT(x) ((unsigned long)0x00000001 << (x))


	/* Function Prototypes: */
		void SetupHardware(void);
		void CheckJoystickMovement(void);

		void EVENT_USB_Device_Connect(void);
		void EVENT_USB_Device_Disconnect(void);
		void EVENT_USB_Device_ConfigurationChanged(void);
		void EVENT_USB_Device_ControlRequest(void);
		
		void delay_ms(int ms);
		void blink(int myVal);
		void setupHardware(void);
		void setupIO(void);
		void interruptInit(void);
		int signum(int n);
		long millis(void);		
		
		void process_SET_GOAL(void);
		void process_SET_STOP(void);
		void process_GET_POSE(void);
		void process_GET_STATUS(void);
				

#endif

