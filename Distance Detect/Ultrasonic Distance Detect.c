#include <p32xxxx.h>
#include <plib.h>

#define Ultrasonic1_ECHO	PORTDbits.RD8
#define Ultrasonic1_TRIG	PORTDbits.RD9
#define Ultrasonic2_ECHO	PORTDbits.RD10
#define Ultrasonic2_TRIG	PORTDbits.RD11

float distance1 = 0.0;		// unit: m
float distance2 = 0.0;		// unit: m
unsigned char detector = 0x1;	// choose which detector
short IC1begin = 0;				// flag for rising edge(0) / falling edge(1)
short IC3begin = 0;				// flag for rising edge(0) / falling edge(1)

void Config_Timer5() {
	IPC5SET = 0x0000000C;	// Set priority level = 3
	IPC5SET = 0x00000001;	// Set subpriority level = 1
	IFS0CLR = 0x00100000;	// Clear Timer 5 interrupt flag
	IEC0SET = 0x00100000;	// Enable Timer 5 interrupts

	// 16-bit timer, 100ms, SYSCLK = 4 MHz, PBCLK = SYSCLK/1
	T5CON = 0x0060; 	// stop Timer 5 and clear control register, 16-bit timer
						// set prescaler as 1:64, internal clock source
	TMR5 = 0x0; 		// clear timer register
	PR5 = 0x1869;		// load period register, 100ms
	T5CONSET = 0x8000;	// start Timer 5
}

#pragma interrupt T5_ISR ipl3 vector 20
void T5_ISR() {
	if (detector == 0x1){
		Ultrasonic1_TRIG = 1;	// Generate 10 us pulse signal to trigger Ultrasonic Distance Detector 1
		unsigned int tStart = ReadCoreTimer();
		while((ReadCoreTimer()-tStart)<40);
		Ultrasonic1_TRIG = 0;
		detector = 0x2;
	}
	else if (detector == 0x2){
		Ultrasonic2_TRIG = 1;	// Generate 10 us pulse signal to trigger Ultrasonic Distance Detector 2
		unsigned int tStart = ReadCoreTimer();
		while((ReadCoreTimer()-tStart)<40);
		Ultrasonic2_TRIG = 0;
		detector == 0x1;
	}
	IFS0CLR = 0x00100000;	// Clear Timer 5 interrupt flag
}

void Config_UltrasonicDistDetectors(){
	PORTDCLR = 0x0F00;	// clear RD[8], RD[9], RD[10], RD[11]
	TRISDCLR = 0x0F00;	// set RD[8] = TRIG1
						// set RD[9] = ECHO1
						// set RD[10] = TRIG2
						// set RD[11] = ECHO2
}

void Config_Timer3() {
	// 16-bit timer, SYSCLK = 4 MHz, PBCLK = SYSCLK/1
	T3CON = 0x0020; 	// stop Timer3 and clear control register
						// configure Timer3 as 16-bit timer,
						// set prescaler as 1:4, internal clock source
	TMR3 = 0x0; 		// clear timer register
	PR3 = 0xFFFF;		// load period register
	T3CONSET = 0x8000;	// start Timer 3
}

void Config_ICs(){
	/** Config **/
	IPC1SET = 0x00001000;	// Set priority level = 4
	IPC1SET = 0x00000100;	// Set subpriority level = 1
	IFS0CLR = 0x00000020;	// Clear IC1 interrupt flag
	IEC0SET = 0x00000020;	// Enable IC1 interrupts
	IC1CON = 0x00000206;	// Disable and reset the IC Module, disable interrupt generation and allow SFR modifications;
							// simple Capture Event mode, capture rising edge first, count source is 16-bit Timer3
							// interrupt on every capture event
	IPC3SET = 0x00001000;	// Set priority level = 4
	IPC3SET = 0x00000100;	// Set subpriority level = 1
	IFS0CLR = 0x00002000;	// Clear IC3 interrupt flag
	IEC0SET = 0x00002000;	// Enable IC3 interrupts
	IC3CON = 0x00000206;	// Disable and reset the IC Module, disable interrupt generation and allow SFR modifications;
							// simple Capture Event mode, capture rising edge first, count source is 16-bit Timer3
							// interrupt on every capture event
	/** Start **/
	IC1CONSET = 0x8000;		// start Input Capture 1 (IC1)
	IC3CONSET = 0x8000;		// start Input Capture 3 (IC3)
}

/** Get distance **/
#pragma interrupt IC1_ISR ipl4 vector 5
void IC1_ISR(void) {
	if (!IC1begin) TMR3 = 0;
	else{
		distance1 = 0.0;
		unsigned int time = IC1BUF;
		distance1 = time * 0.00017;
	}
	IC1begin  = 1 - IC1begin;
	IFS0CLR = 0x00000020;
}

#pragma interrupt IC3_ISR ipl4 vector 13
void IC3_ISR(void) {
	if (!IC3begin) TMR3 = 0;
	else{
		distance2 = 0.0;
		unsigned int time = IC3BUF;
		distance2 = time * 0.00017;
	}
	IC3begin  = 1 - IC3begin;
	IFS0CLR = 0x00002000;
}

int main() {
	OSCSetPBDIV(OSC_PB_DIV_1);	// Configure PBDIV so PBCLK = SYSCLK
	INTCONbits.MVEC = 1;		// Enable multiple vector interrupt
	asm ("ei");					// Enable all interrupts
	Config_UltrasonicDistDetectors();
	Config_ICs();
	Config_Timer3();
	Config_Timer5();

	while(1);
}
