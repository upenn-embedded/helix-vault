/*
Header file for LCD driver
Author: Tomas
Note: someone should definitely double check this because I have no clue if it is going to work
*/

#include <avr/io.h>
#include <stdlib.h>
#include <stddef.h>   // for NULL

#ifndef ST7735_new_H_
#define ST7735_new_H_

// LCD pin connections based on Lab 4:
#define LCD_PORT		PORTB
#define LCD_DDR			DDRB
#define LCD_DC			PORTB0
#define LCD_RST			PORTB1
#define LCD_TFT_CS		PORTB2
#define LCD_MOSI		PORTB3
#define LCD_SCK			PORTB5

//PWM on pin 6 for brightness control/connect to 5V for full brightness
#define LCD_LITE_PORT	PORTD
#define LCD_LITE_DDR	DDRD
#define LCD_LITE		PORTD6

// LCD height, width, and size in pixels (from data sheet):
#define LCD_WIDTH 160
#define LCD_HEIGHT 128
#define LCD_SIZE  LCD_WIDTH * LCD_HEIGHT

// ST7735 registers from datasheet (starting on page 79):
#define NOP 0x00    // no operation
#define SWRESET 0X01    // software reset
#define RDDID 0x04  // read display ID
#define RDDST 0x09  // read display status
#define RDDPM 0x0A  // read display power
#define RDDMADCTL 0x0B  // read display
#define RDDCOLMOD 0x0C  // read display pixel
#define RDDIM 0x0D  // read display image
#define RDDSM 0x0E  // read display signal
#define SLPIN 0x10  //sleep in and booster off
#define SLPOUT 0x11 // sleep in and booster on
#define PTLON 0x12  // partial mode on
#define NORON 0x13  // partial mode off
#define INVOFF 0x20 // display inversion off
#define INVON 0x21  // display inversion on
#define GAMSET 0x26 // gamma curve select
#define DISPOFF 0x28    // display off
#define DISPON 0x29 // display on
#define CASET 0x2A  // column address set
#define RASET 0x2B  // row address set
#define RAMWR 0x2C  // memory write
#define RGBSET 0x2D // LUT for 4k, 65k, 262k
#define RAMRD 0x2E  // memory read
#define PTLAR 0x30  // partial start/end address set
#define TEOFF 0x34  // tearing effect line off
#define TEON 0x35   // tearing effect mode set and on
#define MADCTL 0x36 // memory data access control
#define IDMOFF 0x38 // idle mode off
#define IDMON 0x39  // idel mode on
#define COLMOD 0x3A // interface pixel format
#define RDID1 0xDA  // read ID1
#define RDID2 0xDB  // read ID2
#define RDID3 0xDC  // read ID3

// more registers (starting from page 122):
#define FRMCTR1 0xB1    // in normal mode (full colors)
#define FRMCTR2 0xB2    // in idle mode (8-colors)
#define FRMCTR3 0xB3    // in partial mode + full colors
#define INVCTR 0xB4     // display inversion control
#define DISSET5 0xB6    // display function setting
#define PWCTR1 0xC0     // power control setting
#define PWCTR2 0xC1     // power control setting
#define PWCTR3 0xC2     // in normal mode
#define PWCTR4 0xC3     // in idle mode (8-colors)
#define PWCTR5 0xC4      // in partial mode + full colors
#define VMCTR1 0xC5     // VCOM control 1
#define VMOFCTR 0xC7    // set VCOM offset control
#define WRID2 0xD1      // set LCM version code
#define WRID1 0xD2      // customer project code
#define NVCTR1 0xD9     // NVM control status
#define NVCTR2 0xDE     // NVM Read Command
#define NVCTR3 0xDF     // NVM write command
#define GAMCTRP1 0xE0   // set gamma adjustment (+polarity)
#define GAMCTRN1 0xE1   // set gamma adjustment (-polarity)

// define bit flags inside of MADCTL register (will be needed for initialization I believe):
#define MADCTL_MY 0x80  // vertically flip display when set
#define MADCTL_MX 0x40  // horizontally flip display when set
#define MADCTL_MV 0x20  // rotate display 90 degrees
#define MADCTL_ML 0x10  // control direction of LCD refresh: top to bottom or bottom to top
#define MADCTL_RGB 0x00 // RGB pixel format
#define MADCTL_MH 0x04  // control direction of LCD refresh: left to right or right to left

typedef struct {
    uint8_t cmd;           // command code
    uint8_t numArgs;       // number of argument bytes
    const uint8_t *args;   // pointer to argument bytes (or NULL)
    uint8_t delayMs;       // post-command delay (0 = no delay)
} lcd_cmd_t;

void LCD_init(void);
void sendCommands(const lcd_cmd_t *cmdList, uint8_t count);
void LCD_setAddress(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
void SPI_controllerTx_byte(uint8_t stream);
void SPI_controllerTx(uint16_t data);
void Delay_ms(unsigned int n);

#endif /* ST7735_NEW_H_ */