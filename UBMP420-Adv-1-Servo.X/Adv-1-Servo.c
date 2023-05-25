/*==============================================================================
 Project: Advanced-1-Servo              Activity: mirobo.tech/ubmp4-advanced-1
 Date:    May 19, 2023
 
 This advanced programming activity for the mirobo.tech UBMP4 demonstrates the
 use of a simple servo function library to generate analog servo pulses. Two
 different methods are used to repetitively generate servo pulses: the first
 uses a software delays to generate periodic pulses, and the second uses a
 hardware timer interrupt function to generate periodic servo pulses.
 
 Servo Operation
 The output position of an analog hobby servo is controlled by a variable width
 output pulse typically ranging between 1ms-2ms for 90° servos. Some types of
 wider output range servos (eg. 180° servos) may use different pulse widths --
 often 544us-2.4ms, or 0.5ms-2.5ms -- depending on the servo. This example servo
 library creates 1ms-2ms pulses for 90° servos and can easily be modified for
 other pulse widths. Servo pulses must be sent at regular intervals to ensure
 accurate servo position holding. While pulse rates can vary, servos are
 normally updated at rates between 50-60Hz, or approximately every 15-20ms.
==============================================================================*/

#include    "xc.h"              // Microchip XC8 compiler include file
#include    "stdint.h"          // Include integer definitions
#include    "stdbool.h"         // Include Boolean (true/false) definitions

#include    "UBMP420.h"         // Include UBMP4.2 constants and functions
#include    "servo.h"           // Include servo functions

// TODO Set linker ROM ranges to 'default,-0-7FF' under "Memory model" pull-down.
// TODO Set linker code offset to '800' under "Additional options" pull-down.

// Program variable definitions
unsigned char servo1Position = 128; // Set servo1 position to centre (~128)
unsigned char timer0Periods = 3;    // TMR0 interrupt periods x 5ms for 15ms servo delay


// Servo interrupt service routine (ISR) uses TMR0 interrupt to create servo
// frame delays and servo pulses. DO NOT CALL THIS INTERRUPT FUNCTION! Interrupt
// functions are automatically invoked for pre-configured hardware interrupts.
void __interrupt() servo_int(void)
{
	if(TMR0IF && TMR0IE)        // Validate Timer 0 as interrupt source
	{
        TMR0IF = 0;             // Reset TMR0 interrupt flag
        TMR0 = 21;              // Pre-set TMR0 offset for next 5 ms period
        timer0Periods --;       // Count down 5 ms timer periods
        if(timer0Periods == 0)  // Are all delay periods done?
        {
            timer0Periods = 3;  // Reset counter for next 15 ms servo delay
            servo_pulse(SERVO1, servo1Position); // Create new servo pulse
        }
	}
}

int main(void)
{
    OSC_config();               // Configure internal oscillator for 48 MHz
    UBMP4_config();             // Configure on-board UBMP4 I/O devices and
                                // configure TMR0 interrupt
    servo_config(SERVO1);       // Set header H1 (SERVO1) as a servo output
	// ei();                       // Enable global interrupts -- disable using di();
    
    while(1)
	{
        // Servo pulse timing test code. Comment this out for interrupt use.
        servo_pulse(SERVO1,servo1Position);
        
        // Read pushbuttons and adjust servo position
        if(SW3 == 0 && servo1Position > 0)
        {
            servo1Position --;
        }
        
        if(SW4 == 0 && servo1Position < 255)
        {
            servo1Position ++;
        }
        
        // Delay between servo pulses for servo control without interrupts
        __delay_ms(15);
        
        // Delay between pushbutton updates when using TMR0 interrupt
        // __delay_ms(4);
               
        // Activate bootloader if SW1 is pressed.
        if(SW1 == 0)
        {
            RESET();
        }
    }
}

/* Learn More - Program Analysis Activities
 * 
 * 1.   The header file defines each servo pin using bitmaps as shown in this
 *      example for SERVO1:

#define SERVO1      0b00000001      // Servo output on H1 header pins

 *      The servo_pulse() function receives a SERVO[n] bitmap definition and
 *      stores it in its local 'servo' variable. Explain how the SERVO[n]
 *      bitmaps are able to set specific register bits by using logical
 *      operators as shown in this code from the servo_pulse() function:

    LATC = LATC | servo;      // Set servo pin high and start fixed delay

 * 2.   Explain how the same SERVO[n] bitmaps that are used to set register
 *      bits can be used to clear register bits through the use of the logical
 *      operator shown in this statement from the servo_config() function:
 
    TRISC = TRISC & ~servo;
 
 * 3.   When using the servo program code without interrupts, the variable-
 *      length servo pulses plus the fixed time delay between the pulses will
 *      affect the servo frame duration and, by extension, the servo update
 *      frequency. What is the shortest and longest duration that the servo
 *      frame (from the start of one H1 servo pulse to the start of the next
 *      H1 servo pulse) will be? If you have access to an oscilloscope, you
 *      can confirm your prediction and verify the timing of the servo pulses.
 * 
 *      If the hardware and software were to be modified to output servo pulses
 *      on all eight PORTC pins (all H1-H8 header pins on UBMP4) in sequence,
 *      what are shortest and longest servo frame durations that you could
 *      expect to see being produced?
 * 
 * 4.   The servo_int() interrupt function is automatically triggered by the
 *      interrupt controller in the microcontroller in response to specific
 *      hardware events. The hardware used in this example is TMR0, an 8-bit
 *      timer module/counter module that can count clock cycles directly or
 *      using a pre-scaler (configured through the OPTION_REG register). When
 *      TMR0 overflows (from 255 to 0), an interrupt occurs if 1) the TMR0
 *      interrupt is enabled and, 2) hardware interrupts are globally enabled.
 *      Let's take a look at the code that enables the TMR0 interrupt in the
 *      UBMP4_config() function:
 
void UBMP4_config(void)
{
    OPTION_REG = 0b01010111;    // Enable port pull-ups, TMR0 internal, div-256
      .
      . (PORT configuration statements omitted to focus on TMR0 configuration)
      . 
    // Configure TMR0 interrupt for servo_int() function
    TMR0 = 21;                  // Pre-set TMR0 period for 5ms time-out (@48MHz)
    TMR0IF = 0;                 // Clear TMR0 interrupt flag
    TMR0IE = 1;                 // Enable TMR0 interrupt
}

 *      The Option register configuration statement sets the instruction cycle
 *      clock (1/4 of the 48 MHz oscillator clock frequency) as the internal
 *      TMR0 clock source, but sends it through a 256:1 pre-scaler before
 *      it is counted by the TMR0 register. The TMR0 register is pre-set to 21
 *      before the TMR0 interrupt flag is cleared and the interrupt is enabled.
 *      How can we be sure that TMR0 will overflow after exactly 5 ms in this
 *      configuration? Describe at least two ways to verify this timing.
 *      
 * 5.   The servo_int() interrupt function includes an extra __interrupt()
 *      specifier in its function declaration, which is necessary for the
 *      compiler to locate its code at the interrupt vector location in the
 *      microcontroller's program memory:
 
 void __interrupt() servo_int(void)

 *      There can only be one interrupt function in a program, and its tasks
 *      are to quickly determine the cause of an interrupt, act on it, and
 *      return processing to the other parts of the program. The interrupt
 *      function cannot return any data to, or accept data from, the rest of
 *      the program. Can you explain why?
 * 
 * 6.   Let's alter the program to use the interrupt function to generate both
 *      servo frames and servo pulses. As explained in activity 4, above, the
 *      TMR0 interrupt is configured and enabled in the UBMP4_config() function,
 *      but global interrupts have not been enabled, yet. Enable all interrupts
 *      by un-commenting the line of code containing the ei(); compiler macro:
 
    ei();                       // Enable global interrupts -- disable using di();

 *      Next, disable the servo pulse test code by commenting out the
 *      servo_pulse() function in the main code (but not in the servo_int()
 *      function):

        // Servo pulse timing test code. Comment this out for interrupt use.
        // servo_pulse(SERVO1,servo1Position);

 *      Last, to prove that the interrupt is actually generating the complete
 *      servo frames and to speed up the pushbutton response, swap the active
 *      loop delays in the program code by commenting out the 15ms delay and
 *      un-commenting the 4ms delay:

        // Delay between servo pulses for servo control without interrupts
        // __delay_ms(15);
        
        // Delay between pushbutton updates when using TMR0 interrupt
        __delay_ms(4);

 *      Monitor the output of your program using an oscilloscope to verify
 *      that the servo pulses are 1-2ms in duration and that servo frames
 *      repeat exactly every 15ms. The interrupt-generated servo frames will
 *      not change in duration as the servo pulses change in width, and are
 *      generated independently of the code running in the main while loop.
 * 
 *      The ei(); compiler macro was used to enable global interrupts, allowing
 *      all pre-enabled interrupt sources to generate interrupts. Global
 *      interrupts are automatically disabled by compiler-added code when a
 *      properly-specified __interrupt() function runs, and re-enabled when
 *      the function exits and control returns to the main code. Why is it
 *      necessary for interrupts to be disabled during interrupt service
 *      routines? What other kinds of code might require interrupts to be
 *      temporarily disabled?
 * 
 * 7.   The pushbuttons are able to use an interrupt-on-change (IOC) interrupt
 *      that can not only can respond to key-press events without repeatedly
 *      polling the switches, but can also wake the PIC16F1459 microcontoller
 *      from its SLEEP state. SLEEP enables programs such as a TV remote control
 *      transmitter or Simon-style memory game to go into a low power state
 *      while waiting for user input and resume processing nearly instantly
 *      after a button is pressed. Unfortunately, SLEEP won't work for this
 *      servo program since the TMR0 timer also stops when the microcontroller
 *      clock stops, but we can still explore the use of IOC interrupts for
 *      instantaneous response to input events.
 * 
 *      First, start by configuring IOC on selected PORTB pins by adding the
 *      following code to the UBMP4_config() function in the UBMP420.c file:

    // Configure PORTB IOC (Interrupt-on-Change) interrupt
    IOCBF = 0;                  // Clear all PORTB IOC flags
    IOCBN = 0b10010000;         // Enable negative-edge IOC for SW2 and SW5
    IOCIF = 0;                  // Clear IOC interrupt flag
    IOCIE = 1;                  // Enable IOC interrupts

 *      This IOC configuration enables SW2 and SW5 to generate interrupts on
 *      the negative-going edge of the switch pin, corresponding to a key press.
 * 
 *      Next, add the following program code to the servo_int() function, after
 *      the existing TMR0 service code, to service the IOC interrupts:

    if(IOCIF && IOCIE)          // Validate IOC as interrupt source
    {
        if(IOCBF7)              // Check for SW5 (PORTB.7) interrupt
        {
            IOCBF7 = 0;         // Reset IOC SW5 interrupt flag
            servo1Position = 255;
        }
        if(IOCBF4)              // Check for SW2 (PORTB.4) interrupt
        {
            IOCBF4 = 0;         // Reset IOC SW2 interrupt flag
            servo1Position = 0;
        }
        IOCIF = 0;              // Reset IOC flag
    }

 *      Last, disable both the 15ms and 4ms delays in the main function, and
 *      add this longer delay to simulate a slow-running process:

        LED1 = !LED1;           // Simulate other processing by toggling LED1
        __delay_ms(50);

 *      Try the code in your circuit. Pressing and holding either SW4 or SW3
 *      still increases or decreases the length of the servo pulse, but very
 *      slowly due to the 50ms long delay in the while loop. Pressing SW2 or
 *      SW5 instantly changes the value of the servo1Position variable through
 *      the IOC interrupt, and the very next servo pulse created by the TMR0
 *      interrupt will be new length set by servo1Position. Servo pulses are
 *      created completely independently of the code in the main while loop
 *      (at least three are generated during the time that LED D1 is either on,
 *      or off) and this gives the program the appearance of multi-tasking!
 */