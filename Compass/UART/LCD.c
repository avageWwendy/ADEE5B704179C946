#include <plib.h>
#include "LCD.h"

short T3_flag = 0;

/* initialize the PIC32 MCU */
void MCU_init() {
	/* setup I/O ports to connect to the LCD module */
	LATECLR = 0xFFFF;		// Clear the outputs before configuring the IO pins as outputs
	TRISECLR = 0x00FF;		// Set RE3-RE0 to be DB7-DB4
	LATDCLR = 0xFFFF;		// Clear the outputs before configuring the IO pins as outputs
	TRISDCLR = 0x0003;		// Set RD1, RD0 to be RS, E
	/* setup Timer and Timer interrupts to count for 1 us and 1 ms */
	Config_UsecTimer();
	Config_MsecTimer();
	initIntGlobal();
}

/* Configure interrupt globally */
void initIntGlobal() {
	INTCONbits.MVEC = 1;	// Enable multiple vector interrupt
	asm("ei");				// Enable all interrupts
}

/* initialize the LCD module */
void LCD_init() {
	DelayMsec(15);		//wait for 15 ms
	RS = 0;				//send command
	/*Data = LCD_IDLE;	//function set - 8 bit interface
	DelayMsec(5);		//wait for 5 ms
	Data = LCD_IDLE;	//function set - 8 bit interface
	DelayUsec(100);		//wait for 100 us
	Data = LCD_IDLE;	//function set - 8 bit interface
	DelayMsec(5);
	Data = LCD_IDLE;	//function set - 8 bit interface
	DelayUsec(100);*/
	LCD_putchar(LCD_2_LINE_4_BITS);
	DelayUsec(40);
	LCD_putchar(LCD_DSP_CSR);
	DelayUsec(40);
	LCD_putchar(LCD_CLR_DSP);
	DelayMsec(5);
	LCD_putchar(LCD_CSR_INC);
	DelayMsec(5);
}

/* Send one byte c (instruction or data) to the LCD */
void LCD_putchar(uchar c) {
	E = 1;
	Data = c;		//sending higher nibble
	E = 0;			//producing falling edge on E
	E = 1;
	Data <<= 4;		//sending lower nibble through higher 4 ports
	E = 0;			//producing falling edge on E
}

/* Display a string of characters *s by continuously calling LCD_putchar() */
void LCD_puts(const uchar *s) {
	RS = 1;
	int i = 0;
	while(s[i]){
		LCD_putchar(s[i]);
		DelayUsec(40);
		i++;
	}
}

/* go to a specific DDRAM address addr */
void LCD_goto(uchar addr) {
	RS = 0;
	LCD_putchar(addr | 0x80);
	DelayUsec(40);
}

/* configure timer SFRs to generate num us delay*/
void Config_UsecTimer(){
	IFS0CLR = 0x00000100;	//Clear Timer interrupt flag
	IEC0SET = 0x00000100;	//Enable Timer interrupts

	// 16-bit timer, 1 usec, SYSCLK = 80 MHz, PBCLK = SYSCLK/8
	T2CON = 0x0000; 	// stop Timer2 and clear control register
						// configure Timer2 as 16-bit timer,
						// set prescaler as 1:1, internal clock source
	TMR2 = 0x0; 		// clear timer register
	PR2 = 0x0027;		// load period register
}

void GenUsec() {
	T2CONSET = 0x8000;		// start Timer2
	while(!IFS0bits.T2IF);
	IFS0bits.T2IF = 0;
	T2CONCLR = 0x8000;  	// Stop Timer2
	TMR2 = 0x0;				// Reset Timer2 count
}

/* Call GenUsec() num times to generate num us delay*/
void DelayUsec(uchar num) {
	uchar i;
	for (i=0; i<num; i++) {
		GenUsec();
	}
}

/* configure timer SFRs to generate 1 ms delay*/
void Config_MsecTimer(){
	IPC3SET = 0x0000000C;	//Set priority level = 3
	IPC3SET = 0x00000001;	//Set subpriority level = 1
	IFS0CLR = 0x00001000;	//Clear Timer interrupt flag
	IEC0SET = 0x00001000;	//Enable Timer interrupts

	// 16-bit timer, 1 msec, SYSCLK = 8 MHz, PBCLK = SYSCLK/8
	T3CON = 0x0020; 	// stop Timer3 and clear control register
						// configure Timer3 as 16-bit timer,
						// set prescaler as 1:4, internal clock source
	TMR3 = 0x0; 		// clear timer register
	PR3 = 0x4E1F;		// load period register
}

void GenMsec() {
	T3_flag = 0;
	T3CONSET = 0x8000; 	// start Timer3
	while(!T3_flag);
}

/* Call GenMsec() num times to generate num ms delay*/
void DelayMsec(uchar num) {
	uchar i;
	for (i=0; i<num; i++) {
		GenMsec();
	}
}

/* timer interrupt handler */
#pragma interrupt Timer3_ISR ipl3 vector 12
void Timer3_ISR(void) {
	T3CONCLR = 0x8000;  	// Stop Timer3
	TMR3 = 0x0;				// Reset Timer3 count
	IFS0CLR = 0x00001000;	// Clear Timer3 interrupt flag
	T3_flag = 1;
}
