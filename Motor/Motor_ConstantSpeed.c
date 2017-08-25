#include <p32xxxx.h>
#include <plib.h>

#define Motor_PWM	PORTDbits.RD0
#define Motor_Left	PORTDbits.RD1
#define Motor_Right	PORTDbits.RD2

#define speed	0xC8	// B8 least speed
unsigned short pwm = speed;

void Config_PWM(){
	OC1CON = 0x0000;
	OC1RS = 0x00;
	OC1R = 0x00;
	OC1CON = 0x0006;	// OC1 use 16-bit Timer2
	PR2 = 0x00FF;
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

int main(){
	OSCSetPBDIV(OSC_PB_DIV_1);	// Configure PBDIV so PBCLK = SYSCLK
	INTCONbits.MVEC = 1;
	asm("ei");
	Config_PWM();
	Config_Motor();
	//Motor_TurnLeft();
	//Motor_TurnRight();	

	unsigned int tStart;
	while(1){
		Motor_TurnLeft();
		tStart = ReadCoreTimer();
		while ((ReadCoreTimer()-tStart)< 20000000);	// 10s
		Motor_Stop();
		tStart = ReadCoreTimer();
		while ((ReadCoreTimer()-tStart)< 20000000);	// 10s
		Motor_TurnRight();
		tStart = ReadCoreTimer();
		while ((ReadCoreTimer()-tStart)< 20000000);	// 10s
	}
	
	while(1);
}
