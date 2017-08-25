#include <p32xxxx.h>
#include <plib.h>

#define Motor_PWM			PORTDbits.RD0
#define Motor_Left			PORTDbits.RD1
#define Motor_Right			PORTDbits.RD2
#define Ultrasonic1_TRIG	PORTDbits.RD8
#define Ultrasonic1_ECHO	PORTDbits.RD9
#define Ultrasonic2_TRIG	PORTDbits.RD10
#define Ultrasonic2_ECHO	PORTDbits.RD11

// Motor
#define speed	0x6000	// B800 least speed
unsigned short pwm = speed;

void Config_PWM();
void Config_Motor();
void Motor_Stop();
void Motor_TurnLeft();
void Motor_TurnRight();

// Distant Detector
float distance1 = 0.0;		// unit: m
float distance2 = 0.0;		// unit: m
unsigned char detector = 0x1;	// choose which detector
void Config_UltrasonicDistDetectors();
void Config_Timer3();
void Config_Timer5();
void Config_ICs();
BOOL User_IsNear();
BOOL User_GoDetector1();
BOOL User_GoDetector2();

// Motor
void Config_PWM(){
	OC1CON = 0x0000;
	OC1RS = 0x0000;
	OC1R = 0x0000;
	OC1CON = 0x0006;		// OC1 use 16-bit Timer2
	T2CONSET = 0x0050;		// Timer 2 Prescale 1:32
	PR2 = 0xFFFF;
	// Set Timer 2 interrupts
	IPC2SET = 0x0000000C;	// Set priority level = 3
	IPC2SET = 0x00000001;	// Set subpriority level = 1
	IFS0CLR = 0x00000100;	// Clear interrupt flag
	IEC0SET = 0x00000100;	// Enable Timer 2 interrupts
	// Enable Timer2 and OC1/RD0 module
	T2CONSET = 0x8000;
	OC1CONSET = 0x8000;
}

#pragma interrupt T2_PWM_ISR ipl3 vector 8
void T2_PWM_ISR (void){
	if (User_GoDetector1()) Motor_TurnLeft();
	else if (User_GoDetector2()) Motor_TurnRight();
	else if (User_IsNear()) Motor_Stop();
	else Motor_TurnRight();
	OC1RS = pwm;
	IFS0CLR = 0x0100;
}

void Config_Motor(){
	LATDCLR = 0x0006;		// Clear the outputs before configuring the IO pins as outputs
	TRISDCLR = 0x0006;		// set RD[1] = Motor_Left
							// set RD[2] = Motor_Right
}

void Motor_Stop(){
	//Motor_Left = 1;
	//Motor_Right = 1;
	pwm = 0x0;	// speed = 0x0;
}

void Motor_TurnLeft(){
	Motor_Left = 1;
	Motor_Right = 0;
	pwm = speed;
}

void Motor_TurnRight(){
	Motor_Left = 0;
	Motor_Right = 1;
	pwm = speed;
}

// Distant Detector
void Config_UltrasonicDistDetectors() {
	LATDCLR = 0x0F00;	// clear RD[8], RD[9], RD[10], RD[11]
	TRISDSET = 0x0A00;	// set RD[8] = TRIG1
						// set RD[10] = TRIG2
	TRISDCLR = 0x0500;	// set RD[9] = ECHO1
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

void Config_Timer5() {
	IPC5SET = 0x0000000C;	// Set priority level = 3
	IPC5SET = 0x00000001;	// Set subpriority level = 1
	IFS0CLR = 0x00100000;	// Clear Timer 5 interrupt flag
	IEC0SET = 0x00100000;	// Enable Timer 5 interrupts

	// 16-bit timer, 100ms, SYSCLK = 4 MHz, PBCLK = SYSCLK/1
	T5CON = 0x0060; 	// stop Timer 5 and clear control register, 16-bit timer
						// set prescaler as 1:64, internal clock source
	TMR5 = 0x0; 		// clear timer register
	PR5 = 0xFFFF;
	//PR5 = 0x1869;		// load period register, 100ms
	T5CONSET = 0x8000;	// start Timer 5
}

#pragma interrupt T5_ISR ipl3 vector 20
void T5_ISR() {
	if (detector == 0x1){
		Ultrasonic1_ECHO = 0;
		Ultrasonic1_TRIG = 1;	// Generate 15 us pulse signal to trigger Ultrasonic Distance Detector 1
		unsigned int tStart = ReadCoreTimer();
		while((ReadCoreTimer()-tStart)<60);
		Ultrasonic1_TRIG = 0;
		IC2CONSET = 0x8000;		// start Input Capture 2 (IC2)
		detector = 0x2;
	}
	else if (detector == 0x2){
		Ultrasonic2_ECHO = 0;
		Ultrasonic2_TRIG = 1;	// Generate 15 us pulse signal to trigger Ultrasonic Distance Detector 2
		unsigned int tStart = ReadCoreTimer();
		while((ReadCoreTimer()-tStart)<60);
		Ultrasonic2_TRIG = 0;
		IC4CONSET = 0x8000;		// start Input Capture 4 (IC4)
		detector == 0x1;
	} 
	IFS0CLR = 0x00100000;	// Clear Timer 5 interrupt flag
}

void Config_ICs() {
	/** IC2/RD9 **/
	IPC2SET = 0x00001000;	// Set priority level = 4
	IPC2SET = 0x00000100;	// Set subpriority level = 1
	IFS0CLR = 0x00000200;	// Clear IC2 interrupt flag
	IEC0SET = 0x00000200;	// Enable IC2 interrupts
	IC2CON = 0x00000206;	// Disable and reset the IC Module, disable interrupt generation and allow SFR modifications;
							// simple Capture Event mode, capture rising edge first, count source is 16-bit Timer3
							// interrupt on every capture event
	/** IC4/RD11 **/
	IPC4SET = 0x00001000;	// Set priority level = 4
	IPC4SET = 0x00000100;	// Set subpriority level = 1
	IFS0CLR = 0x00020000;	// Clear IC4 interrupt flag
	IEC0SET = 0x00020000;	// Enable IC4 interrupts
	IC4CON = 0x00000206;	// Disable and reset the IC Module, disable interrupt generation and allow SFR modifications;
							// simple Capture Event mode, capture rising edge first, count source is 16-bit Timer3
							// interrupt on every capture event
}

/** Get distance **/
#pragma interrupt IC2_ISR ipl4 vector 9
void IC2_ISR(void) {
	if (Ultrasonic1_ECHO){
		TMR3 = 0x0;
		unsigned int time1 = IC2BUF;
	}
	else{
		unsigned int time1 = IC2BUF;
		distance1 = time1 * 0.00017;
		IC2CONCLR = 0x8000;		// stop Input Capture 2 (IC2)
	}
	IFS0CLR = 0x00000200;
}

#pragma interrupt IC4_ISR ipl4 vector 17
void IC4_ISR(void) {
	if (Ultrasonic2_ECHO){
		TMR3 = 0x0;
		unsigned int time2 = IC4BUF;
	}
	else{
		unsigned int time2 = IC4BUF;
		distance2 = time2 * 0.00017;
		IC4CONCLR = 0x8000;		// stop Input Capture 2 (IC2)
	}
	IFS0CLR = 0x00020000;
}

BOOL User_IsNear() {
	return (distance1 < 0.6 || distance2 < 0.6);
}

BOOL User_GoDetector1() {
	return (User_IsNear() && (distance2 - distance1 > 0.2));
}

BOOL User_GoDetector2() {
	return (User_IsNear() && (distance1 - distance2 > 0.2));
}

int main(){
	OSCSetPBDIV(OSC_PB_DIV_1);	// Configure PBDIV so PBCLK = SYSCLK
	INTCONbits.MVEC = 1;
	asm("ei");
	Config_PWM();
	Config_Motor();
	Config_UltrasonicDistDetectors();
	Config_Timer3();
	Config_Timer5();
	Config_ICs();

	while(1);
}
