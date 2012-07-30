/* TinyArmTroller Firmware v0.02
 * by Daniel J. Gonzalez and Azureviolin, modified from Arduino Stepper.cpp V.04
 * July 2012, yameb.blogspot.com
 * 
 */

#define F_CPU 	32000000UL //32 MHz Internal Oscillator
#define LOW 0
#define HIGH 1

//State Machine Definitions
#define INIT 0
#define IDLE 1
#define MOVING 2
#define LOST 3

#include <avr/io.h>
#include <util/delay.h>
#include "requiredFiles/avr_compiler.h"
#include "requiredFiles/clksys_driver.h"
#include "requiredFiles/clksys_driver.c"

#define bit_get(p,m) ((p) & (m)) 
#define bit_set(p,m) ((p) |= (m)) 
#define bit_clear(p,m) ((p) &= ~(m)) 
#define bit_flip(p,m) ((p) ^= (m)) 
//#define bit_write(c<Plug>PeepOpen,m) (c ? bit_set(p,m) : bit_clear(p,m)) //Giving me issues with c<Plug>PeepOpen
#define BIT(x) (0x01 << (x))
#define LONGBIT(x) ((unsigned long)0x00000001 << (x))

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

long millis(void){
	return 0;
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
	int motorNumber; //Which motor is this
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

// Set Speed, in RPM
void setSpeed(struct Stepper stepper, long whatSpeed){
	stepper.step_delay = 60L * 1000L / stepper.number_of_steps / whatSpeed;
}

// Moves the motor by 1 step. calls clearPin and setPin.
void stepMotor(struct Stepper stepper, int thisStep) {
  if (stepper.pin_count == 2) {
    switch (thisStep) {
      case 0: // 01 
      clearPin(stepper.motor_port_1,stepper.motor_pin_1);
      setPin(stepper.motor_port_2,stepper.motor_pin_2);
      break;
      case 1: // 11 
      setPin(stepper.motor_port_1,stepper.motor_pin_1);
      setPin(stepper.motor_port_2,stepper.motor_pin_2);
      break;
      case 2: // 10 
      setPin(stepper.motor_port_1,stepper.motor_pin_1);
      clearPin(stepper.motor_port_2,stepper.motor_pin_2);
      break;
      case 3: // 00 
      clearPin(stepper.motor_port_1,stepper.motor_pin_1);
      clearPin(stepper.motor_port_2,stepper.motor_pin_2);
      break;
    } 
  }
  if (stepper.pin_count == 4) {
    switch (thisStep) {
      case 0:    // 1010
      setPin(stepper.motor_port_1,stepper.motor_pin_1);
      clearPin(stepper.motor_port_2,stepper.motor_pin_2);
      setPin(stepper.motor_port_3,stepper.motor_pin_3);
      clearPin(stepper.motor_port_4,stepper.motor_pin_4);
      break;
      case 1:    // 0110
      clearPin(stepper.motor_port_1,stepper.motor_pin_1);
      setPin(stepper.motor_port_2,stepper.motor_pin_2);
      setPin(stepper.motor_port_3,stepper.motor_pin_3);
      clearPin(stepper.motor_port_4,stepper.motor_pin_4);
      break;
      case 2:    //0101
      clearPin(stepper.motor_port_1,stepper.motor_pin_1);
      setPin(stepper.motor_port_2,stepper.motor_pin_2);
      clearPin(stepper.motor_port_3,stepper.motor_pin_3);
      setPin(stepper.motor_port_4,stepper.motor_pin_4);
      break;
      case 3:    //1001
      setPin(stepper.motor_port_1,stepper.motor_pin_1);
      clearPin(stepper.motor_port_2,stepper.motor_pin_2);
      clearPin(stepper.motor_port_3,stepper.motor_pin_3);
      setPin(stepper.motor_port_4,stepper.motor_pin_4);
      break;
    } 
  }
}


// Move Stepper. This function calls millis() and stepMotor()
void step(struct Stepper stepper, int steps_to_move){
	int steps_left = 0;
	if (steps_to_move > 0) {
		steps_left = steps_to_move; // how many steps to take
		stepper.direction = 1;// determin direction
	} 
    else {
		steps_left = -steps_to_move;
		stepper.direction = 0;
	}
	
	// decrease the number of steps, moving on step each time:
	while(steps_left >0) {
	// move only if the appropriate delay has passed:
	if (millis() - stepper.last_step_time >= stepper.step_delay){
		// get the timeStamp of when you stepped:
		stepper.last_step_time = millis();
		// increase or decrease the step number, depending on direction
		if (stepper.direction == 1) {
			stepper.step_number++;
			if (stepper.step_number == stepper.number_of_steps) {
          		stepper.step_number = 0;
        	}
		}
      	else { 
        	if (stepper.step_number == 0) {
          	stepper.step_number = stepper.number_of_steps;
        	}
        stepper.step_number--;
      	}
      // decrease the steps left:
      steps_left--;
      // step the motor to step number 0, 1, 2, or 3:
      stepMotor(stepper,stepper.step_number % 4);
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
	int state = INIT;
	while(1){
		usbUpdate();
		
		if(state == INIT){
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

// Initialization, first thing called by main()
void init(void){
		Config32MHzClock(); // Set Clock Speed to 32MHz
		
		PORTA_DIR = 0XFF;// Set PortA to output
		PORTB_DIR = 0xFF;// Set PortB to output
		PORTC_DIR = 0xFF;// Set PortC to output
		PORTD_DIR = (_BV(0)|_BV(1)|_BV(2)|_BV(3));// Set PortD pins 0123 to output
		
		PORTD.PIN4CTRL = PORT_OPC_PULLUP_gc; //Set pin D4 as INPUT and as a pullup (Detect when pulled LOW)
		PORTD.PIN5CTRL = PORT_OPC_PULLUP_gc; //Set pin D5 as INPUT and as a pullup (Detect when pulled LOW)
		PORTE.PIN0CTRL = PORT_OPC_PULLUP_gc; //Set pin E0 as INPUT and as a pullup (Detect when pulled LOW)
		PORTE.PIN1CTRL = PORT_OPC_PULLUP_gc; //Set pin E1 as INPUT and as a pullup (Detect when pulled LOW)
		PORTE.PIN2CTRL = PORT_OPC_PULLUP_gc; //Set pin E2 as INPUT and as a pullup (Detect when pulled LOW)
		PORTE.PIN3CTRL = PORT_OPC_PULLUP_gc; //Set pin E3 as INPUT and as a pullup (Detect when pulled LOW)
}

int main (void)
{
	init();	
	delay_ms(1000);
	mainLoop();
	return 0;
}
