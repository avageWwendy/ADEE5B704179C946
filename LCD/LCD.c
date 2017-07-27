#include <p32xxxx.h>
#include <plib.h>

#define LCD_RS 		PORTDbits.RD12
#define LCD_E 		PORTDbits.RD13
#define LCD_Data 	PORTE

/* define macros for LCD instructions */
#define LCD_IDLE 			0x33
#define LCD_2_LINE_4_BITS	0x28
#define LCD_DSP_CSR 		0x0c
#define LCD_CLR_DSP 		0x01
#define LCD_CSR_INC 		0x06
#define LCD_SFT_MOV 		0x14

unsigned int tStart;

/* Send one byte c (instruction or data) to the LCD */
void LCD_putchar(unsigned char c) {
	LCD_E = 1;
	LCD_Data = c;		//sending higher nibble
	LCD_E = 0;			//producing falling edge on E
	LCD_E = 1;
	LCD_Data <<= 4;		//sending lower nibble through higher 4 ports
	LCD_E = 0;			//producing falling edge on E
}

/* Display a string of characters *s by continuously calling LCD_putchar() */
void LCD_puts(const unsigned char *s) {
	LCD_RS = 1;
	int i = 0;
	while(s[i]){
		LCD_putchar(s[i]);
		tStart = ReadCoreTimer();
		while((ReadCoreTimer()-tStart)<160);	// delay 40 us
		i++;
	}
}

/* go to a specific DDRAM address addr */
void LCD_goto(unsigned char addr) {
	LCD_RS = 0;
	LCD_putchar(addr | 0x80);
	tStart = ReadCoreTimer();
	while((ReadCoreTimer()-tStart)<160);	// delay 40 us
}

void Config_LCD() {
	/* setup I/O ports to connect to the LCD module */
	PORTECLR = 0x00FF;		// Clear the outputs before configuring the IO pins as outputs
	TRISECLR = 0x00FF;		// Set RE3-RE0 to be DB7-DB4
	PORTDCLR = 0x3000;		// Clear the outputs before configuring the IO pins as outputs
	TRISDCLR = 0x3000;		// Set RD[12] = LCD_RS
							// Set RD[13] = LCD_E
	/** Initial LCD module **/
	tStart = ReadCoreTimer();
	while((ReadCoreTimer()-tStart)<60000);	// wait for 15 ms
	LCD_RS = 0;				// send command
	/*LCD_Data = LCD_IDLE;	//function set - 8 bit interface
	DelayMsec(5);			//wait for 5 ms
	LCD_Data = LCD_IDLE;	//function set - 8 bit interface
	DelayUsec(100);			//wait for 100 us
	LCD_Data = LCD_IDLE;	//function set - 8 bit interface
	DelayMsec(5);
	LCD_Data = LCD_IDLE;	//function set - 8 bit interface
	DelayUsec(100);*/
	LCD_putchar(LCD_2_LINE_4_BITS);
	tStart = ReadCoreTimer();
	while((ReadCoreTimer()-tStart)<160);	// delay 40 us
	LCD_putchar(LCD_DSP_CSR);
	tStart = ReadCoreTimer();
	while((ReadCoreTimer()-tStart)<160);	// delay 40 us
	LCD_putchar(LCD_CLR_DSP);
	tStart = ReadCoreTimer();
	while((ReadCoreTimer()-tStart)<20000);	// delay 5 ms
	LCD_putchar(LCD_CSR_INC);
	tStart = ReadCoreTimer();
	while((ReadCoreTimer()-tStart)<20000);	// delay 5 ms
}

int main(){
	Config_LCD();

	while(1);
}
