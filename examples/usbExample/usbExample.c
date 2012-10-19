/* TinyArmTroller V-USB Example
 * 
 * Trying to use LUFA v120730:
 *  - Endpoint 0 
 *  - - Device Enumeration (Automatic by LUFA)
 *  - Control Transfer
 *  - - Setup Transaction:
 *  - - 1. SETUP Token from Host
 *  - - - - EVENT_USB_Device_Control_Request() called
 *  - - - - calling Endpoint_ClearSETUP() allows user to process packet
 *  - - 2. Setup Data (details about control request) from Host
 *  - - 3. ACK (handshake) from Device (automatic)
 *  - - Data Transaction: 
 *  - - - OUT (From Host to Device)
 *  - - - 1. OUT
 *  - - - 2. Data x
 *  - - - - - Endpoint_Read_xx(...)
 *  - - - 3. ACK sent by device
 *  - - - - - Endpoint_ClearOUT(); called by user
 *  - - - IN (From Device to Host)
 *  - - - 1. IN
 *  - - - 2. Data x
 *  - - - - - Endpoint-Write_xx(...)
 *  - - - - - - Data x, STALL, NAK
 *  - - - 3. ACK sent by Host
 *  - - Status Transaction:
 *  - - - 1. IN if it sent data / OUT if it received data
 *  - - - 2. Data 0 (empty) by Device/ Data0 Empty by Host
 *  - - - - - Endpoint_ClearIN(); called to receive handshake
 *  - - - 3. ACK by Host / ACK by Device
 *  - - - - - Endpoint_ClearOUT(); called to meet handshake
 *  - - - Note: Endpoint_ClearStatusStage() calls clearOUT or IN depending on the situation
 *  - Example: 
 *  - - USB_Task() called periodically
 *  - - USB control requests (bRequest) must be defined as constants
 *  - - bmRequestType can be indicated
 */
 
#include "usbExample.h"

//Desired operation in bRequest field of USB Setup data.
#define SET_GOAL 0
#define SET_STOP 1 
#define GET_STATUS 2
#define GET_POSE 3

volatile unsigned long elapsedMillis = 0;

/** LUFA CDC Class driver interface configuration and state information. This structure is
 *  passed to all CDC Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface =
	{
		.Config =
			{
				.ControlInterfaceNumber   = 0,
				.DataINEndpoint           =
					{
						.Address          = CDC_TX_EPADDR,
						.Size             = CDC_TXRX_EPSIZE,
						.Banks            = 1,
					},
				.DataOUTEndpoint =
					{
						.Address          = CDC_RX_EPADDR,
						.Size             = CDC_TXRX_EPSIZE,
						.Banks            = 1,
					},
				.NotificationEndpoint =
					{
						.Address          = CDC_NOTIFICATION_EPADDR,
						.Size             = CDC_NOTIFICATION_EPSIZE,
						.Banks            = 1,
					},
			},
	};

/** Standard file stream for the CDC interface when set up, so that the virtual CDC COM port can be
 *  used like any regular character stream in the C APIs
 */
static FILE USBSerialStream;


/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
	int i;
	for (i = 0; i < 10; i++){
		blink(50);
	}

}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
	blink(500);
	blink(500);
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;
	ConfigSuccess &= CDC_Device_ConfigureEndpoints(&VirtualSerial_CDC_Interface);
	
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	//blink(5000);
//	CDC_Device_ProcessControlRequest(&VirtualSerial_CDC_Interface);
	
	if(((USB_ControlRequest.bmRequestType & CONTROL_REQTYPE_TYPE) == REQTYPE_CLASS) &&
		((USB_ControlRequest.bmRequestType & CONTROL_REQTYPE_RECIPIENT) == REQREC_DEVICE)){
		if((USB_ControlRequest.bmRequestType & CONTROL_REQTYPE_DIRECTION) == REQDIR_HOSTTODEVICE){
			switch(USB_ControlRequest.bRequest){
				case SET_GOAL:
					process_SET_GOAL();
					break;
				case SET_STOP:
					process_SET_STOP();
					break;
			}
		}else{
			switch(USB_ControlRequest.bRequest){
				case GET_STATUS:
					process_GET_STATUS();
					break;
				case GET_POSE:
					process_GET_POSE();
					break;
			}
		}
	}
	
}


//LUFA USB Event Processer Functions
void process_SET_GOAL(){
	uint16_t newGoal[6];
	/* Accept command */
	Endpoint_ClearSETUP();
	/* read data from endpoint */
	Endpoint_Read_Control_Stream_LE(newGoal, 16);
	/* and mark the whole request as successful: */
	Endpoint_ClearStatusStage();
	/* process command parameters: */
	//Call changeGoal(newGoal);
}
void process_SET_STOP(){
	/* Accept command */
	Endpoint_ClearSETUP();
	/* and mark the whole request as successful: */
	Endpoint_ClearStatusStage();
	/* Process Command Parameters: */
	//Call fuckingStop();
}
void process_GET_STATUS(){
	uint16_t myStatus = 0;
	Endpoint_ClearSETUP();
	Endpoint_Write_16_LE(myStatus);
	/* send packet */
	Endpoint_ClearIN();
	/* and mark the whole request as successful: */
	Endpoint_ClearStatusStage();	
}
void process_GET_POSE(){
	uint16_t myPose[6];
	myPose[0] = 0;//Do This Right
	Endpoint_ClearSETUP();
	Endpoint_Write_Stream_LE(myPose, 16, 0);
	Endpoint_ClearIN();
	Endpoint_ClearStatusStage();
}

void delay_ms(int ms) 
{ 
    while (ms--) 
        _delay_us(1000);  
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

//Return only the sign of number n: 1, -1, or 0. 
int signum(int n) { 
	return (n < 0) ? -1 : (n > 0) ? +1 : 0; 
}


/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware(void)
{
	// Start the PLL to multiply the 2MHz RC oscillator to 32MHz and switch the CPU core to run from it
	XMEGACLK_StartPLL(CLOCK_SRC_INT_RC2MHZ, 2000000, F_CPU);
	XMEGACLK_SetCPUClockSource(CLOCK_SRC_PLL);

	// Start the 32MHz internal RC oscillator and start the DFLL to increase it to 48MHz using the USB SOF as a reference
	XMEGACLK_StartInternalOscillator(CLOCK_SRC_INT_RC32MHZ);
	XMEGACLK_StartDFLL(CLOCK_SRC_INT_RC32MHZ, DFLL_REF_INT_USBSOF, F_USB);

	PMIC.CTRL = PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm;

	/* Hardware Initialization */
	USB_Init();
}

void setupIO(void){
	PORTE.DIRSET |= _BV(0); // DEBUG: Set pin E0 as OUTPUT
}

void interruptInit(void){
		/* Timer for keeping track of milliseconds:
	 * The result of this triggers the interrupt once per millisecond.
	 */
	TCC0.PER = 6000;// Set HIGH to 4000
	TCC0.CTRLA = ( TCC0.CTRLA & ~TC0_CLKSEL_gm ) | TC_CLKSEL_DIV8_gc; // Work from CPUCLK/8
	TCC0.INTCTRLA = ( TCC0.INTCTRLA & ~TC0_OVFINTLVL_gm ) | TC_OVFINTLVL_HI_gc; // Enable overflow interrupt.--
	// Enable low interrupt level in PMIC and enable global interrupts.
	PMIC.CTRL |= PMIC_HILVLEN_bm;
	sei();
}

/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
	SetupHardware();
	setupIO();
	interruptInit();

	/* Create a regular character stream for the interface so that it can be used with the stdio.h functions */
	CDC_Device_CreateStream(&VirtualSerial_CDC_Interface, &USBSerialStream);

	sei();

	
	while (1)
	{
		
		/* Must throw away unused bytes from the host, or it will lock up while waiting for the device */
		CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);

		CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
		USB_USBTask();
	}
}
