#include <p32xxxx.h>
#include <plib.h>

const unsigned short data[16] = {0x00, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F};
int data_num = 0;

#define LED_SERIAL	PORTDbits.RD4
#define LED_SRCLK	PORTDbits.RD5
#define LED_RCLK	PORTDbits.RD6
#define LED_EN		PORTDbits.RD7

void Config_LED(){
	PORTDCLR = 0xFFFF;	// Clear all PORTD
	TRISDCLR = 0x00F0;	// bit = 0 output, bit = 1 input
						// PORTD[4] = shift reg serial input
						// PORTD[5] = shift reg CLK
						// PORTD[6] = reg CLK
						// PORTD[7] = OE'
	LED_EN = 1;			// Stop display LED
}

void shift_LED(){ 
	int k;
	data_num = (data_num+1)%16;
	for (k = 0; k < 16; k++) {
		LED_SRCLK = 0;				// shift reg CLK = 0
		LED_SERIAL = ((data[data_num] >> k) & 0x0001);	// shift data into reg
		LED_SRCLK = 1;				// shift reg CLK = 1
	}
}

void Config_Timer4() {
	IPC4SET = 0x0000000C;	// Set priority level = 3
	IPC4SET = 0x00000001;	// Set subpriority level = 1
	IFS0CLR = 0x00010000;	// Clear Timer 4 interrupt flag
	IEC0SET = 0x00010000;	// Enable Timer 4 interrupts

	// 16-bit timer, 1s, SYSCLK = 4 MHz, PBCLK = SYSCLK/1
	T4CON = 0x0060; 	// stop Timer 4 and clear control register, 16-bit timer
						// set prescaler as 1:64, internal clock source
	TMR4 = 0x0; 		// clear timer register
	PR4 = 0xF423;		// load period register, 1 s
	T4CONSET = 0x8000;	// start Timer 4
}

#pragma interrupt T4_ISR ipl3 vector 16
void T4_ISR() {
	LED_EN = 1;				// Stop display LED
	LED_RCLK = 0;
	shift_LED();			// Load data to shift reg
	LED_RCLK = 1;			// Load data to reg
	LED_EN = 0;				// Display LED
	IFS0CLR = 0x00010000; 	// Clear Timer 4 interrupt flag
}

int main() {
	OSCSetPBDIV(OSC_PB_DIV_1);	// Configure PBDIV so PBCLK = SYSCLK
	INTCONbits.MVEC = 1;		// Enable multiple vector interrupt
	asm ("ei");					// Enable all interrupts
	Config_LED();
	Config_Timer4();

	while(1);
}
