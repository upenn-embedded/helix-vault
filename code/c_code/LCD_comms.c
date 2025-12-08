#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include "ST7735_new.h"
#include "LCD_GFX_new.h"
#include "uart.h"
#include <avr/interrupt.h>

#define BIT0    PC3   // PC4 input
#define BIT2    PC5   // PC5 input
#define BIT1    PC4   // PC3 input
#define SERVO   PD2   // servo PWM

#define FINGER0 PC1   // Fingerprint check pin
#define FINGER1 PC2   // Fingerprint check pin
#define FINGER_OUT PC0 // fingerprint reset line

// motor pins:
#define IN1 PD3     // H-bridge pin 2 (IN1)
#define IN2 PD4     // H-bridge pin 7 (IN2)
#define PWM PD5     // PWM for speed control

// parameters for servo controls
volatile uint16_t servo_pulse_ticks = 150;  // default = 1.5ms
volatile uint16_t tick_counter = 0;
volatile uint8_t open = 120;    // open latch in degrees
volatile uint8_t closed = 0;    // closed latch in degrees
volatile uint8_t fingerprint_read = 0;

// ---------------------------------- MCU Pinout -------------------------------------------
/*
LCD:
- PB0 --> LCD DC
- PB1 --> LCD_RST
- PB2 --> LCD_TFT_CS
- PB3 --> LCD_MOSI
- PB5 --> LCD_SCK
- PD6 --> LCD_LITE

SERVO:
- PD2 --> servo PWM (orange)
- 5V rail --> servo power (red)
- GND --> GND (brown)

H-Drive + DC motor:
- PD3 --> IN1 (pin 2)
- PD4 --> IN2 (pin 7)
- 5V rail --> 1,2 EN (pin 1)
- 5V rail --> Vcc1 (pin 16)
- GND --> GND (pins 4, 5)
- 1Y (pin 3) --> motor +
- 2Y (pin 6) --> motor -
- Buck 6V --> Vcc2 (pin 8)

Other ATmega MCU:
- PC3 --> PC3 (bit0)
- PC4 --> PC4 (bit1)
- PC5 --> PC5 (bit2)
*/


void setup_inputs(void)
{
    // Set PC2, PC3, PC4, PC5 as inputs
    DDRC &= ~((1 << BIT0) | (1 << BIT1) | (1 << BIT2) | (1 << FINGER0) | (1 << FINGER1));
    
    // Disable pull-ups 
    PORTC &= ~((1 << BIT0) | (1 << BIT1) | (1 << BIT2) | (1 << FINGER0) | (1 << FINGER1));
}

void setup_outputs(void)
{
    // Set PC3, PC4, PC5 as outputs
    DDRC |= ((1 << BIT0) | (1 << BIT1) | (1 << BIT2) | (1 << FINGER_OUT));
    
    // Disable pull-ups 
    PORTC &= ~((1 << BIT0) | (1 << BIT1) | (1 << BIT2) | (1 << FINGER_OUT));
}

void motor_init(void)
{
    // Motor pins output
    // DDRD |= (1<<IN1) | (1<<IN2) | (1<<PWM);
    DDRD |= (1<<IN1) | (1<<IN2);

    // // PWM initialization:
    // TCCR0A = (1<<WGM00) | (1<<WGM01);   // fast PWM mode
    // TCCR0A |= (1<<COM0B1);              // non-inverting PWM
    // TCCR0B = (1<<CS01);                 // /8 clock prescaler 
    // OCR0B = 0;                          // set compare register to 0 to start (0 speed)

    // // ADC initialization:
    // ADMUX = (1<<REFS0);         // set 5V as reference voltage
    // ADCSRA = (1<<ADEN) | 7;     // enable ADC and set prescaler to /128 (ADPS2 = 1, ADPS1 = 1, ADPS0 = 1)
}

void servo_init(void)
{
    // PD2 as output
    DDRD |= (1 << SERVO);

    // Timer 2 setup (0.5 us ticks with /8 prescaler):    
    TCCR2A = (1 << WGM21);  // CTC mode
    TCCR2B = (1 << CS21);   // prescaler = 8
    OCR2A = 19;             // 10 µs interrupt

    // Enable timer compare interrupt
    TIMSK2 = (1 << OCIE2A);

    sei();
}

// // interrupt for finger print (PC2)
// void pc2_interrupt_init(void) {
//     PCICR |= (1<<PCIE1);    // Enable Pin Change Interrupt group 1 (PCINT[14:8])
//     PCMSK1 |= (1<<PCINT10); // Enable interrupt for PC2 (PCINT10)

//     sei();                  // Enable global interrupts
// }

// interrupt service routine for timer 2 to generate pulse on PD2:
ISR(TIMER2_COMPA_vect)
{
    tick_counter++;

    if (tick_counter <= servo_pulse_ticks)
        PORTD |= (1 << SERVO);  // output HIGH for the pulse
    else
        PORTD &= ~(1 << SERVO); // output LOW for the pulse

    // Restart 20 ms frame after 2000 ticks (10 us each)
    if (tick_counter >= 2000)
        tick_counter = 0;
}

// ISR(PCINT1_vect) {
//     // Check if PC2 is actually high
//     if (PINC & (1<<PC2)) {
//         fingerprint_read = 1;
//     }
// }

// ------------------------------------ SERVO CONTROLS --------------------------------------

void servo_set_us(uint16_t us)
{
    // constrain pulse to 1000?2000 µs (based on servo data sheet)
    // this gives about 90 deg ROM, I found that using 500 and 2500 instead gives more ROM
    if (us < 1000) us = 1000;
    if (us > 2000) us = 2000;

    // convert microseconds to 10 µs ticks
    servo_pulse_ticks = us / 10;
}

// function to map angle in degrees to pulse length in us
void servo_write_deg(uint8_t angle)
{
    if (angle > 180) angle = 180;

    uint16_t us = 600 + ((uint32_t)angle * 1800UL) / 180;
    servo_set_us(us);
}

// ------------------------------ MOTOR CONTROLS ------------------------------------

// function that reads ADC in the given channel
// uint16_t adc_read(uint8_t ch) {
//     ADMUX = (ADMUX & 0xF0) | (ch & 0x0F);   // select channel to measure (only change bottom 4 bits - MUX3, MUX2, MUX1, MUX 0)
//     ADCSRA |= (1<<ADSC);                    // start ADC conversion
//     while (ADCSRA & (1<<ADSC));             // while loop that wait for conversion to finish (ADSC --> 0)
//     return ADC;                             // return measured ADC
// }

// function to stop motor:
void motor_stop() {
    // OCR0B = 0;              // set speed to 0
    PORTD &= ~(1<<IN1);     // set IN1 to 0
    PORTD &= ~(1<<IN2);     // set IN2 to 0
}

// function to move motor up at a given speed:
void motor_up(uint8_t speed) {
    PORTD |=  (1<<IN1);     // set IN1 HIGH
    PORTD &= ~(1<<IN2);     // set IN2 LOW
    // OCR0B = speed;          // set timer output compare to speed (0-255)
}

// function to move motor down at a given speed:
void motor_down(uint8_t speed) {
    PORTD &= ~(1<<IN1);     // set IN1 LOW
    PORTD |=  (1<<IN2);     // set IN2 HIGH
    // OCR0B = speed;          // set timer output compare for speed
}

// ------------------------------------ MCU COMMUNICATION -------------------------------------------------

void talk_to_MCU(uint8_t state){ // PC3 = bit0, PC4 = bit1, PC5 = bit2. Parallel 3 bit comms. SENDING TO LCD
    // Clear PC3, PC4, PC5
    PORTC &= ~((1 << BIT0) | (1 << BIT1) | (1 << BIT2));

    // Mask state to 3 bits (0?7) and shift into PC3?PC5
    PORTC |= (state & 0x07) << BIT0;
    //printf("State:%u \r\n",state);
}

// function that checks state of progress from other MCU
uint8_t read_inputs(void)
{
    uint8_t pin_state = PINC;  // Read entire port once (best practice)

    uint8_t BIT0_high     = (pin_state & (1 << BIT0))     != 0;
    uint8_t BIT1_check_high = (pin_state & (1 << BIT1)) != 0;
    uint8_t BIT2_check_high = (pin_state & (1 << BIT2)) != 0;

    // return a bitmask:
    return (BIT2_check_high << 2) | (BIT1_check_high << 1) | (BIT0_high);
}

/*
Possible states for transmitter:
0 - wait
1 - switch
*/

uint8_t read_fingerprint_holder(void)
{
    // placeholder screen:
    LCD_setScreen(WHITE);
    LCD_drawString(15, 50, "Waiting for Fingerprint", BLUE, WHITE, 8);
    Delay_ms(50);

    // Wait loop: Wait until EITHER FINGER0 or FINGER1 goes high
    // This assumes that "00" is the "no signal" state.
    while( !((PINC & (1<<FINGER0)) || (PINC & (1<<FINGER1))) ){
        Delay_ms(2);
    }

    Delay_ms(50); // Allow signals to settle

    // Construct the 2-bit value using bit manipulation
    uint8_t fp_value = ((PINC >> FINGER0) & 0x01) | (((PINC >> FINGER1) & 0x01) << 1);

    Delay_ms(50);
    
    LCD_setScreen(WHITE);
    if (fp_value == 1){
      LCD_drawString(20, 50, "Welcome, Jeevan", BLUE, WHITE, 8);  
    }
    if (fp_value == 2){
      LCD_drawString(20, 50, "Welcome, Yongwoo", BLUE, WHITE, 8);  
    }
    if (fp_value == 3){
      LCD_drawString(20, 50, "Welcome, Tomas", BLUE, WHITE, 8);  
    }
    
    Delay_ms(100);

    return fp_value; // Return the identity value
}

/*
Possible states (for receiver):
0 - ADC wrong
1 - ADC correct
2 - Wait
3 - Pin value entered
4 - Clear
5 - Correct PIN
6 - Wrong PIN
7 - 
*/

uint8_t LCD_receiveControls(void){
    uint8_t state = read_inputs();      // check state
    //printf("STATE: %u \r\n", state);    // print state (for debugging)
    uint8_t prev_state = 10;            // initialize previous state as 10 so it doesn't mess with the logic (will be overwritten shortly)
    uint8_t PIN_count = 0;              // set pin count to 0
    uint8_t not_done = 1;               // keep track of whether the right combination and pin have been received

    // loop that runs as long as the right combination and pin haven't been received:
    while(not_done){
        state = read_inputs();              // check state
        //printf("STATE: %u \r\n", state);    // print state

        // if the correct combination hasn't been received:
        if ((state == 0)&(prev_state!=0)){
            prev_state = 0;                 // set previous state to 0
            // display correspnding screen on LCD:
            LCD_setScreen(WHITE);
            LCD_drawString(29, 50, "Enter Combination", BLUE, WHITE, 8);
            // wait for the state to change:
            while(state == 0){
                state = read_inputs();
                Delay_ms(5);
            }
        }

        // if the correct combination was received:
        if ((state == 1))
        {
            prev_state = 1;         // set previous state to 1
            // display combination accepted screen for 1 second:
            LCD_setScreen(WHITE);
            LCD_drawString(20, 50, "Combination Accepted", BLUE, WHITE, 8);
            Delay_ms(1000);
            // display "Enter PIN" screen:
            LCD_setScreen(WHITE);
            LCD_drawString(20, 45, "Enter PIN:", BLUE, WHITE, 8);
            LCD_drawString(20, 55, "# to finish", BLUE, WHITE, 8);
            LCD_drawString(20, 80, "-", BLUE, WHITE, 8);
            LCD_drawString(30, 80, "-", BLUE, WHITE, 8);
            LCD_drawString(40, 80, "-", BLUE, WHITE, 8);
            LCD_drawString(50, 80, "-", BLUE, WHITE, 8);
            PIN_count = 0;  // set PIN count to 0
            // wait for state to change
            while(state == 1){
                state = read_inputs();
                Delay_ms(5);
            }
        }

        // if in state 2 - just waiting for comunication
        if (state == 2)
        {
            Delay_ms(5);
            state = read_inputs();  // check state
        }

        // if PIN value was entered:
        if (state == 3)
        {
            prev_state = 3; // set previous state to 3
            // first PIN value entered:
            if (PIN_count == 0){
                // replace "-" with "*" on LCD screen
                LCD_drawString(20, 80, "*", BLUE, WHITE, 8);
                PIN_count += 1;
            }
            // second PIN value entered:
            else if (PIN_count == 1)
            {
                // replace "-" with "*" on LCD screen
                LCD_drawString(30, 80, "*", BLUE, WHITE, 8);
                PIN_count += 1;
            }
            // third PIN value entered:
            else if (PIN_count == 2)
            {
                // replace "-" with "*" on LCD screen
                LCD_drawString(40, 80, "*", BLUE, WHITE, 8);
                PIN_count += 1;
            }
            // fourth PIN value entered:
            else if (PIN_count == 3)
            {
                // replace "-" with "*" on LCD screen
                LCD_drawString(50, 80, "*", BLUE, WHITE, 8);
                PIN_count += 1;
            }
            // reset PIN count if else:
            else {
                PIN_count = 0;
                Delay_ms(5);
            }
            // wait for state to change:
            while(state == 3){
                state = read_inputs();
                Delay_ms(5);
            }
        }

        // if the user clears the PIN values entered:
        if (state == 4)
        {
            PIN_count = 0;  // reset PIN count
            // check if the PIN was entered incorrectly
            if (prev_state == 5){
                // reset entire screen display
                LCD_setScreen(WHITE);
                LCD_drawString(20, 50, "Enter PIN:", BLUE, WHITE, 8);
            }
            // reset PIN "- - - -"
            LCD_drawString(20, 80, "-", BLUE, WHITE, 8);
            LCD_drawString(30, 80, "-", BLUE, WHITE, 8);
            LCD_drawString(40, 80, "-", BLUE, WHITE, 8);
            LCD_drawString(50, 80, "-", BLUE, WHITE, 8);
            prev_state = 4; // set previous state
            
            // wait for state to change:
            while(state == 4){
                state = read_inputs();
                Delay_ms(5);
            }
        }

        // if the user inputs the incorrect pin:
        if (state == 5)
        {
            // set corresponding screen display:
            LCD_setScreen(RED);
            LCD_drawString(41, 60, "Incorrect PIN", WHITE, RED, 8);
            LCD_drawString(41, 80, "Press * to retry", WHITE, RED, 8);
            // wait for state to change:
            prev_state = state; // save previous state
            // wait for state to change:
            while(state == 5){
                state = read_inputs();
                Delay_ms(5);
            }
        }

        // if the user inputs the correct pin
        if (state == 6)
        {
            // set corresponding screen display:
            LCD_setScreen(GREEN);
            LCD_drawString(44, 60, "PIN accepted", WHITE, GREEN, 8);
            not_done = 0;           // signal that the function is done
            prev_state = state;     // save previous state
            // wait for state to change:
            while(state == 0){
                state = read_inputs();
                Delay_ms(5);
            }
        }
    }
    return not_done;   
}

// subroutine to open latch with servo
void openBox(void){
    servo_write_deg(open);
    Delay_ms(1000);
}

// subroutine to close latch with servo
void closeBox(void){
    servo_write_deg(closed);
    Delay_ms(1000);
}

int main(void){
    // initialize pins:
    setup_outputs(); // Initialize as output first for safety, loop handles switching
    servo_init();
    LCD_init();
    motor_init();
    
    // START INFINITE LOOP (Security System Loop)
    while(1) {
        
        // --- 1. LOCKDOWN & RESET STATE ---
        // Ensure inputs/outputs are correct for Fingerprint phase (this MCU talks TO other)
        setup_outputs(); 

        // make sure box is closed:
        LCD_setScreen(WHITE);
        LCD_drawString(20, 50, "System Locked", RED, WHITE, 8);
        Delay_ms(50);
        closeBox();
        
        // make sure sliding door is closed:
        motor_up(200);
        Delay_ms(500); // Give motor time to close door
        motor_stop();
        
        // --- 2. AUTHENTICATION ---
        // wait for fingerprint to be accepted:
        uint8_t finger_identity = read_fingerprint_holder();
        talk_to_MCU(finger_identity);
        Delay_ms(50);
        
        // --- 3. PIN ENTRY PHASE ---
        // Switch communication direction: We now listen to the other MCU
        setup_inputs();
        
        // open sliding door:
        motor_down(200);
        Delay_ms(500);
        motor_stop();
        
        // wait for correct combination + PIN:
        uint8_t fook = 0;
        LCD_setScreen(WHITE);
        fook = LCD_receiveControls();
        
        // --- 4. UNLOCKED STATE ---
        // open latch
        openBox();
        
        LCD_setScreen(GREEN);
        LCD_drawString(20, 50, "UNLOCKED", WHITE, GREEN, 8);
        LCD_drawString(20, 70, "Press * to Lock", WHITE, GREEN, 8);
        
        // --- 5. WAIT FOR LOCK SIGNAL (State 7) ---
        uint8_t current_state = 0;
        
        // Stay in this loop while the state IS NOT 7
        while (current_state != 7) {
            current_state = read_inputs();
            Delay_ms(5); // check every 5ms
        }
        
        // If we break the loop, state 7 was received
        LCD_setScreen(RED);
        LCD_drawString(20, 50, "LOCKING SYSTEM...", WHITE, RED, 8);
        PORTC |=  (1 << FINGER_OUT); // finger out high, tell it to reset
        Delay_ms(500);
        PORTC &=  ~(1 << FINGER_OUT); // set it back low
    }
}
