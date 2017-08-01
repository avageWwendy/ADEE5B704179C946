#include <p32xxxx.h>
#include <plib.h>
#include "LCD.h"

const unsigned char startStr[] = "Received Letter";
unsigned char str[1];
//short readCN = 0;
unsigned char letter = 'f';
unsigned char flag = 1;


/* Compass */
typedef struct{
	unsigned short x, y, z;
} my_QMC5883L_Data;

my_QMC5883L_Data Compass_current_val, Compass_ready_val;
unsigned char Compass_ready = 0; /*set when at Compass_ready_val has a valid value*/

void Compass_Receive_Data(char ReceivedData){
	/* parameter ReceivedData: expect value of U1ARXREG */
	/* this function will store received data into Compass_current
		and copy the data to Compass_ready_val when done.
	 */
	static char stage = 0;
	/*
		0: not started, waiting for 'x'
			0->1: 'x' received
		1: 'x' received, waiting for xLSB
		2: waiting for xMSB
		3: waiting for 'y'
			3->4: 'y' received
			3->0: not 'y'
		4: 'y' received, waiting for yLSB
		5: waiting for yMSB
		6: waiting for 'z'
			6->7: 'z' received
			6->0: not 'z'
		7: 'z' received, waiting for zLSB
		8: waiting for zMSB, and calculate Compass_ready_val, set Compass_ready
			8->0
	*/
	switch (stage){
		case 0: 
			if (ReceivedData == 'x') stage = 1;
			else return;
			break;
		case 1:
			Compass_current_val.x = Compass_current_val.y = Compass_current_val.z = 0;
			Compass_current_val.x = ReceivedData; // xLSB
			stage = 2; break;
		case 2:
			Compass_current_val.x |= (ReceivedData << 8); // xMSB
			stage = 3; break;
		case 3:
			if (ReceivedData == 'y') stage = 4;
			else stage = 0;
			break;
		case 4:
			Compass_current_val.y = ReceivedData; // yLSB
			stage = 5; break;
		case 5:
			Compass_current_val.y |= (ReceivedData << 8); // yMSB
			stage = 6; break;
		case 6:
			if (ReceivedData == 'z') stage = 7;
			else stage = 0;
			break;	
		case 7:
			Compass_current_val.z = ReceivedData; // zLSB
			stage = 8; break;
		case 8:
			Compass_current_val.z |= (ReceivedData << 8); // zMSB
			Compass_ready_val.x = Compass_current_val.x;
			Compass_ready_val.y = Compass_current_val.y;
			Compass_ready_val.z = Compass_current_val.z;
			Compass_ready += 1;
			stage = 0; break;	
		default:
			stage = 0;
	}

}


void Config_UART1A_TXRX(){
	// U1ABRG = 832;	// 8MHz, baud rate = 2400
	U1ABRG = 416;		// 4MHz, baud rate = 2400
	U1AMODE = 0x0008;	// UART1A is disabled <ON>
						// UxRTS pin is in Simplex mode <RTSMD> <UEN>??
						// High-Speed mode <BRGH>
						// 8-bit data, no parity <PDSEL>
						// 1 Stop bit <STSEL>
	/********* Set interrupt configurations *********/
	U1ASTAbits.UTXSEL = 0b10;	// Generate interrupt when the transmit buffer is empty <UTXISEL>
	IPC6SET = 0x0000000C;		// Set priority level = 3
	IPC6SET = 0x00000001;		// Set subpriority level = 1
	IFS0CLR = 0x10000000;		// Clear UART1 Transmitter interrupt flag
	IEC0SET = 0x10000000;		// Enable UART1 Transmitter interrupts
	/********* Set receiver interrupt configurations *********/
	U1ASTAbits.URXISEL = 0b00;	// Generate interrupt when a character is received <URXISEL>
	IPC6SET = 0x0000000C;		// Set priority level = 3
	IPC6SET = 0x00000001;		// Set subpriority level = 1
	IFS0CLR = 0x08000000;		// Clear UART1 Receiver interrupt flag
	IEC0SET = 0x08000000;		// Enable UART1 Receiver interrupts
	/********* Enable transmission & receiving & UART module *********/
	U1AMODESET = 0x8000;	// Enable UART module <ON>
	U1ASTASET = 0x1000;		// Enable receiving <URXEN>
}

#pragma interrupt UART1A_TXRX_ISR ipl3 vector 24
void UART1A_TXRX_ISR(void){
	if (IFS0bits.U1TXIF == 1){
		while(U1ASTAbits.TRMT == 0); 
		U1ASTACLR = 0x0400;					// Disable transmission <UTXEN>
		flag = 1;
		IFS0CLR = 0x10000000;				// Clear UART1 Transmitter interrupt flag
	}
	if (IFS0bits.U1RXIF == 1){
		LCD_goto(0x40);
		short received = U1ARXREG;
		Compass_Receive_Data(received);
		sprintf(str, "%c", received);
		LCD_puts(str);
		//U1ASTACLR = 0x0400;				// Disable transmission <UTXEN>
		IFS0CLR = 0x08000000;				// Clear UART1 Receiver interrupt flag
	}
}

/*void Config_CN()
{
	IPC6SET = 0x000C0000;	// Set priority level = 3
	IPC6SET = 0x00010000;	// Set subpriority level = 1
	IFS1CLR = 0x0001;		// Clear interrupt flag
	IEC1SET = 0x0001;		// Enable Change Notice interrupts
	CNCON = 0x8000;			// Enable Change Notice module
	CNEN = 0x00008000;		// Enable individual CN15/RD6 pin
}

#pragma interrupt CN_ISR ipl3 vector 26
void CN_ISR(void) {
	IEC1CLR = 0x0001;			// disable interrupt
	if (readCN != PORTDbits.RD6){
		U1ATXREG = 0x05;
	}
	IFS1CLR = 0x0001;			// Clear interrupt flag
	IEC1SET = 0x0001;			// enable interrupt
}*/

void iniPorts(){
	TRISDSET = 0x0080;
}

int main() {
	OSCSetPBDIV(OSC_PB_DIV_1);	//configure PBDIV so PBCLK = SYSCLK

	MCU_init();
	LCD_init();
	LCD_goto(0x00);
	LCD_puts(startStr);
	Config_UART1A_TXRX();
	//Config_CN();
	//readCN = PORTDbits.RD7;
	iniPorts();

	while(1){
		if(PORTDbits.RD7 == 0 && flag == 1){
			U1ATXREG = letter;
			flag = 0;
			U1ASTASET = 0x0400;		// Enable transmission <UTXEN>
			letter = letter + 1;
		}
	}
}
