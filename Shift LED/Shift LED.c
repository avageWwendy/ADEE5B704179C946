#include <plib.h>
#include <p32xxxx.h>

const unsigned char data = 0b00101101;

void ini_LED_IOports(){
	PORTDCLR = 0xFFFF;	// Clear all PORTD
	TRISD = 0x0000;		// bit = 0 output, bit = 1 input
						// PORTD[0] = shift reg input
						// PORTD[1] = asychronize active-low CLR
						// PORTD[2] = clk
}

void shift_LED(){ 
	int k = 0;
	for (k = 0; k < 8; k++) {
		PORTDbits.RD2 = 0;	// clk = 0
		PORTDbits.RD0 = (data << k);	// shift data into reg
		PORTDbits.RD2 = 1;	// clk = 1
	}
}

void Config_Timer1() {
	IPC6SET = 0x00000018;	// Set priority level = 6
	IPC6SET = 0x00010001;	// Set subpriority level = 1
	IFS0CLR = 0x00000010;	//Clear Timer1 interrupt flag
	IEC0SET = 0x00000010;	//Enable Timer1 interrupts

	// 16-bit timer, SYSCLK = 80 MHz, PBCLK = SYSCLK/1
	T1CON = 0x0000; 	// stop Timer1 and clear control register,
						// set prescaler as 1:1, internal clock source
	TMR1 = 0x0; 		// clear timer register
	PR1 = 0x66CE;		// load period register ??80000000/360/speed
	T1CONSET = 0x8000;	// start Timer1
}

#pragma interrupt T1_ISR ipl6 vector 4
void T1_ISR() {
	TMR1 = 0x0;
	shiftout();
	IFS0CLR = 0x00000010; 	//clear Timer1 interrupt flag
}

int main() {
	OSCSetPBDIV(OSC_PB_DIV_1);	//configure PBDIV so PBCLK = SYSCLK
	ini_LED_IOports();
	shift_LED();

	while(1);
}

/* test git */