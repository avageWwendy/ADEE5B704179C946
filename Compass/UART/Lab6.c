#include <p32xxxx.h>
#include <plib.h>
#include "LCD.h"

const unsigned char startStr[] = "Received Letter";
const unsigned char data = 'W';
unsigned char str[1];
short readCN = 0;

void Config_UART1A_TXRX(){
	U1ABRG = 832;
	U1AMODE = 0x0008;	// UART1A is disabled <ON>
						// UxRTS pin is in Simplex mode <RTSMD> <UEN>??
						// High-Speed mode <BRGH>
						// 8-bit data, no parity <PDSEL>
						// 1 Stop bit <STSEL>
	/********* Set transmission interrupt configurations *********/
	U1ASTAbits.UTXSEL = 0b00;	// Generate interrupt when the transmit buffer contains at least one empty space <UTXISEL>
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
		while (DCH0INTbits.CHCCIF != 1);	// Wait until DMA successfully copied data (one cell)
		DCH0INTbits.CHCCIF = 0;				// Clear DMA Channel 0 cell transfer complete interrupt flag
		U1ASTACLR = 0x0400;					// Disable transmission <UTXEN>
		IFS0CLR = 0x10000000;				// Clear UART1 Transmitter interrupt flag
	}
	else{
		LCD_goto(0x40);
		short received = U1ARXREG;
		sprintf(str, "%c", received);
		LCD_puts(str);
		IFS0CLR = 0x08000000;				// Clear UART1 Receiver interrupt flag
	}
}

void Config_DMA(){
	IEC1CLR = 0x00010000;	// Disable DMA channel 0 interrupts
	IFS1CLR = 0x00010000;	// Clear DMA channel 0 interrupt flag
	DMACONSET = 0x00008000;	// Enable DMA module (controller) <ON>
	DCH0CON = 0x0003;		// Channel 0 disabled <CHEN>
							// Do not allow channel to be chained <CHCHN>
							// Channel priority 3 <CHPRI>
	DCH0ECON = 0x001C0010;	// UART1TX interrupt initiate a DMA transfer <CHSIRQ>
							// Enable channel start IRQ <SIRQEN>
							// Disable pattern match <PATEN>
	/********* Set transfer configurations *********/
	DCH0SSA = KVA_TO_PA(&data);		// Source address (physical address)
	DCH0DSA = KVA_TO_PA(&U1ATXREG);	// Destination address (physical address)
	DCH0SSIZ = 1;					// Source size: 1 bytes
	DCH0DSIZ = 1;					// Destination size: 1 byte
	DCH0CSIZ = 1;					// Cell size: 1 byte
									// Note if cell transferred: CHCCIF = 1
	DCH0INT = 0x00000000;			// Disable all interrupts
									// Clear all interrupt flags (existing events)
	/********* Initiate transfer *********/
	DCH0CONSET = 0x00080;			// Enable channel 0 <CHEN>
	// IEC1SET = 0x00010000;		// Enable DMA channel 0 interrupts
	// DCH0ECONSET = 0x00000080;	// Set CFORCE to 1 to trigger a cell transfer
}

void Config_CN()
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
		U1ASTASET = 0x0400;		// Enable transmission <UTXEN>
	}
	IFS1CLR = 0x0001;			// Clear interrupt flag
	IEC1SET = 0x0001;			// enable interrupt
}

int main() {
	OSCSetPBDIV(OSC_PB_DIV_1);	//configure PBDIV so PBCLK = SYSCLK

	Config_UART1A_TXRX();
	Config_DMA();
	MCU_init();
	LCD_init();
	LCD_goto(0x00);
	LCD_puts(startStr);
	Config_CN();
	TRISDSET = 0x0040;

	readCN = PORTDbits.RD6;
	while (1);
}
