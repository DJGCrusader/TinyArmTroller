/* TinyArmTroller Firmware v0.02
 * by Daniel J. Gonzalez and Azureviolin, modified from Arduino stepper.cpp V.04
 * July 2012, yameb.blogspot.com
 * 
 * TODO: 
 *  - Finish mainLoop() implementation
 *  - - Initialization/localize State
 *  - - - Use single switch, don't mount
 *  - - - mount multiple switches
 *  - - - Finish kinematic characterization
 *  - Implement USB
 * 
 * Configuration:
 *  - Port A0-D3 are Stepper Outputs
 *  - Port D4 is Gripper Limit Switch Input. 
 *  - Port D6 is USB D-
 *  - Port D7 is USB D+
 *  - Port D5, E0 - E3 Unused I/O
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
#define MAXSPEED 65
#define NAK  0x15

#include <stdio.h> 
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
int maxSpeed = 50; //in RPM

int gripperState = 0; 
int gripperStatePrev = 0;

static int uart_putchar(char c, FILE *stream); 
static void uart_init (void); 

static FILE mystdout = FDEV_SETUP_STREAM (uart_putchar, NULL, _FDEV_SETUP_WRITE);
//static FILE mystdin = FDEV_SETUP_STREAM (uart_putchar, NULL, _FDEV_SETUP_READ);

static int uart_putchar (char c, FILE *stream);

int uart_getchar (void);

// Init USART.  Transmit only (we're not receiving anything) 
// We use USARTD1, transmit pin on PC7. 
// Want 115200 baud. Have a 32 MHz clock. BSCALE = 0 
// BSEL = ( 32000000 / (2^0 * 16*115200)) -1 = 16.361111111
// Fbaud = 32000000 / (2^0 * 16 * (16+1))  = 117647 bits/sec 
static void uart_init (void);

// Compare two arrays
int compareArray(int a[6], int b[6]);

//Return only the sign of number n: 1, -1, or 0. 
int signum(int n);

// Set Clock Speed to 32MHz (XMega)
void Config32MHzClock(void);

//Get time elapsed in millis 
long millis(void);

void blink(int myVal);

void blinkN(int n);

// Set selected pin to 1/HIGH 
// TODO: rest of ports
void setPin(char portNum, int pinNum);

// Set selected pin to 0/LOW 
// TODO: Rest of ports
void clearPin(char portNum, int pinNum);

// struct of Stepper Motor
struct Stepper;

void setSpeed(struct Stepper *stepper, long whatSpeed);
void stepMotor(struct Stepper stepper, int thisStep);
void step(struct Stepper *stepper, int steps_to_move);
void stepOne(struct Stepper *stepper, int myDir);
void changeGoal(int val0, int val1, int val2, int val3, int val4, int val5);
void moveToGoal(void);
void mainLoop(void);
void updateSign(void);
void update(void);
void init(void);
int main (void);