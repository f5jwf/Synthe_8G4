/* *******************************************************************************
	NAME 			    : serial-com.h
	DESCRIPTION		  	: Synthe: Serial command	  
	AUTHOR			    : Philippe Borghini        
  	COMMENTS        	:
--------------------------------------------------------------------------------
DATE			VERSION	REVISOR 	DESCRIPTION				
--------------------------------------------------------------------------------
13.01.2025 	V0.1 	PHB		Initialization

-------------------------------------------------------------------------------- */
#include <SerialCommands.h>

/* Global variables */
extern char serial_command_buffer_[32];

/* Global prototypes decalaration */
extern  void SERIAL_init(void);
extern void cmd_unrecognized(SerialCommands* sender, const char* cmd);
extern void SERIAL_cmd_pool(void);
