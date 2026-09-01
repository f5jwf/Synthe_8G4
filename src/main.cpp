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


void setup() {
  // put your setup code here, to run once:
    pinMode(LED_LIVE, OUTPUT);
    digitalWrite(LED_LIVE, HIGH); // 
    Serial.begin(38400);
    SYNTHE_init();  //Init Synth
    SERIAL_init();  //Init CLI Serial
  
  
}

void loop() {
    // put your main code here, to run repeatedly:
 
    //digitalWrite(LED_LIVE, HIGH); 
    //delay(10);                 
    //digitalWrite(LED_LIVE, LOW);  
    SYNTHE_SEQUENCER();
    SERIAL_cmd_pool();
    if(U32_Time % 20 == 0) digitalWrite(LED_LIVE, !digitalRead(LED_LIVE)); // Toggle LED_LIVE
    U32_Time++;
    delay(40);

  
 }

