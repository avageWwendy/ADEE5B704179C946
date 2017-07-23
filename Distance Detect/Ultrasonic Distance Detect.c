#include <p32xxxx.h>
#include <plib.h>

#define Ultrasonic_ECHO	PORTDbits.RD8
#define Ultrasonic_TRIG	PORTDbits.RD9

unsigned int distance; // unit: m

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
	Ultrasonic_TRIG = 1;	// Generate 10 us pulse signal to trigger Ultrasonic Distance Detector
	unsigned int tStart = ReadCoreTimer();
	while((ReadCoreTimer()-tStart)<40);
	Ultrasonic_TRIG = 0;
	IFS0CLR = 0x00100000;	// Clear Timer 5 interrupt flag
}

void Config_UltrasonicDistDetector(){
	PORTDCLR = 0x0300;	// clear RD[8], RD[9]
	TRISDCLR = 0x0300;	// set RD[8] = TRIG
						// set RD[9] = ECHO
}

void Config_IC(){
	IPC1SET = 0x00001000;	// Set priority level = 4
	IPC1SET = 0x00000100;	// Set subpriority level = 1
	IFS0CLR = 0x00000020;	// Clear IC1 interrupt flag
	IEC0SET = 0x00000020;	// Enable IC1 interrupts
	IC1CON = 0x00000206;	// Disable and reset the IC Module, disable interrupt generation and allow SFR modifications;
							// simple Capture Event mode, capture rising edge first
							// interrupt on every capture event
	IC1CONSET = 0x8000;		// start Input Capture 1 (IC1)
}

/** Get distance **/
#pragma interrupt IC1_ISR ipl4 vector 5
void IC1_ISR(void) {
	unsigned int time = IC1BUF;
	distance = time * 170;
	IFS0CLR = 0x00000020;
}

int main() {
	OSCSetPBDIV(OSC_PB_DIV_1);	// Configure PBDIV so PBCLK = SYSCLK
	INTCONbits.MVEC = 1;		// Enable multiple vector interrupt
	asm ("ei");					// Enable all interrupts
	Config_UltrasonicDistDetector();
	Config_IC();
	Config_Timer5();

	while(1);
}
