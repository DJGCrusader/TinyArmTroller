/* TinyArmTroller Advertisement Demo
 * by Daniel J. Gonzalez
 * July 2012, yameb.blogspot.com
 * 
 * Home: 
 * * Facing right 
 * * Shoulder and elbow down 
 * * Advert facing wherever 
 * * Gripper already gripping
 */

#define F_CPU 	32000000UL //32 MHz Internal Oscillator
#define LOW 0
#define HIGH 1
#define MOTOR_1 Base
#define MOTOR_2 Shoulder
#define MOTOR_3 Elbow
#define MOTOR_4 Right_Wrist
#define MOTOR_5 Left_Wrist
#define MOTOR_6 Gripper
#define MAXSPEED 35

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "requiredFiles/avr_compiler.h"
#include "requiredFiles/clksys_driver.h"
#include "requiredFiles/clksys_driver.c"

#define set_it(p,m) ((p) = (m))
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

int state = INIT;
int debugFlag = 0;
int goal[6]={0,0,0,0,0,0};
int pose[6]={0,0,0,0,0,0};
volatile unsigned long elapsedMillis = 0;


ISR(TCC0_OVF_vect){
	elapsedMillis++;
}

// My own Delay function, because the built-in _delay_ms() can delay for only a limited time
void delay_ms(int ms) 
{ 
    while (ms--) 
        _delay_us(1000);  
} 

// Compare two arrays
int compareArray(int a[6], int b[6]){
	return ((a[0]==b[0])&&
			(a[1]==b[1])&&
			(a[2]==b[2])&&
			(a[3]==b[3])&&
			(a[4]==b[4])&&
			(a[5]==b[5]));
}

//Return only the sign of number n: 1, -1, or 0. 
int signum(int n) { 
	return (n < 0) ? -1 : (n > 0) ? +1 : 0; 
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
	return elapsedMillis;
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
				case 0: set_it(PORTA_OUTSET,BIT(0)); break; 
				case 1: set_it(PORTA_OUTSET,BIT(1)); break;
				case 2: set_it(PORTA_OUTSET,BIT(2)); break;
				case 3: set_it(PORTA_OUTSET,BIT(3)); break;
				case 4: set_it(PORTA_OUTSET,BIT(4)); break;
				case 5: set_it(PORTA_OUTSET,BIT(5)); break;
				case 6: set_it(PORTA_OUTSET,BIT(6)); break;
				case 7: set_it(PORTA_OUTSET,BIT(7)); break;
			}
		break;
		}
		//PORTB
		case 'B':{
			switch (pinNum){
				case 0: set_it(PORTB_OUTSET,BIT(0)); break;
				case 1: set_it(PORTB_OUTSET,BIT(1)); break;
				case 2: set_it(PORTB_OUTSET,BIT(2)); break;
				case 3: set_it(PORTB_OUTSET,BIT(3)); break;
			}
		break;
		}
		//PORTC
		case 'C':{
			switch (pinNum){
				case 0: set_it(PORTC_OUTSET,BIT(0)); break;
				case 1: set_it(PORTC_OUTSET,BIT(1)); break;
				case 2: set_it(PORTC_OUTSET,BIT(2)); break;
				case 3: set_it(PORTC_OUTSET,BIT(3)); break;
				case 4: set_it(PORTC_OUTSET,BIT(4)); break;
				case 5: set_it(PORTC_OUTSET,BIT(5)); break;
				case 6: set_it(PORTC_OUTSET,BIT(6)); break;
				case 7: set_it(PORTC_OUTSET,BIT(7)); break;
			}
		break;
		}
		//PORTD
		case 'D':{
			switch (pinNum){
				case 0: set_it(PORTD_OUTSET,BIT(0)); break;
				case 1: set_it(PORTD_OUTSET,BIT(1)); break;
				case 2: set_it(PORTD_OUTSET,BIT(2)); break;
				case 3: set_it(PORTD_OUTSET,BIT(3)); break;
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
				case 0: set_it(PORTA_OUTCLR,BIT(0)); break; 
				case 1: set_it(PORTA_OUTCLR,BIT(1)); break;
				case 2: set_it(PORTA_OUTCLR,BIT(2)); break;
				case 3: set_it(PORTA_OUTCLR,BIT(3)); break;
				case 4: set_it(PORTA_OUTCLR,BIT(4)); break;
				case 5: set_it(PORTA_OUTCLR,BIT(5)); break;
				case 6: set_it(PORTA_OUTCLR,BIT(6)); break;
				case 7: set_it(PORTA_OUTCLR,BIT(7)); break;
			}
		break;
		}
				//PORTB
		case 'B':{
			switch (pinNum){
				case 0: set_it(PORTB_OUTCLR,BIT(0)); break;
				case 1: set_it(PORTB_OUTCLR,BIT(1)); break;
				case 2: set_it(PORTB_OUTCLR,BIT(2)); break;
				case 3: set_it(PORTB_OUTCLR,BIT(3)); break;
			}
		break;
		}
		//PORTC
		case 'C':{
			switch (pinNum){
				case 0: set_it(PORTC_OUTCLR,BIT(0)); break;
				case 1: set_it(PORTC_OUTCLR,BIT(1)); break;
				case 2: set_it(PORTC_OUTCLR,BIT(2)); break;
				case 3: set_it(PORTC_OUTCLR,BIT(3)); break;
				case 4: set_it(PORTC_OUTCLR,BIT(4)); break;
				case 5: set_it(PORTC_OUTCLR,BIT(5)); break;
				case 6: set_it(PORTC_OUTCLR,BIT(6)); break;
				case 7: set_it(PORTC_OUTCLR,BIT(7)); break;
			}
		break;
		}
		//PORTD
		case 'D':{
			switch (pinNum){
				case 0: set_it(PORTD_OUTCLR,BIT(0)); break;
				case 1: set_it(PORTD_OUTCLR,BIT(1)); break;
				case 2: set_it(PORTD_OUTCLR,BIT(2)); break;
				case 3: set_it(PORTD_OUTCLR,BIT(3)); break;
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
	int minSteps; //Upper Limit of how many steps 
	int maxSteps; //Lower Limit of how many steps 

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

/*
int getMotor(int n){
	switch (n){
				case 0: return MOTOR_1;
				case 1: return MOTOR_2;
				case 2: return MOTOR_3;
				case 3: return MOTOR_4;
				case 4: return MOTOR_5;
				case 5: return MOTOR_6;
}*/

struct Stepper Base = {
	.motorNumber = 1,
	.direction = 1,
	.speed = 40,
	.step_delay = 10,
	.number_of_steps = 200,
	.pin_count = 4,
	.step_number = 0,
	.minSteps = -850,
	.maxSteps = 850,
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
	.minSteps = 0,
	.maxSteps = -1450,
	.motor_port_1 = 'A',
	.motor_pin_1 = 4,
	.motor_port_2 = 'A',
	.motor_pin_2 = 5, 
	.motor_port_3 = 'A',
	.motor_pin_3 = 6, 
	.motor_port_4 = 'A',
	.motor_pin_4 = 7
	};
struct Stepper Elbow = {
	.motorNumber = 3,
	.direction = 1,
	.speed = 40,
	.step_delay = 10,
	.number_of_steps = 200,
	.pin_count = 4,
	.step_number = 0, 
	.minSteps = 0,
	.maxSteps = 1450,
	.motor_port_1 = 'B',
	.motor_pin_1 = 0,
	.motor_port_2 = 'B',
	.motor_pin_2 = 1, 
	.motor_port_3 = 'B',
	.motor_pin_3 = 2, 
	.motor_port_4 = 'B',
	.motor_pin_4 = 3
	};
struct Stepper Right_Wrist = {
	.motorNumber = 4,
	.direction = 1,
	.speed = 40,
	.step_delay = 10,
	.number_of_steps = 200,
	.pin_count = 4,
	.step_number = 0, 
	.minSteps = 200,
	.maxSteps = -200,
	.motor_port_1 = 'C',
	.motor_pin_1 = 0,
	.motor_port_2 = 'C',
	.motor_pin_2 = 1, 
	.motor_port_3 = 'C',
	.motor_pin_3 = 2, 
	.motor_port_4 = 'C',
	.motor_pin_4 = 3
	};
struct Stepper Left_Wrist = {
	.motorNumber = 5,
	.direction = 1,
	.speed = 40,
	.step_delay = 10,
	.number_of_steps = 200,
	.pin_count = 4,
	.step_number = 0, 
	.minSteps = -200,
	.maxSteps = 200,
	.motor_port_1 = 'C',
	.motor_pin_1 = 4,
	.motor_port_2 = 'C',
	.motor_pin_2 = 5, 
	.motor_port_3 = 'C',
	.motor_pin_3 = 6, 
	.motor_port_4 = 'C',
	.motor_pin_4 = 7
	};
	
struct Stepper Gripper = {
	.motorNumber = 6,
	.direction = 1,
	.speed = 40,
	.step_delay = 10,
	.number_of_steps = 200,
	.pin_count = 4,
	.step_number = 0, 
	.minSteps = 0,
	.maxSteps = 800,
	.motor_port_1 = 'D',
	.motor_pin_1 = 0,
	.motor_port_2 = 'D',
	.motor_pin_2 = 1, 
	.motor_port_3 = 'D',
	.motor_pin_3 = 2, 
	.motor_port_4 = 'D',
	.motor_pin_4 = 3
	};
	

// Set Speed, in RPM
// (60 seconds per minute*1000 Microseconds per second)/200 steps per rotation/ 40 Rotations per minute = units of milliseconds/step
void setSpeed(struct Stepper *stepper, long whatSpeed){
	(*stepper).step_delay = 60L * 1000L / (*stepper).number_of_steps / whatSpeed;
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
      case 1:   // 0110
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
	// decrease the number of steps, moving on step each time:
	while(steps_left >0) {
		// move only if the appropriate delay has passed:
		if (millis() - (*stepper).last_step_time >= (*stepper).step_delay){
			// get the timeStamp of when you stepped:
			(*stepper).last_step_time = millis();
			// increase or decrease the step number, depending on direction
			if ((*stepper).direction == 1) {
				(*stepper).step_number++;
				pose[(*stepper).motorNumber -1] ++;
				if ((*stepper).step_number == (*stepper).number_of_steps) {
					(*stepper).step_number = 0;
				}
			}
			else { 
				if ((*stepper).step_number == 0) {
				(*stepper).step_number = (*stepper).number_of_steps;
				}
			(*stepper).step_number--;
			pose[(*stepper).motorNumber -1] --;
			}
		  // decrease the steps left:
		  steps_left--;
		  
		  // step the motor to step number 0, 1, 2, or 3:
		  stepMotor((*stepper),(*stepper).step_number % 4);
		  
		  // Special Case: Move Gripper if Elbow moves, because of mechanical design limitations.
		  if((*stepper).motorNumber==3){
			  
				Gripper.direction = -1*(*stepper).direction;
				
				if (Gripper.direction == 1) {
					Gripper.step_number++;
					if (Gripper.step_number == Gripper.number_of_steps) {
						Gripper.step_number = 0;
					}
				}
				else { 
					if (Gripper.step_number == 0) {
						Gripper.step_number = Gripper.number_of_steps;
					}
					Gripper.step_number--;
				}
				
				stepMotor(Gripper,Gripper.step_number % 4);
			}
		}
  	}
}

// Step Once in a given direction. 
// *stepper is which motor to move
// myDir is what direction to move the motor: 1 is forwards, 0 is backwards
void stepOne(struct Stepper *stepper, int myDir){
	(*stepper).direction = myDir;
	
	// increase or decrease the step number, depending on direction, keeping up odometry
	if ((*stepper).direction == 1) {
		(*stepper).step_number++;
		pose[(*stepper).motorNumber -1] ++;
		if ((*stepper).step_number == (*stepper).number_of_steps) {
			(*stepper).step_number = 0;
		}
	}
	else if((*stepper).direction == 0){
		
	} else { 
		if ((*stepper).step_number == 0) {
			(*stepper).step_number = (*stepper).number_of_steps;
		}
		(*stepper).step_number--;
		pose[(*stepper).motorNumber -1] --;
	}
  // step the motor to step number 0, 1, 2, or 3:
  stepMotor((*stepper),(*stepper).step_number % 4);
  
  // Special Case: Move Gripper if Elbow moves, because of mechanical design limitations.
  if((*stepper).motorNumber==3){
	  
		Gripper.direction = -1*(*stepper).direction;
		
		if (Gripper.direction == 1) {
			Gripper.step_number++;
			if (Gripper.step_number == Gripper.number_of_steps) {
				Gripper.step_number = 0;
			}
		}else if((*stepper).direction == 0){
		
		}else { 
			if (Gripper.step_number == 0) {
				Gripper.step_number = Gripper.number_of_steps;
			}
			Gripper.step_number--;
		}
		
		stepMotor(Gripper,Gripper.step_number % 4);
	}
}

void changeGoal(int val0, int val1, int val2, int val3, int val4, int val5){
	goal[0] = val0;
	goal[1] = val1;
	goal[2] = val2;
	goal[3] = val3;
	goal[4] = val4;
	goal[5] = val5;
}

void moveToGoal(void){
	// Get Error Array
	int error[6] = {
					goal[0]-pose[0],
					goal[1]-pose[1],
					goal[2]-pose[2],
					goal[3]-pose[3],
					goal[4]-pose[4],
					goal[5]-pose[5],					
					};
					
	// Determine the max error
	int maxError = error[0];
	int i;
	for (i = 1; i < 6 ; i++){
		if(abs(error[i])>abs(maxError)){
			maxError = error[i];
		}
	}	
	//Determine the speed each motor needs to step at based on error array
	//Speeds are such that each motor arrives at the end point at the same time. 
	
	setSpeed(&MOTOR_1,(int)(MAXSPEED*(error[0]/(double)maxError)));
	setSpeed(&MOTOR_2,(int)(MAXSPEED*(error[1]/(double)maxError)));
	setSpeed(&MOTOR_3,(int)(MAXSPEED*(error[2]/(double)maxError)));
	setSpeed(&MOTOR_4,(int)(MAXSPEED*(error[3]/(double)maxError)));
	setSpeed(&MOTOR_5,(int)(MAXSPEED*(error[4]/(double)maxError)));
	setSpeed(&MOTOR_6,(int)(MAXSPEED*(error[5]/(double)maxError)));
	
	//Step
	while(!compareArray(pose,goal)){
		
		for(i = 0; i < 6 ; i++){
			error[i] = goal[i]-pose[i];
		}
		
		if (millis() - MOTOR_1.last_step_time >= MOTOR_1.step_delay){
			MOTOR_1.last_step_time = millis();
			stepOne(&MOTOR_1,signum(error[0]));
		}
		if (millis() - MOTOR_2.last_step_time >= MOTOR_2.step_delay){
			MOTOR_2.last_step_time = millis();
			stepOne(&MOTOR_2,signum(error[1]));
		}
		if (millis() - MOTOR_3.last_step_time >= MOTOR_3.step_delay){
			MOTOR_3.last_step_time = millis();
			stepOne(&MOTOR_3,signum(error[2]));
		}
		if (millis() - MOTOR_4.last_step_time >= MOTOR_4.step_delay){
			MOTOR_4.last_step_time = millis();
			stepOne(&MOTOR_4,signum(error[3]));
		}
		if (millis() - MOTOR_5.last_step_time >= MOTOR_5.step_delay){
			MOTOR_5.last_step_time = millis();
			stepOne(&MOTOR_5,signum(error[4]));
		}
		if (millis() - MOTOR_6.last_step_time >= MOTOR_6.step_delay){
			MOTOR_6.last_step_time = millis();
			stepOne(&MOTOR_6,signum(error[5]));
		}
	}
	
}

void usbUpdate(void){
	if(debugFlag > 3) debugFlag = 0;
	if(debugFlag == 0) changeGoal(-850,-200,-200,-200,-200,0);
	if(debugFlag == 1) changeGoal(-1700,-400,-400,-400,-400,0);
	if(debugFlag == 2) changeGoal(-850,-200,-200,-200,-200,0);
	if(debugFlag == 3) changeGoal(0,0,0,0,0,0);
	return;
}

// Main Loop, called after init() in main()
void mainLoop(void){
	while(1){
		if(state == DEBUG){
		} else if(state == INIT){
			while((PORTD.IN & _BV(4))){ //Localize Base
				break;
			}
			pose[0]=0;
			while((PORTD.IN & _BV(5))){ //Localize Shoulder
			
				break;
			}
			pose[1]=0;
			/*
			while((PORTE.IN & _BV(0))){ //Localize Elbow
				break;
			}*/
			pose[2]=0;
			while((PORTE.IN & _BV(1))){ //Localize Wrist Planar Rotation
				break;
			}
			pose[3]=0;
			while((PORTE.IN & _BV(2))){ //Localize Wrist Axial Rotation
				break;
			}
			pose[4]=0;
			while((PORTE.IN & _BV(3))){ //Localize Gripper
				break;
			}
			pose[5]=0;
			state = IDLE;
		} else if(state == IDLE){			
			usbUpdate();
			if(pose!=goal){
				state = MOVING;
			}
		} else if(state == MOVING){
			//If we're not there yet
			if(!compareArray(pose,goal)){
				
				moveToGoal();

			} else if(compareArray(pose,goal)){ //If we got there
					//Send a signal to USB that we got there
					state = IDLE;
					debugFlag++;
			}
		} else if(state == LOST){
			
		}	
	}
}

int main (void)
{
	Config32MHzClock();// Set Clock Speed to 32MHz	
	
	PORTE.DIRSET |= _BV(0); // DEBUG: Set pin E0 as OUTPUT
	
	
	/* Timer for keeping track of milliseconds:
	 * The result of this triggers the interrupt once per millisecond.
	 */
	TCC0.PER = 4000;// Set HIGH to 4000 and LOW to 0
	TCC0.CTRLA = ( TCC0.CTRLA & ~TC0_CLKSEL_gm ) | TC_CLKSEL_DIV8_gc; // Work from CPUCLK/8
	TCC0.INTCTRLA = ( TCC0.INTCTRLA & ~TC0_OVFINTLVL_gm ) | TC_OVFINTLVL_HI_gc; // Enable overflow interrupt.--
	// Enable low interrupt level in PMIC and enable global interrupts.
	PMIC.CTRL |= PMIC_HILVLEN_bm;
	sei();


	/* Set I/O */
	PORTA.DIRSET |= 0XFF;// Set PortA 0-7 to output
	PORTB.DIRSET |= 0xFF;// Set PortB 0-7 to output
	PORTC.DIRSET |= 0xFF;// Set PortC 0-7 to output
	PORTD.DIRSET |= 0x0F;// Set PortD pins 0-3 to output

	PORTD.PIN4CTRL = PORT_OPC_PULLUP_gc; //Set pin D4 as INPUT and as a pullup (Detect when pulled LOW)
	PORTD.PIN5CTRL = PORT_OPC_PULLUP_gc; //Set pin D5 as INPUT and as a pullup (Detect when pulled LOW)
//	PORTE.PIN0CTRL = PORT_OPC_PULLUP_gc; //Set pin E0 as INPUT and as a pullup (Detect when pulled LOW)
	PORTE.PIN1CTRL = PORT_OPC_PULLUP_gc; //Set pin E1 as INPUT and as a pullup (Detect when pulled LOW)
	PORTE.PIN2CTRL = PORT_OPC_PULLUP_gc; //Set pin E2 as INPUT and as a pullup (Detect when pulled LOW)
	PORTE.PIN3CTRL = PORT_OPC_PULLUP_gc; //Set pin E3 as INPUT and as a pullup (Detect when pulled LOW)
	
	mainLoop();
	return 0;
}
