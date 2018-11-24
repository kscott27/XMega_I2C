/*
 * task_thermocouple.h
 *
 * Created: 2/17/2018 8:31:33 PM
 *  Author: Kevin
 */ 


#ifndef TASK_SONAR_H_
#define TASK_SONAR_H_

#include <stdlib.h>                         // Prototype declarations for I/O functions

#include "FreeRTOS.h"                       // Primary header for FreeRTOS
#include "task.h"                           // Header for FreeRTOS task functions
#include "queue.h"                          // FreeRTOS inter-task communication queues

#include "ansi_terminal.h"

#include "rs232int.h"                       // ME405/507 library for serial comm.
#include "time_stamp.h"                     // Class to implement a microsecond timer
#include "frt_task.h"                       // Header for ME405/507 base task class
#include "frt_queue.h"                      // Header of wrapper for FreeRTOS queues
#include "frt_text_queue.h"                 // Header for a "<<" queue class
#include "frt_shared_data.h"                // Header for thread-safe shared data

#include "shares.h"                         // Global ('extern') queue declarations

#include "MMA8451.h"


/// This macro defines a string that identifies the name and version of this program. 
#define PROGRAM_VERSION		PMS ("ME405 base radio program V0.4 ")


//-------------------------------------------------------------------------------------
/** This task interacts with the user for force him/her to do what he/she is told. What
 *  a rude task this is. Then again, computers tend to be that way; if they're polite
 *  with you, they're probably spying on you. 
 */

class task_sonar : public frt_task
{
private:
	// No private variables or methods for this class

protected:

  uint16_t rangeReading_;
	MMA8451 * mma8451_;
	
	// This method displays a simple help message telling the user what to do. It's
	// protected so that only methods of this class or possibly descendents can use it
	void print_help_message (void);

	// This method displays information about the status of the system
	void show_status (void);

public:

    const char* task_name;

	// This constructor creates a user interface task object
	task_sonar (const char*, unsigned portBASE_TYPE, size_t, emstream*, MMA8451* mma8451);

	/** This method is called by the RTOS once to run the task loop for ever and ever.
	 */
	void run (void);
};



#endif /* TASK_SONAR_H_ */