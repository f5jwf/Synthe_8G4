/********************************************************************************
	NAME 			      : main.cpp
	EXTENDED NAME		: 
	LAYER		      	: High Level 
	DESCRIPTION		  : 
	                  
	AUTHOR			    : Philippe Borghini			  
  COMMENTS        :
                

REVISIONS :
--------------------------------------------------------------------------------
DATE			VERSION	REVISOR 	DESCRIPTION				
--------------------------------------------------------------------------------
23.01.2024 	V0.1 	PHB		Initialization

--------------------------------------------------------------------------------

********************************************************************************/

/*******************************************************************************/
/* 			     DECLARATIONS / DEFINITIONS			  		         */
/*******************************************************************************/

//#include <Arduino.h>
#include "tdef.h"
#include <SPI.h>
#include <SerialCommands.h>

/* Family Declaration(s). */

#include "synthe.h"
#include "serial-com.h"



/* External Declaration(s). */

/* Global functions prototypes */

/* Local functions prototypes */


int test =0;
static bool modbus_mode = true;
static unsigned long last_led_toggle_ms = 0;


void setup() {
  // put your setup code here, to run once:
    pinMode(LED_LIVE, OUTPUT);
    digitalWrite(LED_LIVE, HIGH); // 
    pinMode(SERIAL_MODE_SELECT, INPUT_PULLUP);
    pinMode(LOCK_DETECT, INPUT);
    modbus_mode = (digitalRead(SERIAL_MODE_SELECT) == HIGH);
    Serial.begin(38400);
    SYNTHE_init(modbus_mode);  //Init Synth and selected communication mode
    if (!modbus_mode) SERIAL_init();
  
  
}

void loop() {
    // put your main code here, to run repeatedly:
 
    if (modbus_mode) SYNTHE_modbus_publish();
    else SERIAL_cmd_pool();
    SYNTHE_SEQUENCER();

    const unsigned long now = millis();
    if (now - last_led_toggle_ms >= 500UL) {
      last_led_toggle_ms = now;
      digitalWrite(LED_LIVE, !digitalRead(LED_LIVE));
    }
    U32_Time = now;

  
 }

