#include <p32xxxx.h>
#include <plib.h>

// Data
/* Stars Data */
const unsigned short StarData[12][72] = {{0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0020, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
					0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
					0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
					0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0200, 0x0000, 0x0000, 0x0200, 0x0000,
					0x0000, 0x0000, 0x0000, 0x0400, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
					0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
					{0},
					{0},
					{0},
					{0},
					{0},
					{0},
					{0},
					{0},
					{0},
					{0},
					{0}};
const unsigned short BackData[12][72] = {{0x7800, 0x7800, 0x7800, 0x7800, 0x7800, 0x7800, 0x7F00, 0x7D00, 0x7E00, 0x7F00, 0x3D80, 0x1F00,
					0x1F00, 0x0E00, 0x0600, 0x0A00, 0x0A00, 0x0800, 0x1200, 0x0A00, 0x0A00, 0x0400, 0x0000, 0x0000,
					0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
					0x0000, 0x0000, 0x0000, 0x0000, 0x1C62, 0x3F5E, 0x3F00, 0x1F00, 0x6F40, 0x6F40, 0x7740, 0x7700,
					0x7F00, 0x7F00, 0x7D40, 0x7D60, 0x7B60, 0x7A60, 0x7B20, 0x7900, 0x7840, 0x7840, 0x7E20, 0x7E00,
					0x7F00, 0x7F40, 0x7E40, 0x7E60, 0x7E20, 0x7F00, 0x7D40, 0x787E, 0x7801, 0x7800, 0x7800, 0x7800},
					{0},
					{0},
					{0},
					{0},
					{0},
					{0},
					{0},
					{0},
					{0},
					{0},
					{0}};

unsigned int Constellation = 0;
#define Aries_index			0
#define Cassiopeia_index	0
#define Perseus_index		1
#define Auriga_index		2
#define Gemini_index		3
#define CanisMinor_index	4
#define Pyxis_index			5
#define Antlia_index		6
#define Centaurus_index		7
#define Scorpio_index		8
#define Ophiuchus_index		9
#define Aquila_index		10
#define Cygnus_index		11
#define Cepheus_index		12

unsigned short Star[72] = {0};
unsigned short Back[72] = {0};


// Function
void Config_DMA(){
	asm("di");
	IEC1CLR = 0x00030000;	// Disable DMA channel 0&1 interrupts
	IFS1CLR = 0x00030000;	// Clear DMA channel 0&1 interrupt flag
	DMACONSET = 0x00008000;	// Enable DMA module (controller) <ON>
	DCH0CON = 0x0003;		// Channel 0 disabled <CHEN>
							// Do not allow channel to be chained <CHCHN>
							// Channel priority 3 <CHPRI>
	DCH0ECON = 0x001B0010;	// UART1RX interrupt initiate a DMA transfer <CHSIRQ>
							// Enable channel start IRQ <SIRQEN>
							// Disable pattern match <PATEN>
	DCH1CON = 0x0003;		// Channel 1 disabled <CHEN>
							// Do not allow channel to be chained <CHCHN>
							// Channel priority 3 <CHPRI>
	DCH1ECON = 0x001B0010;	// UART1RX interrupt initiate a DMA transfer <CHSIRQ>
							// Enable channel start IRQ <SIRQEN>
							// Disable pattern match <PATEN>
	/********* Set transfer configurations *********/
	DCH0SSA = KVA_TO_PA(StarData[Constellation]);		// Source address (physical address)
	DCH0DSA = KVA_TO_PA(Star);		// Destination address (physical address)
	DCH0SSIZ = 144;					// Source size: 144 bytes
	DCH0DSIZ = 144;					// Destination size: 144 byte
	DCH0CSIZ = 144;					// Cell size: 144 byte
									// Note if cell transferred: CHCCIF = 1
	DCH0INT = 0x00000000;			// Disable all interrupts
									// Clear all interrupt flags (existing events)
	DCH1SSA = KVA_TO_PA(BackData[Constellation]);		// Source address (physical address)
	DCH1DSA = KVA_TO_PA(Back);		// Destination address (physical address)
	DCH1SSIZ = 144;					// Source size: 144 bytes
	DCH1DSIZ = 144;					// Destination size: 144 byte
	DCH1CSIZ = 144;					// Cell size: 144 byte
									// Note if cell transferred: CHCCIF = 1
	DCH1INT = 0x00000000;			// Disable all interrupts
									// Clear all interrupt flags (existing events)

	/********* Initiate transfer *********/
	DCH0CONSET = 0x00080;		// Enable channel 0 <CHEN>
	DCH1CONSET = 0x00080;		// Enable channel 1 <CHEN>
	asm("ei");
	// IEC1SET = 0x00010000;		// Enable DMA channel 0 interrupts
	// DCH0ECONSET = 0x00000080;	// Set CFORCE to 1 to trigger a cell transfer
}

int main() {
	OSCSetPBDIV(OSC_PB_DIV_1);	//configure PBDIV so PBCLK = SYSCLK
	INTCONbits.MVEC = 1;	// Enable multiple vector interrupt
	asm("ei");				// Enable all interrupts
	Config_DMA();

	while (1);
}


