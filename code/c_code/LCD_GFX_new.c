/*
Source file for graphics
*/
#define F_CPU 16000000UL

#include "LCD_GFX_new.h"
#include "ST7735_new.h"
#include <math.h>

#define PI 3.14159

// function to convert a 24-bit RGB value to 16-bits:
uint16_t rgb565(uint8_t red, uint8_t green, uint8_t blue){
    int red5 = (31*(red+4))/255;        // linear mapping from 8 bits to 5 bits
    int green6 = (63*(green+2))/255;    // linear mapping from 8 bits to 6
    int blue5 = (31*(blue+4))/255;      // linear mapping from 8 bits to 5
    uint16_t combined = (red5<<11) | (green6<<5) | (blue5); // combine into 16-bit RGB data
    return combined;
}

// fucntion to draw pixel to the given x y coordinate:
void draw_pixel(uint8_t x, uint8_t y, uint16_t color){
    LCD_setAddress(x, y, x, y);     // set window to only one pixel
    LCD_PORT |= (1<<LCD_DC); // data mode
    SPI_controllerTx(color);        // send color to pixel
}

// function to draw a character based on ASCII table from lab 4 (8x5)
void draw_char(uint8_t x0, uint8_t y0, uint16_t character, uint16_t f_c, uint16_t b_c){
	uint8_t row = character - 0x20;	// find character in ASCII (it starts at space which is row 20)
	// make sure there is enough space in the screen for the character (characters are 5x8 - 5 columns, 1 byte each (8 rows))
	if ((LCD_WIDTH - x0 > 6)&&(LCD_HEIGHT - y0 > 7)){	
		// set window for the entire 6x8 character block *one time*
        LCD_setAddress(x0, y0, x0 + 5, y0 + 7);
        LCD_PORT |= (1<<LCD_DC); // data mode

		// loop through each row of the 8-row character
        for (uint8_t j = 0; j < 8; j++){
            // loop through each of the 5 ASCII columns
            for (uint8_t i = 0; i < 5; i++){
                uint8_t column_pixels = ASCII[row][i]; // extract byte for this ASCII column
                // check pixel bit at row j
                uint16_t color = ((column_pixels >> j) & 1) ? f_c : b_c;
                SPI_controllerTx(color); // stream pixel color directly
            }
            // add 1 column of background color as spacing between characters
            SPI_controllerTx(b_c);
        }
		LCD_PORT |= (1 << LCD_TFT_CS);
    }
}

// // function to draw a character based on ASCII table of bigger characters (16x10)
// void draw_char16(uint8_t x0, uint8_t y0, uint16_t character, uint16_t f_c, uint16_t b_c){
// 	uint8_t row = character - 0x20;	// find character in ASCII (it starts at space which is row 20)
// 	// make sure there is enough space in the screen for the character (characters are 5x8 - 5 columns, 1 byte each (8 rows))
// 	if ((LCD_WIDTH - x0 > 11)&&(LCD_HEIGHT - y0 > 15)){	
// 		for (int i = 0;i < 5;i++){
// 			uint8_t column_pixels = ASCII_10x16[row][i];	// extract byte (rows) for a given column in ASCII table
// 			for (int j = 0;j < 8;j++){
// 				if (((column_pixels>>j)&1) == 1){	// shift column_pixel by j bits and compare bit to 1 
// 					draw_pixel(x0+i,y0+j,f_c);		// if bit = 1, draw with font color at shifted position (by i in x and j in y)
// 				}
// 				else {
// 					draw_pixel(x0+i,y0+j, b_c);	// if bit = 0, draw pixel with background color at shifted position
// 				}
// 			}
// 		}

// 	}
// }

// draw filled circle (from lab 4)
void LCD_drawCircle(uint8_t x0, uint8_t y0, uint8_t radius,uint16_t color)
{
	// even faster and simpler:
	for (int y = -radius; y <= radius; y++) {
		int x_width = (int)sqrt(radius * radius - y * y);
		LCD_drawBlock(x0 - x_width, y0 + y, x0 + x_width, y0 + y, color);
	}
}

// draw horizontal line (from lab 4)
void drawLineH(short x0, short y0, short x1, short y1, uint16_t color){
	if(x0 > x1){
		short tmp = x0;
		x0 = x1;
		x1 = tmp;
		tmp = y0;
		y0 = y1;
		y1 = tmp;
	}

	uint8_t dx = x1 - x0;
	uint8_t dy = y1 - y0;
	uint8_t D = 0;

	if (dy < 0){
		D = -1;
	}
	if (dy == 0){
		D = 0;
	}
	else{
		D = 1;
	}

	dy = dy * D;

	uint8_t y = y0;
	uint8_t pos = 2*dy - dx;
	for(uint8_t i = 0; i<=dx+1; i++){
		draw_pixel(x0 + i, y, color);
		if(pos >= 0){
			y += D;
			pos = pos - 2 * dx;
		}
		pos = pos + 2 * dy;
	}
}

// draw vertical line (from lab 4)
void drawLineV(short x0, short y0, short x1, short y1, uint16_t color){
	if(y0 > y1){
		short tmp = x0;
		x0 = x1;
		x1 = tmp;
		tmp = y0;
		y0 = y1;
		y1 = tmp;
	}

	uint8_t dx = x1 - x0;
	uint8_t dy = y1 - y0;
	uint8_t D = 0;

	if (dx < 0){
		D = -1;
	}
	if (dx == 0){
		D = 0;
	}
	else{
		D = 1;
	}

	dx = dx * D;

	uint8_t x = x0;
	uint8_t pos = 2*dx - dy;
	for(uint8_t i = 0; i<=dy+1; i++){
		draw_pixel(x, y0 + i, color);
		if(pos >= 0){
			x += D;
			pos = pos - 2 * dy;
		}
		pos = pos + 2 * dx;
	}
}

// draw any line (from lab 4)
void LCD_drawLine(short x0,short y0,short x1,short y1,uint16_t c)
{
	if(fabs(x1-x0) > fabs(y1-y0)){
		drawLineH(x0,y0,x1,y1,c);
	}
	else{
		drawLineV(x0,y0,x1,y1,c);
	}
}

// draw block (from lab 4)
void LCD_drawBlock(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1,uint16_t color)
{
	LCD_setAddress(x0,y0,x1,y1);
    LCD_PORT |= (1<<LCD_DC); // data mode
	for(int i = 0; i < (x1-x0+1)*(y1-y0+1); i++){
		SPI_controllerTx(color);
	}
}

// set screen color (from lab 4)
void LCD_setScreen(uint16_t color) 
{
	LCD_setAddress(0,0,159,127);
    LCD_PORT |= (1<<LCD_DC); // data mode
	for(int i = 0; i < 160*128; i++){
		SPI_controllerTx(color);
	}
	
}

// function to draw a string of characters (from lab 4)
void LCD_drawString(uint8_t x, uint8_t y, char* str, uint16_t fg, uint16_t bg, uint8_t fontSize)
{
	if (fontSize == 8)
		for (int i = 0; str[i] != '\0'; i++){
			draw_char(x + 6*i, y, str[i], fg, bg);
		}
	// if (fontSize == 16)
	// 	for (int i = 0; str[i] != '\0'; i++){
	// 		draw_char16(x + 11*i, y, str[i], fg, bg);
	// 	}
}


// ID verified function
void LCD_ID_verified(void){
	LCD_setScreen(WHITE);
	LCD_drawString(20, 50, "Identity Verified", BLUE, WHITE, 8);
}

void LCD_combination(void){
	LCD_setScreen(WHITE);
	LCD_drawString(20, 50, "Input Combination", BLUE, WHITE, 8);
}

void LCD_pin(void){
	LCD_setScreen(WHITE);
	LCD_drawString(20, 50, "Input PIN", BLUE, WHITE, 8);
	LCD_drawString(50, 80, "- - - -", BLUE, WHITE, 8);
}