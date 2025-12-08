/*
 * File:   ADC+PIN_test.c
 * Author: yongwoo
 *
 * Created on November 17, 2025, 5:35 PM
 * 
 * Note: for whatever reason, with circuit set up with these pins
 * all switches must be HIGH (closed) to flash on the ATMega328PB
 * 
 */
#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include "uart.h"

#define ADC_TOLERANCE 100  // Tolerance for ADC value matching

// PIN configuration
#define PIN_LENGTH 4

// Password structure for ADC and switch requirements
typedef struct {
    uint16_t adc0_target;
    uint16_t adc1_target;
    uint16_t adc2_target;
    uint8_t sw0_state;  // Expected state: 0 (LOW) or 1 (HIGH)
    uint8_t sw1_state;
    uint8_t sw2_state;
} Password;

void adc_init(void) {
    // Set PC0, PC1, PC2 as inputs
    DDRC &= ~((1 << PC0) | (1 << PC1) | (1 << PC2));
    
    // Set reference voltage to AVCC
    ADMUX = (1 << REFS0);
    
    // Enable ADC with prescaler of 128 (16MHz/128 = 125kHz)
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

void switch_init(void) {
    // Set PB2, PB3, PB4 as inputs
    DDRB &= ~((1 << PB2) | (1 << PB3) | (1 << PB4));
    
    // Enable internal pull-up resistors on PB2, PB3, PB4
    PORTB |= (1 << PB2) | (1 << PB3) | (1 << PB4);
}

uint16_t adc_read(uint8_t channel) {
    // Select ADC channel (0-2 for PC0-PC2)
    // Clear lower 3 bits of ADMUX and set channel
    ADMUX = (ADMUX & 0xF8) | (channel & 0x07);
    
    // Wait for the multiplexer to settle
    _delay_us(10);
    
    // Start conversion
    ADCSRA |= (1 << ADSC);
    
    // Wait for conversion to complete
    while (ADCSRA & (1 << ADSC));
    
    // Return ADC value
    return ADC;
}

uint8_t switch_read(uint8_t pin) {
    // Read pin state (returns 0 if pressed/low, 1 if not pressed/high)
    return (PINB & (1 << pin)) ? 1 : 0;
}

void password_init(Password* pwd, uint16_t adc0, uint16_t adc1, uint16_t adc2,
                   uint8_t sw0, uint8_t sw1, uint8_t sw2) {
    pwd->adc0_target = adc0;
    pwd->adc1_target = adc1;
    pwd->adc2_target = adc2;
    pwd->sw0_state = sw0;
    pwd->sw1_state = sw1;
    pwd->sw2_state = sw2;
}

uint8_t password_check(Password* pwd, uint16_t adc0, uint16_t adc1, uint16_t adc2,
                       uint8_t sw0, uint8_t sw1, uint8_t sw2) {
    // Check if switches match
    if (sw0 != pwd->sw0_state || sw1 != pwd->sw1_state || sw2 != pwd->sw2_state) {
        return 0;
    }
    
    // Check if ADC values are within tolerance
    if (abs(adc0 - pwd->adc0_target) > ADC_TOLERANCE) return 0;
    if (abs(adc1 - pwd->adc1_target) > ADC_TOLERANCE) return 0;
    if (abs(adc2 - pwd->adc2_target) > ADC_TOLERANCE) return 0;
    
    return 1;  // Password matches!
}

// Keypad pins: Rows: PB1,PD2,PD3,PD4  Cols: PD5,PD6,PD7,PB0
void keypad_init(void) {
    // Configure columns as inputs with pullups
    // Cols: PD5=COL1, PD6=COL2, PD7=COL3, PB0=COL4
    DDRD &= ~((1 << PD5) | (1 << PD6) | (1 << PD7));
    PORTD |= (1 << PD5) | (1 << PD6) | (1 << PD7);
    
    DDRB &= ~(1 << PB0);
    PORTB |= (1 << PB0);
    
    // Configure rows as outputs driving HIGH
    // Rows: PB1=ROW1, PD2=ROW2, PD3=ROW3, PD4=ROW4
    DDRB |= (1 << PB1);
    PORTB |= (1 << PB1);
    
    DDRD |= (1 << PD2) | (1 << PD3) | (1 << PD4);
    PORTD |= (1 << PD2) | (1 << PD3) | (1 << PD4);
}

int keypad_read(void) {
    // Scan rows by driving them low one at a time
    // Row mapping: PB1=ROW1, PD2=ROW2, PD3=ROW3, PD4=ROW4
    
    for (int row = 0; row < 4; row++) {
        // Set all rows HIGH first
        PORTB |= (1 << PB1);
        PORTD |= (1 << PD2) | (1 << PD3) | (1 << PD4);
        
        // Drive current row LOW based on which row we're scanning
        if (row == 0) PORTB &= ~(1 << PB1);  // ROW1
        else if (row == 1) PORTD &= ~(1 << PD2);  // ROW2
        else if (row == 2) PORTD &= ~(1 << PD3);  // ROW3
        else if (row == 3) PORTD &= ~(1 << PD4);  // ROW4
        
        _delay_us(10);  // Small delay for signal to settle
        
        // Read columns (active low with pullups)
        unsigned char col1 = (PIND & (1 << PD5)) ? 0 : 1;  // COL1
        unsigned char col2 = (PIND & (1 << PD6)) ? 0 : 1;  // COL2
        unsigned char col3 = (PIND & (1 << PD7)) ? 0 : 1;  // COL3
        unsigned char col4 = (PINB & (1 << PB0)) ? 0 : 1;  // COL4
        
        // Check which column is pressed
        if (col1 || col2 || col3 || col4) {
            // Return all rows to HIGH
            PORTB |= (1 << PB1);
            PORTD |= (1 << PD2) | (1 << PD3) | (1 << PD4);
            
            // Map row index to physical row
            // We scan PB1,PD2,PD3,PD4 in order (row 0-3)
            // These correspond to physical ROW1-4 directly
            int physical_row = row;
            
            // Keypad layout mapping
            //      COL1  COL2  COL3  COL4
            // ROW1:  1     2     3     A (10)
            // ROW2:  4     5     6     B (11)
            // ROW3:  7     8     9     C (12)
            // ROW4:  *     0     #     D (13)
            
            if (physical_row == 0 && col1) return 1;
            if (physical_row == 0 && col2) return 2;
            if (physical_row == 0 && col3) return 3;
            if (physical_row == 0 && col4) return 10;  // A
            
            if (physical_row == 1 && col1) return 4;
            if (physical_row == 1 && col2) return 5;
            if (physical_row == 1 && col3) return 6;
            if (physical_row == 1 && col4) return 11;  // B
            
            if (physical_row == 2 && col1) return 7;
            if (physical_row == 2 && col2) return 8;
            if (physical_row == 2 && col3) return 9;
            if (physical_row == 2 && col4) return 12;  // C
            
            if (physical_row == 3 && col1) return 14;  // *
            if (physical_row == 3 && col2) return 0;
            if (physical_row == 3 && col3) return 15;  // #
            if (physical_row == 3 && col4) return 13;  // D
        }
    }
    
    // Return all rows to HIGH
    PORTB |= (1 << PB1);
    PORTD |= (1 << PD2) | (1 << PD3) | (1 << PD4);
    
    return -1;  // No key pressed
}

void setup_inputs(void)
{
    // Set PC3, PC4, PC5 as inputs
    DDRC &= ~((1 << PC3) | (1 << PC4) | (1 << PC5));
    
    // Disable pull-ups (optional)
    PORTC &= ~((1 << PC3) | (1 << PC4) | (1 << PC5));
}

// function that checks state of progress from other MCU
uint8_t read_inputs(void)
{
    uint8_t pin_state = PINC;  // Read entire port once (best practice)

    uint8_t BIT0_high     = (pin_state & (1 << PC3))     != 0;
    uint8_t BIT1_check_high = (pin_state & (1 << PC4)) != 0;
    uint8_t BIT2_check_high = (pin_state & (1 << PC5)) != 0;

    // For debugging:
    // if (comms_high)     uart_tx("COMMS HIGH\n");
    // if (pin_check_high) uart_tx("PIN HIGH\n");

    // You can return a bitmask or handle individually
    return (BIT2_check_high << 2) | (BIT1_check_high << 1) | (BIT0_high);
}

void comms_output_init(void){
    // Setup output communicators (PC3, PC4, PC5)
    DDRC |= (1 << PC3) | (1 << PC4) | (1 << PC5);
    PORTC &= ~((1 << PC3) | (1 << PC4) | (1 << PC5)); // Turn off communicators, state 0
}

/*
0 - ADC wrong
1 - ADC correct
2 - wait for pin value
3 - pin value entered
4 - pin restart
5 - wrong
6 - correct
7 - LOCK/RESET
*/
void talk_to_LCD(uint8_t state){ // PC3 = bit0, PC4 = bit1, PC5 = bit2. Parallel 3 bit comms. SENDING TO LCD
    // Clear PC3, PC4, PC5
    PORTC &= ~((1 << PC3) | (1 << PC4) | (1 << PC5));

    // Mask state to 3 bits (0–7) and shift into PC3–PC5
    PORTC |= (state & 0x07) << PC3;
    printf("State:%u \r\n",state);
}

int main(void) {
    // Initialize UART
    uart_init();
    
    while (1) {
        // ========== IDENTITY VERIFICATION PHASE ==========
        
        uint8_t identity_verified = 0;
        uint8_t state = 8;
        
        while (!identity_verified){
            setup_inputs(); // PC3, PC4, PC5 as inputs
            state = read_inputs();
            printf("waiting for identity\r\n");
            if (state == 1){
                identity_verified = 1;
                printf("identity confirmed: jeevan\r\n");
            }
            if (state == 2){
                identity_verified = 1;
                printf("identity confirmed: yongwoo\r\n");
            }
            if (state == 3){
                identity_verified = 1;
                printf("identity confirmed: tomas\r\n");
            }
        }
        
        // ========== INITIALIZE SECURITY SYSTEM ==========
        // Initialize peripherals
        adc_init();
        switch_init();
        keypad_init();
        
        // Setup output communicators (PC3, PC4, PC5)
        comms_output_init();
        _delay_ms(4000); // delay while screen initializes opening state
        
        // Create password format: ADC0, ADC1, ADC2, SW0, SW1, SW2
        Password myPassword;
        // Pin created in if statements
        uint8_t correct_pin[PIN_LENGTH];
        
        if (state == 1){ // JEEVAN'S PASSWORD
            password_init(&myPassword, 512, 768, 256, 0, 1, 0);
        
            correct_pin[0] = 6;
            correct_pin[1] = 9;
            correct_pin[2] = 6;
            correct_pin[3] = 9;
        }
        if (state == 2){ // YONGWOO'S PASSWORD
            password_init(&myPassword, 512, 768, 256, 0, 1, 0);
        
            correct_pin[0] = 1;
            correct_pin[1] = 2;
            correct_pin[2] = 3;
            correct_pin[3] = 4;
        }
        if (state == 3){ // TOMAS' PASSWORD
            password_init(&myPassword, 512, 768, 256, 0, 1, 0);
        
            correct_pin[0] = 1;
            correct_pin[1] = 2;
            correct_pin[2] = 3;
            correct_pin[3] = 4;
        }

        
        printf("Security System Initialized!\r\n");
        printf("Target: ADC0=%u, ADC1=%u, ADC2=%u, SW0=%s, SW1=%s, SW2=%s\r\n\r\n",
               myPassword.adc0_target, myPassword.adc1_target, myPassword.adc2_target,
               myPassword.sw0_state ? "HIGH" : "LOW",
               myPassword.sw1_state ? "HIGH" : "LOW",
               myPassword.sw2_state ? "HIGH" : "LOW");
        
        uint8_t entered_pin[PIN_LENGTH];
        uint8_t pin_index = 0;
        uint8_t conditions_met = 0;
        uint8_t pin_verified = 0;
        int last_key = -1;
        uint16_t adc0, adc1, adc2;
        uint8_t sw0, sw1, sw2;
        
        // ========== SECURITY CHECK LOOP ==========
        while (identity_verified) {
            // Read ADC and switch values
            adc0 = adc_read(0);  // PC0
            adc1 = adc_read(1);  // PC1
            adc2 = adc_read(2);  // PC2
            sw0 = switch_read(PB2);
            sw1 = switch_read(PB3);
            sw2 = switch_read(PB4);
            
            // Stage 1: Check ADC and switch conditions
            if (!conditions_met) {
                // Print current values
                printf("Current: ADC0=%4u, ADC1=%4u, ADC2=%4u | SW0=%s, SW1=%s, SW2=%s",
                       adc0, adc1, adc2,
                       sw0 ? "HIGH" : "LOW",
                       sw1 ? "HIGH" : "LOW",
                       sw2 ? "HIGH" : "LOW");
                
                // Check if password conditions are met
                if (password_check(&myPassword, adc0, adc1, adc2, sw0, sw1, sw2)) {
                    talk_to_LCD(1);
                    conditions_met = 1;
                    pin_index = 0; // Reset PIN entry
                    printf(" -> *** STAGE 1 PASSED! LED1 ON ***\r\n");
                    _delay_ms(1500);
                } else {
                    talk_to_LCD(0);
                    printf(" -> Access Denied\r\n");
                }
            }
            
            // Stage 2: PIN entry (only if conditions are met and PIN not yet verified)
            if (conditions_met && !pin_verified) {
                talk_to_LCD(2);
                // Continuously check if ADC/switch conditions are still valid
                if (!password_check(&myPassword, adc0, adc1, adc2, sw0, sw1, sw2)) {
                    // Conditions lost, reset everything
                    talk_to_LCD(0);
                    conditions_met = 0;
                    pin_verified = 0;
                    pin_index = 0;
                    last_key = -1;
                    printf("*** CONDITIONS LOST! SYSTEM RESET ***\r\n\r\n");
                    continue;
                }
                
                int key = keypad_read();
                
                // Only process key if it's different from last reading (debounce)
                if (key != -1 && key != last_key) {
                    // Handle special keys
                    if (key == 14) {  // * key - reset
                        talk_to_LCD(4);
                        pin_index = 0;
                        printf("PIN entry reset.\r\n");
                    }
                    else if (key == 15) {  // # key - enter
                        if (pin_index == PIN_LENGTH) {
                            uint8_t match = 1;
                            for (uint8_t i = 0; i < PIN_LENGTH; i++) {
                                if (entered_pin[i] != correct_pin[i]) {
                                    match = 0;
                                    break;
                                }
                            }
                            
                            if (match) {
                                talk_to_LCD(6);
                                pin_verified = 1;
                                printf("*** PIN CORRECT! STAGE 2 PASSED! LED2 ON ***\r\n");
                                printf("*** FULL ACCESS GRANTED! ***\r\n\r\n");
                            } else {
                                // Wrong PIN, reset entry and blink
                                printf("Wrong PIN! Try again.\r\n");
                                pin_index = 0;
                                talk_to_LCD(5);
                                _delay_ms(5000);
                                talk_to_LCD(4);
                                _delay_ms(100);
                            }
                        } else {
                            // Not enough digits entered, reset
                            printf("Incomplete PIN (only %u digits). Resetting.\r\n", pin_index);
                            talk_to_LCD(4);
                            pin_index = 0;
                        }
                    }
                    else if (key >= 0 && key <= 9) {  // Regular digit (0-9)
                        if (pin_index < PIN_LENGTH) {
                            entered_pin[pin_index] = key;
                            pin_index++;
                            printf("Key pressed: %u (Total: %u/%u)\r\n", key, pin_index, PIN_LENGTH);
                            talk_to_LCD(3);
                            _delay_ms(100);
                        }
                    }
                }
                last_key = key;
            }
            
            // Stage 3: System is unlocked - wait for lock command
            if (pin_verified) {
                int key = keypad_read();

                if (key != -1 && key != last_key) {
                    // If * (Key 14) is pressed while unlocked
                    if (key == 14) {
                        printf("Locking System...\r\n");

                        // Send State 7 (LOCK)
                        talk_to_LCD(7);

                        _delay_ms(2000); // wait for servo to close

                        // Reset identity_verified to break out and restart identity verification
                        identity_verified = 0;
                    }
                }
                last_key = key;
            }
            
            _delay_ms(50); // Debounce delay
        }
        
        // System has been locked, loop will restart identity verification
        printf("*** SYSTEM LOCKED - RESTARTING IDENTITY VERIFICATION ***\r\n\r\n");
    }
    
    return 0;
}