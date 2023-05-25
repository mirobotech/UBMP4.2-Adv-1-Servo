/*==============================================================================
 Library: servo.c                       Activity: mirobo.tech/ubmp4-advanced-1
 Date: May 19, 2023
 
 UBMP4 demonstration servo library including functions to configure individual
 H1 - H8 header pins for servo output as well as to produce a servo pulse on a
 selected header output pin.
 =============================================================================*/

#include    "xc.h"              // Microchip XC8 compiler include file
#include    "stdint.h"          // Include integer definitions
#include    "stdbool.h"         // Include Boolean (true/false) definitions

#include    "UBMP420.h"         // Include UBMP4 constants and functions

// Enable output on the selected SERVO1 - SERVO8 (H1 - H8) header pin. Call
// this function for each servo output pin you would like enabled.
void servo_config(unsigned char servo)
{
    // Clear the servo output port pin before enabling the output tristate.
    LATC = LATC & ~servo;
    TRISC = TRISC & ~servo;
}

// Create a single servo pulse on the specified SERVO1 - SERVO8 output, with 
// the pulse length corresponding to the position value (0-255). Call this
// function to produce a pulse for each active servo every 15-20ms.
void servo_pulse(unsigned char servo, unsigned char position)
{
    LATC = LATC | servo;        // Set servo pin high and start fixed delay
    __delay_us(984);            // (change this value for 180 degree servos)
    for(position; position != 0; position--)   // Extend delay by position value
    {
        _delay(38);             // Clock cycle delay to make 1ms pulse (pos = 255)
    }                           // (change delay to modify the servo pulse length)
    LATC = LATC & ~servo;     // End servo pulse by clearing servo output pin
}
