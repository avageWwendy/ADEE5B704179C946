#include <p32xxxx.h>

#define uchar unsigned char

/* define macros for LCD instructions */
#define LCD_IDLE 0x33
//#define LCD_2_LINE_8_BITS 0x38
#define LCD_2_LINE_4_BITS 0x28
#define LCD_DSP_CSR 0x0c
#define LCD_CLR_DSP 0x01
#define LCD_CSR_INC 0x06
#define LCD_SFT_MOV 0x14

/* define macros for interfacing ports */
#define RS PORTDbits.RD0
#define E PORTDbits.RD1
#define Data PORTE

/* Function prototypes */
void initIntGlobal();
void MCU_init(void);
void LCD_init(void);
void LCD_putchar(uchar c);
void LCD_puts(const uchar *s);
void LCD_goto(uchar addr);
void GenMsec(void);
void Config_UsecTimer();
void DelayUsec(uchar num);
void Config_MsecTimer();
void DelayMsec(uchar num);
