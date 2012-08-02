/* TinyArmTroller Firmware v0.02
 * by Daniel J. Gonzalez and Azureviolin, modified from Arduino (*stepper).cpp V.04
 * July 2012, yameb.blogspot.com
 * 
 * TODO: 
 *  - Implement millis() with timer and drive first motor
 *  - Finish mainLoop() implementation
 *  - - Initialization/localize State
 *  - - Check limits and do not exceed them
 *  - Implement V-USB
 */

#define F_CPU 	32000000UL //32 MHz Internal Oscillator
#define LOW 0
#define HIGH 1
#define MOTOR_1 Base
#define MOTOR_2 Shoulder
#define MOTOR_3 Elbow
#define MOTOR_4 Left_Wrist
#define MOTOR_5 Right_Wrist
#define MOTOR_6 Gripper

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
// #define bit_write(c<Plug>PeepOpen,m) (c ? bit_set(p,m) : bit_clear(p,m)) //Giving me issues with c<Plug>PeepOpen
#define BIT(x) (0x01 << (x))
#define LONGBIT(x) ((unsigned long)0x00000001 << (x))

// State Machine Definitions
#define DEBUG 0
#define INIT 1
#define IDLE 2
#define MOVING 3
#define LOST 4

// Function Definitions
int state = DEBUG;
//int[] goal={0,0,0,0,0,0};
unsigned int elapsedTimeInMillis = 0;
unsigned int elapsedTimeInSeconds = 0;


ISR(TCC0_OVF_vect){
	elapsedTimeInMillis++;
	if(elapsedTimeInMillis == 1000){
		elapsedTimeInSeconds++;
		elapsedTimeInMillis = 0;
	}
}

// My own Delay function, because the built-in _delay_ms() can delay for only a limited time
void delay_ms(int ms) 
{ 
    while (ms--) 
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
	return 1000*elapsedTimeInSeconds+elapsedTimeInMillis;
}

void blink(int myVal){
		PORTE.OUTSET = _BV(0);
        delay_ms(myVal);
        PORTE.OUTCLR = _BV(0);
        delay_ms(myVal);
}

// Set selected pin to 1/HIGH 
// TODO: rest of ports
void setPin(char portNum, int pinNum){
	switch (portNum){
		//PORTA 
		case 'A':{
			switch (pinNum){
				case 0: bit_set(PORTA_OUTSET,BIT(0)); break; //Set PA0 = HIGH
				case 1: bit_set(PORTA_OUTSET,BIT(1)); break;
				case 2: bit_set(PORTA_OUTSET,BIT(2)); break;
				case 3: bit_set(PORTA_OUTSET,BIT(3)); break;
			}
		break;
		}
	}
}

// Set selected pin to 0/LOW 
// TODO: Rest of ports
void clearPin(char portNum, int pinNum){
	switch (portNum){
		//PORTA
		case 'A':{
			switch (pinNum){
				case 0: bit_set(PORTA_OUTCLR,BIT(0)); break; //Set PA0 = HIGH
				case 1: bit_set(PORTA_OUTCLR,BIT(1)); break;
				case 2: bit_set(PORTA_OUTCLR,BIT(2)); break;
				case 3: bit_set(PORTA_OUTCLR,BIT(3)); break;
			}
		break;
		}
	}
}

// struct of Stepper Motor
struct Stepper{
	int motorNumber; //Which motor is this, starting at 1
	int direction;
	int speed; // RPM
	unsigned long step_delay; //ms
	int number_of_steps; 
	int pin_count; // 2 pin or 4 pin configuration
	int step_number; //which step the motor is on now
	int maxSteps; //Limit of how many steps 

	// motor pin numbers:

	char motor_port_1;
	int motor_pin_1;
	char motor_port_2;
	int motor_pin_2;
	char motor_port_3;
	int motor_pin_3;
	char motor_port_4;
	int motor_pin_4;

	long last_step_time; //ms, time stamp when last step was taken
} Base,Shoulder,Elbow,Left_Wrist,Right_Wrist,Gripper;

struct Stepper Base = {
	.motorNumber = 1,
	.direction = 1,
	.speed = 40,
	.step_delay = 10,
	.number_of_steps = 200,
	.pin_count = 4,
	.step_number = 0, 
	.maxSteps = 2100,
	.motor_port_1 = 'A',
	.motor_pin_1 = 0,
	.motor_port_2 = 'A',
	.motor_pin_2 = 1, 
	.motor_port_3 = 'A',
	.motor_pin_3 = 2, 
	.motor_port_4 = 'A',
	.motor_pin_4 = 3
	};
	
struct Stepper Shoulder = {
	.motorNumber = 2,
	.direction = 1,
	.speed = 40,
	.step_delay = 10,
	.number_of_steps = 200,
	.pin_count = 4,
	.step_number = 0, 
	.maxSteps = 2100,
	.motor_port_1 = 'A',
	.motor_pin_1 = 4,
	.motor_port_2 = 'A',
	.motor_pin_2 = 5, 
	.motor_port_3 = 'A',
	.motor_pin_3 = 6, 
	.motor_port_4 = 'A',
	.motor_pin_4 = 7
	};
	

// Set Speed, in RPM
void setSpeed(struct Stepper *stepper, long whatSpeed){
	(*stepper).step_delay = 60L * 1000L / (*stepper).number_of_steps / whatSpeed;
	blink(50);
}

// Moves the motor by 1 step. calls clearPin and setPin.
void stepMotor(struct Stepper stepper, int thisStep) {
  if ((stepper).pin_count == 2) {
    switch (thisStep) {
      case 0: // 01 
      clearPin((stepper).motor_port_1,(stepper).motor_pin_1);
      setPin((stepper).motor_port_2,(stepper).motor_pin_2);
      break;
      case 1: // 11 
      setPin((stepper).motor_port_1,(stepper).motor_pin_1);
      setPin((stepper).motor_port_2,(stepper).motor_pin_2);
      break;
      case 2: // 10 
      setPin((stepper).motor_port_1,(stepper).motor_pin_1);
      clearPin((stepper).motor_port_2,(stepper).motor_pin_2);
      break;
      case 3: // 00 
      clearPin((stepper).motor_port_1,(stepper).motor_pin_1);
      clearPin((stepper).motor_port_2,(stepper).motor_pin_2);
      break;
    } 
  }
  if ((stepper).pin_count == 4) {
    switch (thisStep) {
      case 0:    // 1010
      setPin((stepper).motor_port_1,(stepper).motor_pin_1);
      clearPin((stepper).motor_port_2,(stepper).motor_pin_2);
      setPin((stepper).motor_port_3,(stepper).motor_pin_3);
      clearPin((stepper).motor_port_4,(stepper).motor_pin_4);
      break;
      case 1:    // 0110
      clearPin((stepper).motor_port_1,(stepper).motor_pin_1);
      setPin((stepper).motor_port_2,(stepper).motor_pin_2);
      setPin((stepper).motor_port_3,(stepper).motor_pin_3);
      clearPin((stepper).motor_port_4,(stepper).motor_pin_4);
      break;
      case 2:    //0101
      clearPin((stepper).motor_port_1,(stepper).motor_pin_1);
      setPin((stepper).motor_port_2,(stepper).motor_pin_2);
      clearPin((stepper).motor_port_3,(stepper).motor_pin_3);
      setPin((stepper).motor_port_4,(stepper).motor_pin_4);
      break;
      case 3:    //1001
      setPin((stepper).motor_port_1,(stepper).motor_pin_1);
      clearPin((stepper).motor_port_2,(stepper).motor_pin_2);
      clearPin((stepper).motor_port_3,(stepper).motor_pin_3);
      setPin((stepper).motor_port_4,(stepper).motor_pin_4);
      break;
    } 
  }
}


// Move (*stepper). This function calls millis() and stepMotor()
void step(struct Stepper *stepper, int steps_to_move){
	int steps_left = 0;

	if (steps_to_move > 1) {
		steps_left = steps_to_move; // how many steps to take
		(*stepper).direction = 1;// determine direction
	} 
    else {
		steps_left = -steps_to_move;
		(*stepper).direction = 0;
	}
	
		blink(50);
		blink(50);
		blink(50);
		blink(50);
		blink(50);
	// decrease the number of steps, moving on step each time:
	while(steps_left >0) {
		blink(200);		
		// move only if the appropriate delay has passed:
		if (millis() - (*stepper).last_step_time >= (*stepper).step_delay){
			
			blink(2000);	
			
			// get the timeStamp of when you stepped:
			(*stepper).last_step_time = millis();
			// increase or decrease the step number, depending on direction
			if ((*stepper).direction == 1) {
				(*stepper).step_number++;
				if ((*stepper).step_number == (*stepper).number_of_steps) {
					(*stepper).step_number = 0;
				}
			}
			else { 
				if ((*stepper).step_number == 0) {
				(*stepper).step_number = (*stepper).number_of_steps;
				}
			(*stepper).step_number--;
			}
		  // decrease the steps left:
		  steps_left--;
		  // step the motor to step number 0, 1, 2, or 3:
		  stepMotor(*stepper,(*stepper).step_number % 4);
		  if((*stepper).motorNumber==2)
			stepMotor(Gripper,Gripper.step_number );
		}
  	}
}

void usbUpdate(void){
	
	return;
}

void motorUpdate(void){
	return;
}

// Main Loop, called after init() in main()
void mainLoop(void){	
	while(1){
		usbUpdate();
		if(state == DEBUG){
			//setSpeed(&Shoulder,30);
			step(&Shoulder,400);
			delay_ms(2000);
			step(&Shoulder,-400);
			delay_ms(2000);
		} else if(state == INIT){
			while((PORTD.IN & _BV(4))){ //Localize Base
				break;
			}
			while((PORTD.IN & _BV(5))){ //Localize Shoulder
				break;
			}
			while((PORTE.IN & _BV(0))){ //Localize Elbow
				break;
			}
			while((PORTE.IN & _BV(1))){ //Localize Wrist Planar Rotation
				break;
			}
			while((PORTE.IN & _BV(2))){ //Localize Wrist Axial Rotation
				break;
			}
			while((PORTE.IN & _BV(3))){ //Localize Gripper
				break;
			}
			state = IDLE;
		} else if(state == IDLE){
			//If we receive a USB command
			if(1){
				state = MOVING;
			}
		} else if(state == MOVING){
			//If we're not there yet
			if(1){
				//If there is no "USB Cancel", "USB Hold", 
				if(1){
					motorUpdate();
					//If we hit Grasp Switch
					if(!(PORTD.IN & _BV(4))){
						//Send "Grasp Successful" to PC via USB
						
					}
					
					//If we reached goal
					if(1){
						//Send "Reached Goal" to PC via USB
					}
					
					//If we hit a limit switch
					if(1){
						//Send "Odometry Error" to PC via USB
					}

				}
			} else if(1){ //If we got there
					state = IDLE;
			} 
		} else if(state == LOST){
			
		}	
	}
}

int main (void)
{
	Config32MHzClock();// Set Clock Speed to 32MHz
	
	/* Set I/O */
	PORTE.DIRSET |= _BV(0); // DEBUG: Set pin E0 as OUTPUT
	
	PORTA.DIRSET |= 0XFF;// Set PortA 0-7 to output
	PORTB.DIRSET |= 0xFF;// Set PortB 0-7 to output
	PORTC.DIRSET |= 0xFF;// Set PortC 0-7 to output
	PORTD.DIRSET |= 0x0F;// Set PortD pins 0-3 to output
/*
	PORTD.PIN4CTRL = PORT_OPC_PULLUP_gc; //Set pin D4 as INPUT and as a pullup (Detect when pulled LOW)
	PORTD.PIN5CTRL = PORT_OPC_PULLUP_gc; //Set pin D5 as INPUT and as a pullup (Detect when pulled LOW)
	PORTE.PIN0CTRL = PORT_OPC_PULLUP_gc; //Set pin E0 as INPUT and as a pullup (Detect when pulled LOW)
	PORTE.PIN1CTRL = PORT_OPC_PULLUP_gc; //Set pin E1 as INPUT and as a pullup (Detect when pulled LOW)
	PORTE.PIN2CTRL = PORT_OPC_PULLUP_gc; //Set pin E2 as INPUT and as a pullup (Detect when pulled LOW)
	PORTE.PIN3CTRL = PORT_OPC_PULLUP_gc; //Set pin E3 as INPUT and as a pullup (Detect when pulled LOW)
*/
	
	
	/* Timer for keeping track of milliseconds: 
	 * 
	 * The result of this triggers the interrupt once per millisecond.
	 */
	TCC0.CTRLA |=  TC_CLKSEL_DIV8_gc; // Divide Clock by 8
	TCC0.PERBUF = 4000;// Set HIGH to 4000 and LOW to 0
	TCC0.INTCTRLA |= TC_OVFINTLVL_HI_gc; // enable HI interrupt
    TCC0.CTRLB    = ( TCC0.CTRLB & ~TC0_WGMODE_gm ) | TC_WGMODE_NORMAL_gc;
	TCC0.CTRLFSET = 0x0; // count upwards, but don't start to interrupt yet
	/* Enable interrupts.                */
	PMIC.CTRL |= PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm;
//	sei();

	blink(1000);
	delay_ms(1000);
	mainLoop();
	return 0;
}
