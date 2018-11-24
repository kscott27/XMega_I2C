//*************************************************************************************
/** \file lab1_main.cpp
 *    This file contains the main() code for a program which runs a port of the FreeRTOS
 *    for AVR devices. This port is specific to the XMEGA family.
 *
 *  Revisions:
 *    \li 09-14-2017 CTR Adapted from JRR code for AVR to be compatible with xmega 
 *
 *  License:
 *    This file is released under the Lesser GNU Public License, version 2. This 
 *    program is intended for educational use only, but it is not limited thereto. 
 */
//*************************************************************************************



#include <stdlib.h>                         // Prototype declarations for I/O functions
#include <avr/io.h>                         // Port I/O for SFR's
#include <avr/wdt.h>                        // Watchdog timer header
#include <avr/interrupt.h>
#include <string.h>                         // Functions for C string handling

#include "FreeRTOS.h"                       // Primary header for FreeRTOS
#include "task.h"                           // Header for FreeRTOS task functions
#include "queue.h"                          // FreeRTOS inter-task communication queues
#include "croutine.h"                       // Header for co-routines and such

#include "rs232int.h"                       // ME405/507 library for serial comm.
#include "time_stamp.h"                     // Class to implement a microsecond timer
#include "frt_task.h"                       // Header of wrapper for FreeRTOS tasks
#include "frt_text_queue.h"                 // Wrapper for FreeRTOS character queues
#include "frt_queue.h"                      // Header of wrapper for FreeRTOS queues
#include "frt_shared_data.h"                // Header for thread-safe shared data
#include "shares.h"                         // Global ('extern') queue declarations

#include "I2CMaster.h"
#include "MB1202.h"
#include "MMA8451.h"
#include "I2CAgent.h"

#include "task_user.h"                      // Header for user interface task
#include "task_sensor.h"

volatile int counter;
frt_text_queue print_ser_queue (32, NULL, 10);
frt_queue<uint16_t> time_queue(255,NULL,10);
frt_queue<int16_t> ang_queue(255,NULL,10);
frt_queue<int16_t> A_queue(255,NULL,10);
frt_queue<int16_t> B_queue(255,NULL,10);
frt_queue<int16_t> C_queue(255,NULL,10);

// Electrical phase angle
volatile int16_t angle = 0;

// Effort is a variable that describes the percentage of maximum duty cycle for each PWM channel; It is the "effective" 3-phase pwm duty cycle
int16_t effort = 255;
	
// delta_angle is the change in phase angle to be taken for each iteration of the task
int16_t delta_angle = 0; // (1) (1 degree) / (1/10 ms) -> (6) (10,000 degrees) / (1 s) ->  (6) (10,000*60/360 rev/min) -> (6) (166.67 rpm) = 1000 rpm
	

/*! \brief CCP write helper function written in assembly.
 *
 *  This function is written in assembly because of the time critical
 *  operation of writing to the registers.
 *
 *  \param address A pointer to the address to write to.
 *  \param value   The value to put in to the register.
 */
void CCPWrite( volatile uint8_t * address, uint8_t value )
{
	#if defined __GNUC__
	uint8_t volatile saved_sreg = SREG;
	cli();
	volatile uint8_t * tmpAddr = address;
	#ifdef RAMPZ
	RAMPZ = 0;
	#endif
	asm volatile(
	"movw r30,  %0"	      "\n\t"
	"ldi  r16,  %2"	      "\n\t"
	"out   %3, r16"	      "\n\t"
	"st     Z,  %1"       "\n\t"
	:
	: "r" (tmpAddr), "r" (value), "M" (0xD8), "i" (&CCP)
	: "r16", "r30", "r31"
	);

	SREG = saved_sreg;
	#endif
}


//=====================================================================================
/** The main function sets up the RTOS.  Some test tasks are created. Then the 
 *  scheduler is started up; the scheduler runs until power is turned off or there's a 
 *  reset.
 *  @return This is a real-time microcontroller program which doesn't return. Ever.
 */

int main (void)
{
	cli();
	// Configure the system clock
	{	
		// Enable the 32MHz internal RC oscillator and the external 32KHz oscillator
		OSC.CTRL |= (1 << OSC_RC32MEN_bp);
		do {} while((OSC.STATUS & (1 << OSC_RC32MRDY_bp)) != (1 << OSC_RC32MRDY_bp));
		
		// Configure the XOSC for 32.768KHz crystal
//		OSC.XOSCCTRL =  OSC_XOSCSEL_32KHz_gc;
//		OSC.CTRL |= (1<< OSC_XOSCEN_bp);
	
		// Wait for a stable clock on both the internal and external oscillators
		// do {} while((OSC.STATUS & 1 << OSC_RC32MRDY_bp | OSC.STATUS & 1 << OSC_XOSCRDY_bp) != (OSC.STATUS & 1 << OSC_RC32MRDY_bp | OSC.STATUS & 1 << OSC_XOSCRDY_bp));
		// do {} while((OSC.STATUS & (1 << OSC_RC32MRDY_bp | 1 << OSC_XOSCRDY_bp)) != (1 << OSC_RC32MRDY_bp | 1 << OSC_XOSCRDY_bp));
//		do {} while((OSC.STATUS & (1 << OSC_XOSCRDY_bp)) != (1 << OSC_XOSCRDY_bp));

		// Select the 32.768KHz crystal as a calibration source
//		OSC.DFLLCTRL = OSC_RC32MCREF_XOSC32K_gc | OSC_RC2MCREF_XOSC32K_gc;
	
		// Enable the DFLL auto-calibration
//		DFLLRC32M.CTRL = (1 << DFLL_ENABLE_bp);
//		DFLLRC2M.CTRL = (1 << DFLL_ENABLE_bp);

		// Select the clock
		CCPWrite(&(CLK.CTRL),((CLK.CTRL & ~CLK_SCLKSEL_gm) | (1 << CLK_SCLKSEL0_bp)));
		// Enable the RTC as an external oscillator
		//CLK.RTCCTRL = (CLK_RTCSRC_TOSC_gc | CLK_RTCEN_bm);
		// CCPWrite(&(CLK.RTCCTRL),(CLK_RTCSRC_TOSC_gc | CLK_RTCEN_bm));
		
		// Disable the 2MHz internal RC oscillator
		OSC.CTRL &= ~(1 << OSC_RC2MEN_bp);
	}
	
	// Disable the watchdog timer unless it's needed later. This is important because
	// sometimes the watchdog timer may have been left on...and it tends to stay on	 
	wdt_disable ();


	// Configure a serial port which can be used by a task to print debugging infor-
	// mation, or to allow user interaction, or for whatever use is appropriate.  The
	// serial port will be used by the user interface task after setup is complete and
	// the task scheduler has been started by the function vTaskStartScheduler()
	rs232 ser_dev(0,&USARTE0); // Create a serial device on USART E0
	ser_dev << clrscr << "FreeRTOS Xmega Testing Program" << endl << endl;
	
	// Create instance of I2C driver to be used by sensor objects
	I2CMaster i2c(&TWIE, 62000, &ser_dev);
	
	// Create instance of IMU class and pass in a pointer to the I2C driver
	MMA8451 mma8451(&i2c, &ser_dev);
	
	// The user interface is at low priority; it could have been run in the idle task
	// but it is desired to exercise the RTOS more thoroughly in this test program
	new task_user ("UserInt", task_priority (0), 128, &ser_dev);
	
	new task_sensor ("Sensor", task_priority (2), 128, &ser_dev, &mma8451);
	
	// Enable high level interrupts and global interrupts
	PMIC_CTRL = (1 << PMIC_HILVLEN_bp | 1 << PMIC_MEDLVLEN_bp | 1 << PMIC_LOLVLEN_bp);
	sei();
	
	// Here's where the RTOS scheduler is started up. It should never exit as long as
	// power is on and the microcontroller isn't rebooted
	vTaskStartScheduler ();
}

