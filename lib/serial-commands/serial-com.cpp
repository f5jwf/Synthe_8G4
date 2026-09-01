/* *******************************************************************************
	NAME 			    : Serial Commands
	DESCRIPTION		  	: Synthe: Serial command	  
	AUTHOR			    : Philippe Borghini        
    COMMENTS            : Library from https://github.com/ppedro74/Arduino-SerialCommands             
-------------------------------------------------------------
DATE			VERSION	REVISOR 	DESCRIPTION				
--------------------------------------------------------------------------------
13.01.2025 	V0.1 	PHB		Initialization

-------------------------------------------------------------------------------- */

/* *****************************************************************************
 			     DECLARATIONS / DEFINITIONS	  		            
***************************************************************************** */

/* Platform Type Definitions. */
#include <Arduino.h>
#include <SPI.h>
#include <EEPROM.h>

#include <Wire.h>
//#include <Preferences.h>

/* Family Declaration(s). */
#include "synthe.h"
#include "serial-com.h"
//#include <SerialCommands.h>


/* External Declaration(s). */

/* Global functions prototypes */
 void SERIAL_init(void);
 void cmd_unrecognized(SerialCommands* sender, const char* cmd);
 void SERIAL_cmd_pool(void);

/* Local functions prototypes */

void cmd_help(SerialCommands* sender);
void cmd_set_freq(SerialCommands* sender);
void cmd_set_pll_n(SerialCommands* sender);
void cmd_set_pll_num(SerialCommands* sender);
void cmd_set_pll_den(SerialCommands* sender);
void cmd_set_power(SerialCommands* sender);
void cmd_set_reg(SerialCommands* sender);
void cmd_read_reg(SerialCommands* sender);
void cmd_read_allreg(SerialCommands* sender);
void cmd_reset_default(SerialCommands* sender);
void cmd_set_verbose(SerialCommands* sender);
void cmd_debug_0(SerialCommands* sender);
void cmd_debug_tool(SerialCommands* sender);

/* Global variables */
char serial_command_buffer_[32];


/* Local variables */


// Serial object declaration
//Creates SerialCommands object attached to Serial
//working buffer = serial_command_buffer_
//command delimeter: Cr & Lf
//argument delimeter: SPACE

SerialCommands serial_commands_(&Serial, serial_command_buffer_, sizeof(serial_command_buffer_), "\r", " ");
// "\r\n" with PlatformIO (CR + LF)
// "\r" with MobaXterm and Putty (CR only)

//Note: Commands are case sensitive
/*
      Serial.println(F("freq xxx          Set freq in [Hz]  (ex. 3590000 for 3.59G)")); //    
      Serial.println(F("pll_n xxx         Set N counter (19bits)")); // 
      Serial.println(F("pll_num xxx       Set Numerator counter (32bits)")); //             
      Serial.println(F("pll_den xxx       Set Denominator (32bits)")); //  */

SerialCommand cmd_set_freq_("freq", cmd_set_freq);
SerialCommand cmd_set_pll_n_("pll_n", cmd_set_pll_n);
SerialCommand cmd_set_pll_num_("pll_num", cmd_set_pll_num);
SerialCommand cmd_set_pll_den_("pll_den", cmd_set_pll_den);
SerialCommand cmd_set_power_("power", cmd_set_power);
SerialCommand cmd_set_reg_("set_reg", cmd_set_reg);
SerialCommand cmd_read_reg_("read_reg", cmd_read_reg);
SerialCommand cmd_read_allreg_("9", cmd_read_allreg,true);
SerialCommand cmd_reset_default_("reset_default", cmd_reset_default);
SerialCommand cmd_set_verbose_("verbose", cmd_set_verbose); 
SerialCommand cmd_debug_0_("0", cmd_debug_0, true); //Single touch
SerialCommand cmd_debug_tool_("debug", cmd_debug_tool); 

SerialCommand cmd_help_("?", cmd_help, true); //Single touch
/*******************************************************************************/
/* 			        		PROGRAM CODE	     			    */
/*******************************************************************************/


/*--------------------------------------------------------------------------------
Description    : void SERIAL_init(void)
                Init Serial Command object. All command have to be added below 
--------------------------------------------------------------------------------*/
 void SERIAL_init(void)
 {
    serial_commands_.SetDefaultHandler(cmd_unrecognized);
    serial_commands_.AddCommand(&cmd_set_freq_); 
    serial_commands_.AddCommand(&cmd_set_pll_n_); 
    serial_commands_.AddCommand(&cmd_set_pll_num_); 
    serial_commands_.AddCommand(&cmd_set_pll_den_); 
    serial_commands_.AddCommand(&cmd_set_power_);     
    serial_commands_.AddCommand(&cmd_set_reg_); 
    serial_commands_.AddCommand(&cmd_read_reg_);  
    serial_commands_.AddCommand(&cmd_read_allreg_);     
    serial_commands_.AddCommand(&cmd_reset_default_);      
    serial_commands_.AddCommand(&cmd_set_verbose_);
    serial_commands_.AddCommand(&cmd_debug_0_);     
    serial_commands_.AddCommand(&cmd_debug_tool_);        
    serial_commands_.AddCommand(&cmd_help_);

    Serial.println("Ready!");

 }

/*--------------------------------------------------------------------------------
Description    : void cmd_unrecognized(SerialCommands* sender, const char* cmd)
                print error message if command not recognized
                This is the default handler, and gets called when no other command matches. 
--------------------------------------------------------------------------------*/
void cmd_unrecognized(SerialCommands* sender, const char* cmd)
{
  sender->GetSerial()->print("Unrecognized command [");
  sender->GetSerial()->print(cmd);
  sender->GetSerial()->println("]");
}
/*--------------------------------------------------------------------------------
Description    : void SERIAL_cmd_pool(void);
                Pool serial interface for commands. 
--------------------------------------------------------------------------------*/
void SERIAL_cmd_pool(void)
{
    serial_commands_.ReadSerial();

}

//
void cmd_set_freq(SerialCommands* sender)
{
    char string[20];
    DOUBLEVAL param;

    char* param_str = sender->Next();
    if (param_str != NULL)	{
		    param.u32 = strtol(param_str,NULL,10);    //Convert decimal string into integer
            Serial.print(F("Freq set to: "));
            sprintf(string, "%lu", param.u32);
            Serial.println(string);
            SYNTHE_set_PLL(SET_FREQ, param.u32);
            EEPROM.put(DEFAULT_FREQ_ADD, param.u32);      
	    } else sender->GetSerial()->println("Error No Argument");


}

void cmd_set_pll_n(SerialCommands* sender)
{
    char string[15];
    DOUBLEVAL param;

    char* param_str = sender->Next();
    if (param_str != NULL)	{
		    param.u32 = strtol(param_str,NULL,10);    //Convert decimal string into integer
            Serial.print(F("PLL_N set to: "));
            sprintf(string, "%04X %04X", param.u16[1], param.u16[0]);
            Serial.println(string);
            SYNTHE_set_PLL(SET_PLL_N, param.u32);
            EEPROM.put(DEFAULT_PLL_N_ADD, param.u32);       
	    } else sender->GetSerial()->println("Error No Argument");


}

void cmd_set_pll_num(SerialCommands* sender)
{
    char string[15];
    DOUBLEVAL param;

    char* param_str = sender->Next();
    if (param_str != NULL)	{
		    param.u32 = strtol(param_str,NULL,10);    //Convert decimal string into integer
            Serial.print(F("PLL_NUM set to: "));
            sprintf(string, "%04X %04X", param.u16[1], param.u16[0]);
            Serial.println(string);
            SYNTHE_set_PLL(SET_PLL_NUM, param.u32);
            EEPROM.put(DEFAULT_PLL_NUM_ADD, param.u32);      
	    } else sender->GetSerial()->println("Error No Argument");


}

void cmd_set_pll_den(SerialCommands* sender)
{
    char string[15];
    DOUBLEVAL param;

    char* param_str = sender->Next();
    if (param_str != NULL)	{
		    param.u32 = strtol(param_str,NULL,10);    //Convert decimal string into integer
            Serial.print(F("PLL_DEN set to: "));
            sprintf(string, "%04X %04X", param.u16[1], param.u16[0]);
            Serial.println(string);
            SYNTHE_set_PLL(SET_PLL_DEN, param.u32);
            EEPROM.put(DEFAULT_PLL_DEN_ADD, param.u32);      
	    } else sender->GetSerial()->println("Error No Argument");
}

void cmd_set_power(SerialCommands* sender)
{
    char string[15];
    DOUBLEVAL param;
    param.u32 =0;
    char* param_str = sender->Next();
    if (param_str != NULL)	{
		    param.u32 = strtol(param_str,NULL,10);    //Convert decimal string into integer
            //param.u32 &= 0x3f;  //limit to 0...63 
            Serial.print(F("power set to: "));
            sprintf(string, "%u", param.u8[0]);
            Serial.println(string);
            SYNTHE_set_power(param.u32);
            EEPROM.put(DEFAULT_POWER_ADD, (U8)param.u8[0]);      
	    } else sender->GetSerial()->println("Error No Argument");


}
void cmd_set_reg(SerialCommands* sender)
{
    char string[20];
    U32 reg=0;
    U8 reg_nr=0;
    U16 reg_value=0;

    char* param_str = sender->Next();
    if (param_str != NULL)	{
		    reg = strtol(param_str,NULL,16);    //Convert 0xaaaa into integer 
            reg_value =reg & 0xffff; //keep 16bits LSB
            reg_nr = (reg>>16) & 0x7f; //keep the 7bits LSB
            Serial.print(F("SPI="));
            sprintf(string, "reg_nr=0x%02X, reg_value=0x%04X",reg_nr, reg_value);
            Serial.println(string);
            SYNTHE_prog_reg(reg_nr, reg_value);
            //EEPROM.put(DEFAULT_FREQ_ADD, freq);      
	    } else sender->GetSerial()->println("Error No Argument");


}


void cmd_read_reg(SerialCommands* sender)
{
    char string[15];
    U8 reg_nr=0;
    U16 reg_value;

    char* param_str = sender->Next();
    if (param_str != NULL)	{
		    reg_nr = strtol(param_str,NULL,16);    //Convert 0x10 into integer 
            Serial.print(F("Read SPI: "));
            sprintf(string, "reg_nr=%u",reg_nr);
            Serial.println(string);
            reg_value = SYNTHE_read_reg(reg_nr&0x7f) ;

            Serial.print(F("Read LMX: reg_nr=0x"));
            sprintf(string, "%02X",reg_nr);
            Serial.print(string);
            Serial.print(F(" reg_value=0x"));
            sprintf(string, "%04X", reg_value);
            Serial.println(string);
	    } else sender->GetSerial()->println("Error No Argument");


}


void cmd_set_verbose(SerialCommands* sender)
{
    char string[15];

    char* param_str = sender->Next();
    if (param_str != NULL)	{
		U8_Verbose = atoi(param_str);
        sprintf(string, "Verbose=%u\n\r", U8_Verbose);
        Serial.println(string);
	    } else sender->GetSerial()->println("Error No Argument");

}

void cmd_debug_tool(SerialCommands* sender)
{
    char string[15];
    U8 debug=0;

    char* param_str = sender->Next();
    if (param_str != NULL)	{
		debug = atoi(param_str);
        sprintf(string, "Debug=%u\n\r", U8_Debugg);
        Serial.println(string);
        SYNTHE_debug(debug);
	    } else sender->GetSerial()->println("Error No Argument");

}

void cmd_debug_0(SerialCommands* sender)
{
    /* //Send Request to SPI
    char string[10];
    U16 reg_value;
    U8 reg_nr=0x00;//Single touch read reg 0

    reg_value = SYNTHE_read_reg(reg_nr) ; //Single touch read reg 0
    Serial.print(F("Read LMX: reg_nr=0x"));
    sprintf(string, "%02X",reg_nr);
    Serial.print(string);
    Serial.print(F(" reg_value=0x"));
    sprintf(string, "%04X", reg_value);
    Serial.println(string); 
    */
    // LMX Init prefered
    /*
    Serial.println(F("Init LMX with prefered values"));
    SYNTHE_LMX25XX_init_prefered();
    */
uint8_t ee_address = 0;        // Adresse dans l'EEPROM
uint8_t data_to_write = 0x42;  // Exemple de donnée
uint8_t data_read = 0;         // Pour stocker la lecture



  // Écriture dans l'EEPROM
  eeprom_update_byte((uint8_t*)ee_address, data_to_write);  // écrit seulement si différent

  // Lecture depuis l'EEPROM
  data_read = eeprom_read_byte((uint8_t*)ee_address);

  Serial.print("Donnée lue dans l'EEPROM : 0x");
  Serial.println(data_read, HEX);


}

void cmd_read_allreg(SerialCommands* sender)
{
    SYNTHE_read_all_reg();
}

void cmd_reset_default(SerialCommands* sender)
{
    SYNTHE_reset_default();
}


//called Help display
void cmd_help(SerialCommands* sender)
{
     SYNTHE_help();  
}



