/* 	LED Panel Header File
		
	Revision History:
	05.24.2003	RB	Created
	11.05.2003  	MR 	Modified form RB's
	
*/

#include <avr/eeprom.h>
#include "single_handler.h"
#include "i2c.h"
#include "legacy.h"

#define LED_ADDRESS_PORT PORTD
#define LED_ADDRESS_PORT_DIRECTION DDRD

#define LED_DATA_PORT_05 PORTB
#define LED_DATA_PORT_DIRECTION_05 DDRB

#define LED_DATA_PORT_67 PORTC
#define LED_DATA_PORT_DIRECTION_67 DDRC

#define MASK_05 0x3F   	//this is 00111111
#define MASK_67 0xC0	//this is 11000000

#define DD_WRITE 0xFF

//#define F_CPU    16000000      // 20MHz processor
//#define CYCLES_PER_US ((F_CPU+500000)/1000000)

#define EEPROM __attribute__((section(".eeprom")))

#define EEPAT_NUMS 2

#define RAM_PAT_NUMS 100
//used to be 100

void UpdateDisplay(void);
void DisplayChar(unsigned char c,unsigned char col);
void StorePattern(unsigned char patternNumber, unsigned char *pattern);
void LoadPattern_1byte(unsigned char pattern);
void LoadPattern_3byte(unsigned char *pattern);
void LoadPattern_4byte(unsigned char *pattern);
void LoadPattern(unsigned char* pattern_byte);
void LoadPattern16(unsigned char *pattern);
void LoadPattern24(unsigned char *pattern);
void LoadPattern32(unsigned char *pattern);
void LoadPatternEEP(unsigned char *pattern);
void delay(unsigned short us);
void long_delay(unsigned short ms); 
void SystemReset(void);
void DisplayNum(void);
void DisplayBusNum(uint8_t busNum);

unsigned char Panel_ID[1] EEPROM = {0x7F}; //change the initial address to 127 instead of 0
			
static unsigned char NUMS[10][4] EEPROM = { 	{0xFE, 0x82, 0xFE, 0x00},
						{0x84, 0xFE, 0x80, 0x00},					
						{0xF2, 0x92, 0x9E, 0x00},
						{0x92, 0x92, 0xFE, 0x00},
						{0x1E, 0x10, 0xFE, 0x00},
						{0x9E, 0x92, 0xF2, 0x00},
						{0xFE, 0x92, 0xF2, 0x00},
						{0x02, 0x02, 0xFE, 0x00},
						{0xFE, 0x92, 0xFE, 0x00},
						{0x1E, 0x12, 0xFE, 0x00}	};
						
						
//contains symbols used to display on panels, could put in here an alphabet, etc.
//pattern 0 is an 'X' used to signify non-initialized memory						
//patterns 1-8 are error messages, with a dot to signify which one.
static unsigned char SYMBOLS[9][8] EEPROM = {	{0xC3, 0xE7, 0x7E, 0x3C, 0x3C, 0x7E, 0xE7, 0xC3},
						{0xBE, 0x2A, 0x2E, 0x00, 0x00, 0x3E, 0x02, 0x06},
						{0x3E, 0xAA, 0x2E, 0x00, 0x00, 0x3E, 0x02, 0x06},
						{0x3E, 0x2A, 0xAE, 0x00, 0x00, 0x3E, 0x02, 0x06},
						{0x3E, 0x2A, 0x2E, 0x80, 0x00, 0x3E, 0x02, 0x06},
						{0x3E, 0x2A, 0x2E, 0x00, 0x80, 0x3E, 0x02, 0x06},
						{0x3E, 0x2A, 0x2E, 0x00, 0x00, 0xBE, 0x02, 0x06},
						{0x3E, 0x2A, 0x2E, 0x00, 0x00, 0x3E, 0x82, 0x06},
						{0x3E, 0x2A, 0x2E, 0x00, 0x00, 0x3E, 0x02, 0x86} };
						

						
						
/*	To store patterns in EEPROM, just append or replace the patterns in EEPATTERNS, 
	also change number of rows. EEPAT_START contains the starting row number for each of the 
	patterns, and EEPAT_LENGTH contains the length of each of these patterns. 
	Right now Pat1 is 2 column width bar patterns, and Pat2 is 4 column width bar patterns. */


static unsigned int EEPAT_START[EEPAT_NUMS]	= {0, 4};
static unsigned int EEPAT_LENGTH[EEPAT_NUMS] 	= {4, 8};

static unsigned char EEPATTERNS[12][8] EEPROM = {{0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00},
						{0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00},					
						{0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF},	
						{0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF},
						{0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00},
						{0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00},
						{0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00},
						{0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00},
						{0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF},
						{0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF},
						{0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF},
						{0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF} };


/*static unsigned char GS[3][8] = { { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
				  { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
				  { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF} };
*/


//static unsigned char GS[3][8] = { { 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF},
//				  { 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF},
//				  { 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF} };
