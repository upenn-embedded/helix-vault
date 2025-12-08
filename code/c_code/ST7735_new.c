/*
LCD driver
*/

#ifndef F_CPU
#define F_CPU 16000000UL   // set to MCU clock — change if different
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include "ST7735_new.h"

// Just using _delay_ms() was giving me issues because it expected a "time integer constant", so this function
// loops 1 ms delays as many times as need:

void Delay_ms(unsigned int n)
{
    while (n--) {
        _delay_ms(1);    // _delay_ms accepts a compile-time constant here, 1 is fine
    }
}


// pin initialization
static void LCD_pin_init(void){
    // digital pin setup (all output pins):
    LCD_DDR |= (1<<LCD_DC);       // data/command (output)
    LCD_DDR |= (1<<LCD_RST);      // reset (output)
    LCD_DDR |= (1<<LCD_TFT_CS);   // chip select (output)
    LCD_DDR |= (1<<LCD_MOSI);     // Master Out Slave In (output)
    LCD_DDR |= (1<<LCD_SCK);      // clock (output)
    LCD_LITE_DDR |= (1<<LCD_LITE);

    // Brightness control using PWM:
    // PWM setup:
    TCCR0A |= (1<<COM0A1);    // non-inverting PWM mode
    TCCR0A |= (1<<WGM01);     // Fast PWM, TOP=0xFF
    TCCR0A |= (1<<WGM00);     // Fast PWM, TOP=0xFF
    TCCR0B |= (1<<CS02);      // /256 prescaler (62,500 Hz/256 = 244 Hz - fast PWM freq is 62,500 Hz)
    OCR0A = 100;            // 39% duty cycle for low-ish brightness

    // Default pin states BEFORE reset:
    LCD_PORT |= (1<<LCD_TFT_CS);  // CS high = inactive
    LCD_PORT &= ~(1<<LCD_DC);     // DC low = command by default
    LCD_PORT |= (1<<LCD_RST);     // make sure RESET starts HIGH

    // Hardware reset pulse (active low)
    LCD_PORT &= ~(1<<LCD_RST);    // RST = low
    Delay_ms(20);                 // hold reset low (>=10ms recommended)
    LCD_PORT |= (1<<LCD_RST);     // RST = high
    Delay_ms(150);                // wait for internal startup
}

// SPI controller initialization in master mode
static void SPI_controller_init(void){
    SPCR0 |= (1<<SPE);        // enable SPI module
    SPCR0 |= (1<<MSTR);       // set MCU as SPI master
    // clock rate is set to default (SPR01 = 0, SPR00 = 0), which is fck/4
    SPSR0 |= (1<<SPI2X);      // double the SPI speed (so fck/2 = 8 MHz)
}

// SPI helper function to send 1 byte of data (8 bits) by loading it into data register and waiting for the SPI to transmit it
void SPI_controllerTx_byte(uint8_t data){
    SPDR0 = data;       // write byte on SPI data register
    while(!(SPSR0 & (1<<SPIF)));    // do nothing while waiting for SPIF flag to become 1 (means transmission is complete)
}

// helper function that splits a 16-bit value into 2 bytes to send over SPI, also controlling chip select to select
// which device we want to talk to (only LCD through SPI for now)
void SPI_controllerTx(uint16_t data){
    uint8_t low = data & 0xFF;   // extract lower 8 bits
    uint8_t high = (data >> 8);  // extract upper 8 bits

    LCD_PORT &= ~(1<<LCD_TFT_CS);       // pull chip select low to tell LCD data transfer is starting
    SPI_controllerTx_byte(high);        // send upper 8 bits using helper function
    SPI_controllerTx_byte(low);         // send lower 8 bits using helper function

    LCD_PORT |= (1<<LCD_TFT_CS);          // pull chip select high to tell LCD data transfer is done
}

// // command structure for initialization:
// typedef struct {
//     uint8_t cmd;           // command code
//     uint8_t numArgs;       // number of argument bytes
//     const uint8_t *args;   // pointer to argument bytes (or NULL)
//     uint8_t delayMs;       // post-command delay (0 = no delay)
// } lcd_cmd_t;

// manufacturer-recommended initialization values from online sources and lab:

// software reset requires 120 ms delay (pg. 83)
// no arguments needed for SWRESET

// exit sleep mode requires ≥120 ms delay (pg. 94)
// no arguments needed for SLPOUT

// frame rate control 1 (RTNA, FPA, BPA - vertical refresh)
static const uint8_t frmctr1_args[] = {0x01, 0x2C, 0x2D};

// frame rate control 2 (same as FRMCTR1)
static const uint8_t frmctr2_args[] = {0x01, 0x2C, 0x2D};

// frame rate control 3 (parameters for dot inversion and back/front porch)
static const uint8_t frmctr3_args[] = {0x01, 0x2C, 0x2D, 0x01, 0x2C, 0x2D};

// inversion control - normal
static const uint8_t invctr_args[]  = {0x07};

// power control 1: AVDD, VRH, MODE (VRH sets GVDD voltage)
static const uint8_t pwctr1_args[]  = {0x0A, 0x02, 0x84};

// power control 2: VGH2 (set VGH/VGL voltage)
static const uint8_t pwctr2_args[]  = {0xC5};

// power control 3: APA, DCA (adjust op amp and booster voltage)
static const uint8_t pwctr3_args[]  = {0x0A, 0x00};

// power control 4: APB, DCB (adjust op amp and booster voltage)
static const uint8_t pwctr4_args[]  = {0x8A, 0x2A};

// power control 5: APC, DCC (booster/amp idle adjustments)
static const uint8_t pwctr5_args[]  = {0x8A, 0xEE};

// vcom control 1: reduce flicker
static const uint8_t vmctr1_args[]  = {0x0E};

// memory access control (initial, will set rotation again later)
static const uint8_t madctl_args[]  = {0xC8};

// interface pixel format: RGB565
static const uint8_t colmod_args[]  = {0x05};

// column address set: start at 0, end at 127
static const uint8_t caset_args[]   = {0x00,0x00,0x00,0x7F};

// row address set: start at 0, end at 159
static const uint8_t raset_args[]   = {0x00,0x00,0x00,0x9F};

// gamma correction (positive polarity), recommended values
static const uint8_t gamctrp1_args[]= {
    0x02,0x1C,0x07,0x12,0x37,0x32,0x29,0x2D,
    0x29,0x25,0x2B,0x39,0x00,0x01,0x03,0x10
};

// gamma correction (negative polarity), recommended values
static const uint8_t gamctrn1_args[]= {
    0x03,0x1D,0x07,0x06,0x2E,0x2C,0x29,0x2D,
    0x2E,0x2E,0x37,0x3F,0x00,0x00,0x02,0x10
};

// final rotation setting
static const uint8_t madctl_rot_args[]= {
    MADCTL_MY | MADCTL_MV | MADCTL_RGB
};

// initialization table using values defined above + delays when needed
static const lcd_cmd_t init_cmds[] = {

    {SWRESET, 0, NULL, 130},
    {SLPOUT,  0, NULL, 200},

    {FRMCTR1, 3, frmctr1_args, 0},
    {FRMCTR2, 3, frmctr2_args, 0},
    {FRMCTR3, 6, frmctr3_args, 0},

    {INVCTR,  1, invctr_args,  0},

    {PWCTR1,  3, pwctr1_args,  0},
    {PWCTR2,  1, pwctr2_args,  0},
    {PWCTR3,  2, pwctr3_args,  0},
    {PWCTR4,  2, pwctr4_args,  0},
    {PWCTR5,  2, pwctr5_args,  0},

    {VMCTR1,  1, vmctr1_args,  0},

    {INVOFF,  0, NULL,         0},

    {MADCTL,  1, madctl_args,  0},
    {COLMOD,  1, colmod_args,  0},

    {CASET,   4, caset_args,   0},
    {RASET,   4, raset_args,   0},

    {GAMCTRP1,16, gamctrp1_args, 0},
    {GAMCTRN1,16, gamctrn1_args, 0},

    {NORON,   0, NULL,        10},
    {DISPON,  0, NULL,       100},

    {MADCTL,  1, madctl_rot_args, 10}
};

void LCD_init(void){
    LCD_pin_init();         // initialize pins
    SPI_controller_init();  // initialize SPI controller
    Delay_ms(20);          // small delay

    sendCommands(init_cmds, sizeof(init_cmds)/sizeof(lcd_cmd_t));   // send initialization commands
}

// function prototype to send commands. Takes a pointer to an array of lcd_cmd_t structs and the number 
// of entries in that array. Should work for any list of commands
void sendCommands(const lcd_cmd_t *cmdList, uint8_t count)
{
    LCD_PORT &= ~(1<<LCD_TFT_CS);   // CS low - begin communication for init sequence

    for (uint8_t i = 0; i < count; i++) {   // loop over each command entry - note that max count = 255

        LCD_PORT &= ~(1<<LCD_DC);   // command mode
        SPI_controllerTx_byte(cmdList[i].cmd);  // sends the cmd byte using helper function

        if (cmdList[i].numArgs > 0) {   // check for data bytes
            LCD_PORT |= (1<<LCD_DC);    // switch to data mode
            for (uint8_t j = 0; j < cmdList[i].numArgs; j++) {  // loop over bytes stored in cmdList
                SPI_controllerTx_byte(cmdList[i].args[j]);  // send byte using helper functino
            }
        }

        if (cmdList[i].delayMs) // check if delay is nonzero
            Delay_ms(cmdList[i].delayMs);  // delay 
    }

    LCD_PORT |= (1<<LCD_TFT_CS);    // CS high - end communication
}

// function that sets the memory address of the pixel we want to write to
void LCD_setAddress(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
    uint8_t c_args[] = {0x00, x0, 0x00, x1};  // column arguments
    uint8_t r_args[] = {0x00, y0, 0x00, y1};  // row arguments

    lcd_cmd_t cmds[] = {
        {CASET, 4, c_args, 0},     // set column range
        {RASET, 4, r_args, 0},     // set row range
        {RAMWR, 0, NULL, 10}       // begin writing to RAM
    };

    sendCommands(cmds, 3);     // send commands
}
