/* *******************************************************************************
NAME 			: synthe.cpp
DESCRIPTION		: Synthe Main part related to Synthe  programming	
                  : Programming LMX2572 or LMX2592
AUTHOR	      : Philippe Borghini        
COMMENTS          : Use Library
                        - MODBUS: https://github.com/andresarmento/modbus-arduino/tree/master/libraries/ModbusSerial
                        - Serial-com: https://github.com/ppedro74/Arduino-SerialCommands   
--------------------------------------------------------------------------------
DATE			VERSION	REVISOR 	DESCRIPTION				
--------------------------------------------------------------------------------
13.01.2025 	V0.1 	PHB		Initialization
24.11.2025 V0.4   PHB         Modif code to incluse Synt version LMX2592

-------------------------------------------------------------------------------- */

/* *****************************************************************************
 			     DECLARATIONS / DEFINITIONS	  		            
***************************************************************************** */

/* Platform Type Definitions. */
#include <Arduino.h>
#include <SPI.h>
//#include <ADS1115_WE.h>
#include <Wire.h>
#include <EEPROM.h>
#include <avr/eeprom.h>
#include <ModbusSerial.h> // //ModbusSerial by André Sarmento Barbos

/* Family Declaration(s). */
#include "synthe.h"

/* Global functions prototypes */
void SYNTHE_help(void);

void SYNTHE_init(boolean modbus_mode);
void SYNTHE_cold_start(void);
void SYNTHE_hot_start(void);
void SYNTHE_debug_trace(const char *message);
void SYNTHE_SEQUENCER(void);
void SYNTHE_status(void);
void SYNTHE_debug(U8 debug);
void SYNTHE_prog_reg(U8 reg_nr, U16 value);
U16 SYNTHE_read_reg(U8 reg_nr);
void SYNTHE_read_all_reg(void);
void SYNTHE_reset_default(void);
void SYNTHE_LMX25XX_init_prefered(void);
void SYNTHE_set_power(U8 power);
void SYNTHE_set_PLL(U8 parameter, U32 value);
void SYNTHE_modbus_init(void);
void SYNTHE_modbus_publish(void);
void SYNTHE_save_config(void);



/* Local functions prototypes */
//void SYNTHE_LMX2572_init(void);
U32 set_field(U32 reg, U32 field_value, U8 field_pos, U8 field_size);
void SYNTHE_read_eeprom(void);
void SYNTHE_sendto_LMX(U32 data);
U16 SYNTHE_config_crc(void);
boolean SYNTHE_config_valid(void);



/* Global variables */
U16 U16_restart =0;
U8 U8_Verbose = 0;
U8 U8_Debugg =0;
U32 U32_Time =0;
U32 pll_freq=0;
U32 pll_N=0;
U32 pll_num=0;
U32 pll_den=0;
U8 U8_modbus_slave_add = DEFAULT_MODBUS_ADD;
boolean B_modbus_mode = true;



/* create an instance of Modbus library */
ModbusSerial mb;



/*******************************************************************************/
/* 			        		PROGRAM CODE	     			    */
/*******************************************************************************/


/*--------------------------------------------------------------------------------
Description    : Init all parameters of axis 
Call           : void AXIS_init(void)  
Input(s)       :               
Output(s)      : 
Return         : 
--------------------------------------------------------------------------------*/
void SYNTHE_init(boolean modbus_mode)
{
      char string[8];
      U16 boot_counter=0;

      B_modbus_mode = modbus_mode;
      if (!B_modbus_mode) {
            Serial.println(F(VERSION));
            Serial.println(F("......................................"));
      }

      //Init SPI 
      pinMode(DATAOUT, OUTPUT);
      pinMode(SPICLOCK,OUTPUT);
      pinMode(SYNTHE_CS, OUTPUT);
      digitalWrite(SYNTHE_CS, HIGH);         // Active LOW

      
      SPI.begin();  // Begin SPI hardware
      SPI.setClockDivider(SPI_CLOCK_DIV64);  // Slow down SPI clock
      SPI.setBitOrder(MSBFIRST);    //MSB First
      SPI.setDataMode(SPI_MODE0);   //Mode 0

      //Default varaiable
      U8_Verbose = DEBUG_NONE;

      //Restore Preference (Cold or Hot start)
      EEPROM.get(BOOT_COUNT_ADD, boot_counter);
      if (!B_modbus_mode) {
            Serial.print(F("Boot Count: "));
            sprintf(string,"%u",boot_counter);
            Serial.println(string);
      }
      if (!SYNTHE_config_valid()) SYNTHE_cold_start();
      else SYNTHE_hot_start();

      //Init  Synthetizer
      SYNTHE_LMX25XX_init_prefered();

      // Init Modbus
      if (B_modbus_mode) SYNTHE_modbus_init();
}

/*--------------------------------------------------------------------------------
Description    : Sent 16bits word into specific LMX register trough SPI interface
Call           :  
Input(s)       :               
Output(s)      : 
Return         : 
--------------------------------------------------------------------------------*/
void SYNTHE_prog_reg(U8 reg_nr, U16 value)
{

      U32 dump1 =0;
	DOUBLEVAL reg;  
      char string[10];

      reg_nr &= 0x7f; //mask to keep 0...127add 
      reg.u32 = (U32)value & 0xffff; //clear MSB byte 3 & 4
      reg.u32 = set_field(reg.u32, (U32)reg_nr, 16, 7); //Add reg_nr in the MSB byte
      //reg.u32 |= 0x800000; // set bit 23 to 1 to Write
      digitalWrite(SYNTHE_CS, LOW); //select LMX
      SPI.transfer(reg.u8[2]); //MSB
      SPI.transfer(reg.u8[1]);  //MID
      SPI.transfer(reg.u8[0]);  //LSB
      digitalWrite(SYNTHE_CS, HIGH); //unselect LMX



     if(U8_Verbose == DEBUG_SPI_PROG || U8_Verbose == DEBUG_ALL)
     {
 
            Serial.print(F("LMX SPI Prog debug:"));

            Serial.print(F("reg_nr="));
            sprintf(string,"0x%02X",reg_nr);
            Serial.print(string);


            Serial.print(F(" M.u8[2]="));
            sprintf(string,"0x%02X",reg.u8[2]);
            Serial.print(string);
      
            Serial.print(F(" M.u8[1]="));
            sprintf(string,"0x%02X",reg.u8[1]);
            Serial.print(string);

            Serial.print(F(" M.u8[0]="));
            sprintf(string,"0x%02X",reg.u8[0]);
            Serial.println(string);

     }


}


/*--------------------------------------------------------------------------------
Description    : Read 16bits word into specific LMX register trough SPI interface
Call           :  
Input(s)       :               
Output(s)      : 
Return         : 
--------------------------------------------------------------------------------*/
U16 SYNTHE_read_reg(U8 reg_nr)
{

	DOUBLEVAL reg, dump1, rx_word;  
      char string[10];
   //   U16 rx_word;

      // Set Reg0 for Readback bit2=0 Reg0=0x2218
      reg_nr = 0;                                                                        
      reg.u32 =  pgm_read_dword_near(LMX25XX_Register_Prefered + reg_nr);               // Read default value of the Register
      reg.u32 = set_field(reg.u32, (U32)0, 2, 1);                                  //Set bit 2 of REG0 to 0: Readback enable
      SYNTHE_sendto_LMX(reg.u32);
/*
      reg.u32 = 0x2218;             //Set R0 default with ReadBack
      digitalWrite(SYNTHE_CS, LOW); //select LMX
      SPI.transfer(reg.u8[2]); //MSB
      SPI.transfer(reg.u8[1]);  //MID
      SPI.transfer(reg.u8[0]);  //LSB
      digitalWrite(SYNTHE_CS, HIGH); //unselect LMX
*/
      
     // Read Reg ADD(7bit) + 16bits Blank 
      reg_nr &= 0x7f; //mask to keep 0...127add 
      reg.u32 = 0;
      reg.u32 = set_field(reg.u32, (U32)reg_nr, 16, 7); //Set Address
      reg.u32 |= 0x800000;     //Force bit23 to 1 to READ
      //reg.u32= 0x800000;
      rx_word.u32 =0;

      digitalWrite(SYNTHE_CS, LOW); //select LMX
      rx_word.u8[2]= SPI.transfer(reg.u8[2]);
      rx_word.u8[1]= SPI.transfer(reg.u8[1]);
      rx_word.u8[0]= SPI.transfer(reg.u8[0]);
      digitalWrite(SYNTHE_CS, HIGH); //unselect LMX



      // Restore Lock detect (no readback) Reg0 bit2 to 1
      reg_nr = 0;                                                                        
      reg.u32 =  pgm_read_dword_near(LMX25XX_Register_Prefered + reg_nr);               // Read default value of the Register
      reg.u32 = set_field(reg.u32, (U32)1, 2, 1);                                  //Set bit 2 of REG0 to 1: Readback enable
      SYNTHE_sendto_LMX(reg.u32);
  
      /* reg.u32 = 0x221c;             //Set R0 default with no ReadBack
      digitalWrite(SYNTHE_CS, LOW); //select LMX
      SPI.transfer(reg.u8[2]); //MSB
      SPI.transfer(reg.u8[1]);  //MID
      SPI.transfer(reg.u8[0]);  //LSB
      digitalWrite(SYNTHE_CS, HIGH); //unselect LMX
*/

     if(U8_Verbose == DEBUG_SPI_PROG || U8_Verbose == DEBUG_ALL)
     {
 
            Serial.print(F("LMX SPI Read debug:"));

            Serial.print(F("reg_nr="));
            sprintf(string,"0x%02X",reg_nr);
            Serial.print(string);


            Serial.print(F(" RX_word="));
            sprintf(string,"0x%04X",rx_word.u16[0]);
            Serial.print(string);
      
            Serial.print(F(" TXquery[2-0]="));

            sprintf(string," 0x%04X",reg.u16[1]);
            Serial.print(string);

            sprintf(string," 0x%04X",reg.u16[0]);
            Serial.println(string);

     }
     return rx_word.u16[0];

}


/*--------------------------------------------------------------------------------
Description    : Read all register of LMX
Call           :  
Input(s)       :               
Output(s)      : 
Return         : 
--------------------------------------------------------------------------------*/
void SYNTHE_read_all_reg(void)
{

	DOUBLEVAL reg, default_reg, rx_word;  
      char string[10];
      U8 reg_nr, linecr;
   //   U16 rx_word;

         // Set Reg0 for Readback bit2=0 Reg0=0x2218
      reg_nr = 0;                                                                        
      reg.u32 =  pgm_read_dword_near(LMX25XX_Register_Prefered + reg_nr);               // Read default value of the Register
      reg.u32 = set_field(reg.u32, (U32)0, 2, 1);                                  //Set bit 2 of REG0 to 0: Readback enable
      SYNTHE_sendto_LMX(reg.u32);   
   
   
   // Set Reg0 for Readback bit2=0 Reg0=0x2218
      /*reg.u32 = 0x2218;             //Set R0 default with ReadBack
      digitalWrite(SYNTHE_CS, LOW); //select LMX
      SPI.transfer(reg.u8[2]); //MSB
      SPI.transfer(reg.u8[1]);  //MID
      SPI.transfer(reg.u8[0]);  //LSB
      digitalWrite(SYNTHE_CS, HIGH); //unselect LMX
      */
      Serial.println(F("LMX SPI Register contents"));
      Serial.println(F("**************************"));
      
      linecr=0;

     // Read all register table
      for(reg_nr=0; reg_nr <=125;reg_nr++){  
            reg.u32 = 0;
            reg.u32 = set_field(reg.u32, (U32)reg_nr, 16, 7); //Set Address
            reg.u32 |= 0x800000;     //Force bit23 to 1 to READ
            rx_word.u32 =0;

            digitalWrite(SYNTHE_CS, LOW); //select LMX
            rx_word.u8[2]= SPI.transfer(reg.u8[2]);
            rx_word.u8[1]= SPI.transfer(reg.u8[1]);
            rx_word.u8[0]= SPI.transfer(reg.u8[0]);
            digitalWrite(SYNTHE_CS, HIGH); //unselect LMX

            
            ///*// With reg_nr count And display the startup Default value. Format:reg_nr=01 reg_word=0x0f0f (0x0f0f)
            Serial.print(F("reg_nr="));
            sprintf(string,"%u",reg_nr);
            Serial.print(string);
            sprintf(string," (0x%2X)",reg_nr);
            Serial.print(string);
            Serial.print(F(" reg_word="));
            sprintf(string,"0x%04X ",rx_word.u16[0]);
            Serial.print(string);
            for(U8 index=0; index<=LMX_REGISTER_TABLE_SIZE-1;index++){        //Scan LMX25XX_Register_Prefered to display the default value
                  reg.u32 =  pgm_read_dword_near(LMX25XX_Register_Prefered + index);   
                  if( (reg.u32 & 0x00ff0000)>>16 == reg_nr){        //Check if register exist in default startup value     
                        reg.u32 &=0xffff;                           //Conserve 16bits LSB 
                        sprintf(string,"[%04X]",reg.u16[0]);
                        Serial.print(string); 
                        if(reg.u16[0] !=rx_word.u16[0]) Serial.print("***"); 
                  } 
            }
            Serial.println(" "); 
            //*/

           //Hexa format for compileur
           /*
            sprintf(string,"0x%02X",reg_nr);
            Serial.print(string);
            sprintf(string,"%04X, ",rx_word.u16[0]);
            Serial.print(string);

            if(linecr==15){                     //tabulation 16 value per row
                  linecr=0;
                  Serial.println(F(""));
            } else linecr++;
            */

      }

      Serial.println(F(""));
      // Restore Lock detect (no readback) Reg0 bit2 to 1
      reg_nr = 0;                                                                        
      reg.u32 =  pgm_read_dword_near(LMX25XX_Register_Prefered + reg_nr);               // Read default value of the Register
      reg.u32 = set_field(reg.u32, (U32)1, 2, 1);                                  //Set bit 2 of REG0 to 1: Readback enable
      SYNTHE_sendto_LMX(reg.u32);    
      /*
      // Restore Lock detect (no readback) Reg0=0x221c for Readback
      reg.u32 = 0x221c;             //Set R0 default with no ReadBack
      digitalWrite(SYNTHE_CS, LOW); //select LMX
      SPI.transfer(reg.u8[2]); //MSB
      SPI.transfer(reg.u8[1]);  //MID
      SPI.transfer(reg.u8[0]);  //LSB
      digitalWrite(SYNTHE_CS, HIGH); //unselect LMX
*/
}



/*--------------------------------------------------------------------------------
Description    : Set all registers of LMX with prefered value as defined in LMX2572_Register_Default[]
Call           :  
Input(s)       :               
Output(s)      : 
Return         : 
--------------------------------------------------------------------------------*/
void SYNTHE_LMX25XX_init_prefered(void)
{
      U32 dump1 =0;
	DOUBLEVAL reg;  
      char string[10];
      S8 index;
      //Reset Synth to default
      SYNTHE_reset_default();

      // Programm Prefered values
      for(index=LMX_REGISTER_TABLE_SIZE-1; index >= 0; index--){  
          
            reg.u32 =  pgm_read_dword_near(LMX25XX_Register_Prefered + index);

            digitalWrite(SYNTHE_CS, LOW); //select LMX
            SPI.transfer(reg.u8[2]); //MSB
            SPI.transfer(reg.u8[1]);  //MID
            SPI.transfer(reg.u8[0]);  //LSB
            digitalWrite(SYNTHE_CS, HIGH); //unselect LMX

            if(U8_Verbose == DEBUG_INIT_PREFERED || U8_Verbose == DEBUG_ALL)
            {
                  sprintf(string,"index=%u ",index);
                  Serial.print(string);
                  sprintf(string,"0x%04X",reg.u16[1]);
                  Serial.print(string);
                  sprintf(string," 0x%04X",reg.u16[0]);
                  Serial.println(string);
            }      


      }

      
      //Restore specific LMX registers related to FREQ
      SYNTHE_set_PLL(SET_PLL_N, pll_N);
      SYNTHE_set_PLL(SET_PLL_NUM, pll_num);
      SYNTHE_set_PLL(SET_PLL_DEN, pll_den);

      if (!B_modbus_mode) Serial.println(F("LMX Register restored to prefered"));

}


/*--------------------------------------------------------------------------------
Description    : Set all registers of LMX to default as defined in datasheet
Call           :  
Input(s)       :               
Output(s)      : 
Return         : 
--------------------------------------------------------------------------------*/
void SYNTHE_reset_default(void)
{
      DOUBLEVAL reg, dump1, rx_word;  
      char string[10];
      U8 reg_nr;

      if (!B_modbus_mode) Serial.println(F("LMX SPI Register Reset to default"));
      // Set Reg0 for RESET bit1=1 Reg0=0x221E
      reg.u32 = 0x221E;             //Set R0 default with ReadBack
      digitalWrite(SYNTHE_CS, LOW); //select LMX
      SPI.transfer(reg.u8[2]); //MSB
      SPI.transfer(reg.u8[1]);  //MID
      SPI.transfer(reg.u8[0]);  //LSB
      digitalWrite(SYNTHE_CS, HIGH); //unselect LMX
      

}





/*--------------------------------------------------------------------------------
Description    : 
Call           :  
Input(s)       :               
Output(s)      : 
Return         : 
--------------------------------------------------------------------------------*/
void SYNTHE_SEQUENCER(void)
{

      //if(U32_Time % 10 == 0)  SYNTHE_modbus_publish();        //Around every 0.65s make a key scan.
      //if(U32_Time %10  == 0) NMON_VIDEO_FILTER_SELECT(); // Scan Video Filter Select
      //if(U32_Time % 1000 == 0) NMON_read_temp();      //Around every 65s make a temperature meas.

}


/*--------------------------------------------------------------------------------
Description    : Program_Synthe
Call           :  
Input(s)       :               
Output(s)      : 
Return         : 
--------------------------------------------------------------------------------*/
void  SYNTHE_set_PLL(U8 parameter, U32 value)
{

      DOUBLEVAL reg;
      LONGVAL num;
      char string[15];
      U8 reg_nr =0;    

      switch(parameter)
      {
            case SET_FREQ:     
                  if (value < MIN_OL_FREQ_KHZ || value > MAX_OL_FREQ_KHZ || pll_den == 0) return;
                  pll_freq = value;
                 #ifdef VERSION_LMX2572 //Set frequency fvco = fdp * (PLL_N + PLL_NUM/PLL_DEN)
                  pll_N = value / DEFAULT_PLL_PD;   //Integer value
                  num.ull = ((U64)value * (U64)pll_den)/DEFAULT_PLL_PD;
                  num.ull -= ( (U64)pll_N * (U64)pll_den) ;                  
                  pll_num = num.ul[0];
                  #endif
                 #ifdef VERSION_LMX2592 //Set frequency fvco = 4 * fdp * (PLL_N + PLL_NUM/PLL_DEN)
                  pll_N = value / (4 * DEFAULT_PLL_PD) ;   //Integer value
                  num.ull = ((U64)value * (U64)pll_den)/(4 * DEFAULT_PLL_PD);
                  num.ull -= ( (U64)pll_N * (U64)pll_den) ;                  
                  pll_num = num.ul[0];
                  #endif

                  
                  if (!B_modbus_mode) {
                        Serial.print(F("PLL_N="));
                        sprintf(string,"%lu",pll_N);
                        Serial.print(string);
                        reg.u32 = pll_num;
                        Serial.print(F(", PLL_num="));
                        sprintf(string,"0x%04X ",reg.u16[1]);
                        Serial.print(string);
                        sprintf(string," 0x%04X",reg.u16[0]);
                        Serial.print(string);
                        reg.u32 = pll_den;
                        Serial.print(F(", PLL_den="));
                        sprintf(string,"0x%04X ",reg.u16[1]);
                        Serial.print(string);
                        sprintf(string," 0x%04X",reg.u16[0]);
                        Serial.println(string);
                  }

                  //Set Parameter to LMX
                  SYNTHE_set_PLL(SET_PLL_N, pll_N);
                  SYNTHE_set_PLL(SET_PLL_NUM, pll_num);                  
                  // Save to EEPROM
                  EEPROM.put(DEFAULT_PLL_N_ADD, pll_N);                            
                  EEPROM.put(DEFAULT_PLL_NUM_ADD, pll_num);                      
                  EEPROM.put(DEFAULT_FREQ_ADD, pll_freq);
                  SYNTHE_save_config();
                  break;


            case SET_PLL_N:        //Set N Register 
                  if (value == 0 || value > 0x0FFF) return;
                  pll_N = value;
                  #ifdef VERSION_LMX2572
                  reg_nr = 36;                                                                        //First 16bits
                  reg.u32 = value & 0xffff;                                                           //limit to 16bits LSB 
                  //reg.u32 =  pgm_read_dword_near(LMX25XX_Register_Prefered + reg_nr);               // Read default value of the Register
                  reg.u32 = set_field(reg.u32, (U32)reg_nr, 16, 7);                                   //SET ADD REG nr: 7 bits wide start at bit16
                  SYNTHE_sendto_LMX(reg.u32);
                  reg_nr = 34;                                                                        //Set MSB part of N reg
                  reg.u32 = (value & 0x00070000) >> 16;                                                //Limit to bits 18-16 and move to b2b1b0
                  reg.u32 = set_field(reg.u32, (U32)reg_nr, 16, 7);                                     //ADD REG nr: 7 bits wide start at bit16
                  SYNTHE_sendto_LMX(reg.u32);
                  #endif
                  #ifdef VERSION_LMX2592
                  reg_nr = 38;                                                                        //First 12bits
                  reg.u32 = (value & 0x0fff)<<1;                                                      //limit to 12bits b12...b1  
                  //reg.u32 =  pgm_read_dword_near(LMX25XX_Register_Prefered + reg_nr);               // Read default value of the Register
                  reg.u32 = set_field(reg.u32, (U32)reg_nr, 16, 7);                                   //ADD REG nr: 7 bits wide start at bit16
                  SYNTHE_sendto_LMX(reg.u32);
                  #endif
                  
                  
                  
                  break;


            case SET_PLL_NUM:     //Set Numerator
                  if (pll_den == 0 || value >= pll_den) return;
                  pll_num = value;
                  #ifdef VERSION_LMX2572
                  reg_nr = 43;                                                                        //First 16bits
                  reg.u32 = value & 0xffff;                                                           //limit to 16bits LSB 
                  //reg.u32 =  pgm_read_dword_near(LMX25XX_Register_Prefered + reg_nr);               // Read default value of the Register
                  reg.u32 = set_field(reg.u32, (U32)reg_nr, 16, 7);                                   //ADD REG nr: 7 bits wide start at bit16
                  SYNTHE_sendto_LMX(reg.u32);
                  reg_nr = 42;                                                                        //Set MSB part of N reg
                  reg.u32 = (value & 0xffff0000) >> 16;                                               //Limit to MSB bits 31-16
                  reg.u32 = set_field(reg.u32, (U32)reg_nr, 16, 7);                                   //ADD REG nr: 7 bits wide start at bit16
                  SYNTHE_sendto_LMX(reg.u32);  
                  #endif
                  #ifdef VERSION_LMX2592
                  reg_nr = 45;                                                                        //First 16bits
                  reg.u32 = value & 0xffff;                                                           //limit to 16bits LSB 
                  //reg.u32 =  pgm_read_dword_near(LMX25XX_Register_Prefered + reg_nr);               // Read default value of the Register
                  reg.u32 = set_field(reg.u32, (U32)reg_nr, 16, 7);                                   //ADD REG nr: 7 bits wide start at bit16
                  SYNTHE_sendto_LMX(reg.u32);
                  reg_nr = 44;                                                                        //Set MSB part of N reg
                  reg.u32 = (value & 0xffff0000) >> 16;                                               //Limit to MSB bits 31-16 shifted b15...b0
                  reg.u32 = set_field(reg.u32, (U32)reg_nr, 16, 7);                                   //ADD REG nr: 7 bits wide start at bit16
                  SYNTHE_sendto_LMX(reg.u32);  
                  
                  #endif
                  break;


            case SET_PLL_DEN:     //Set Denominator
                  if (value == 0 || pll_num >= value) return;
                  pll_den = value;
                  #ifdef VERSION_LMX2572                 
                  reg_nr = 39;                                                                        //First 16bits
                  reg.u32 = value & 0xffff;                                                           //limit to 16bits LSB 
                  //reg.u32 =  pgm_read_dword_near(LMX25XX_Register_Prefered + reg_nr);               // Read default value of the Register
                  reg.u32 = set_field(reg.u32, (U32)reg_nr, 16, 7);                                   //ADD REG nr: 7 bits wide start at bit16
                  SYNTHE_sendto_LMX(reg.u32);
                  reg_nr = 38;                                                                        //Set MSB part of N reg
                  reg.u32 = (value & 0xffff0000) >> 16;                                               //Limit to MSB bits 31-16
                  reg.u32 = set_field(reg.u32, (U32)reg_nr, 16, 7);                                   //ADD REG nr: 7 bits wide start at bit16
                  SYNTHE_sendto_LMX(reg.u32);    
                  #endif
                  #ifdef VERSION_LMX2592
                  reg_nr = 41;                                                                        //First 16bits
                  reg.u32 = value & 0xffff;                                                           //limit to 16bits LSB 
                  //reg.u32 =  pgm_read_dword_near(LMX25XX_Register_Prefered + reg_nr);               // Read default value of the Register
                  reg.u32 = set_field(reg.u32, (U32)reg_nr, 16, 7);                                   //ADD REG nr: 7 bits wide start at bit16
                  SYNTHE_sendto_LMX(reg.u32);
                  reg_nr = 40;                                                                        //Set MSB part of N reg
                  reg.u32 = (value & 0xffff0000) >> 16;                                               //Limit to MSB bits 31-16 shifted b15...b0
                  reg.u32 = set_field(reg.u32, (U32)reg_nr, 16, 7);                                   //ADD REG nr: 7 bits wide start at bit16
                  SYNTHE_sendto_LMX(reg.u32);                      
                  #endif
                  
                  break;

                  default:
                  break;
      }

      // Set Reg0 FCAL=1
      reg_nr = 0;                                                                        
      reg.u32 =  pgm_read_dword_near(LMX25XX_Register_Prefered + reg_nr);               // Read default value of the Register
      reg.u32 = set_field(reg.u32, (U32)1, 3, 1);                                  //Set bit 3 of REG0 to 1: FCAL
      SYNTHE_sendto_LMX(reg.u32);



}



/*--------------------------------------------------------------------------------
Description    : Program_Synthe
Call           :  
Input(s)       :               
Output(s)      : 
Return         : 
--------------------------------------------------------------------------------*/
void SYNTHE_set_power(U8 power)
{
     DOUBLEVAL reg, rx_word;  
      char string[10];
      U8 reg_nr =0;
      #ifdef VERSION_LMX2572
      reg_nr = 44; //Reg 44
      // Read default value of the Register
      reg.u32 =  pgm_read_dword_near(LMX25XX_Register_Prefered + reg_nr);
     // Set Reg 44 to adjust Power PLL A 
      reg_nr = 44; //Reg 44
      reg.u32 = set_field(reg.u32, (U32)power, 8, 6); //6 bits wide start at bit8
      //reg.u32 = set_field(reg.u32, (U32)reg_nr, 16, 7); //7 bits wide start at bit 16
      #endif

      #ifdef VERSION_LMX2592
      reg_nr = 46; //Set Reg 46 to adjust Power PLL A 
      // Read default value of the Register
      reg.u32 =  pgm_read_dword_near(LMX25XX_Register_Prefered + 36);     //Preserved other bit of the reg ???be careful with index 36 if shifted !!!Make a function to retrieve reg value with a loop
      reg.u32 = set_field(reg.u32, (U32)power, 8, 6); //6 bits wide start at bit8
      #endif



      digitalWrite(SYNTHE_CS, LOW); //select LMX
      rx_word.u8[2]= SPI.transfer(reg.u8[2]);
      rx_word.u8[1]= SPI.transfer(reg.u8[1]);
      rx_word.u8[0]= SPI.transfer(reg.u8[0]);
      digitalWrite(SYNTHE_CS, HIGH); //unselect LMX


     if(U8_Verbose == DEBUG_SET_VALUE || U8_Verbose == DEBUG_ALL)
     {
 
            Serial.print(F("LMX SPI SET POWER:"));

            Serial.print(F("reg_nr="));
            sprintf(string,"0x%02X",reg_nr);
            Serial.print(string);
      
            Serial.print(F(" TXword[2-0]="));

            sprintf(string," 0x%04X",reg.u16[1]);
            Serial.print(string);

            sprintf(string," 0x%04X",reg.u16[0]);
            Serial.println(string);

     }

}


//*************************/
/*	LOCAL FUNCTIONS	*/
/*************************/

void SYNTHE_DEBUG(char *messg)
     
{
      if(U8_Verbose !=0)
      {
            switch(U8_Verbose == 2 || U8_Verbose ==DEBUG_ALL)
            {
                  case 1:     // Debbug1
                        Serial.print(messg);
                        break;


                  case 2:     // Debbug2
                        break;


                  case 3:     // Debbug3
                        break;


                  case 10:     // Debbug10

                        break;

                  default:
                  break;
            }
      }

}

/*--------------------------------------------------------------------------------
Description    : Display Status of the Synthe, print freq and the 5 most important registers
Call           :
Input(s)       :               
Output(s)      : 
Return         : 
--------------------------------------------------------------------------------*/

void SYNTHE_status(void)
{

            char string[8];
            //LONGVAL data_l;
/*
            Serial.print(F("Temp:"));
            dtostrf(F32_temp, 4, 2 , string);
            Serial.print(string);
            Serial.print(F("[°C]"));

            //data_l.ull = U32_Freq;
            Serial.print(F(", Frx="));  
            sprintf(string, "%lu", U32_Freq);
            Serial.print(string); 
            Serial.print(F("[kHz]"));

            Serial.print(F(", Offset:"));
            dtostrf(F32_Pcal_offset, 4, 2 , string);        
            Serial.print(string);
            Serial.print(F("[dBm]"));

            Serial.print(F(", Ref:"));
            dtostrf(F32_Pcal_Ref, 4, 2 , string);        
            Serial.print(string);
            Serial.print(F("[dBm]"));

            Serial.print(F(", Video Filter:"));  
            sprintf(string, "%u", U8_video_filter_nr);
            Serial.println(string); 
            */

}    

/*--------------------------------------------------------------------------------
Description    : Display Help 

Call           : void NMON_init(void)  
Input(s)       :               
Output(s)      : 
Return         : 
--------------------------------------------------------------------------------*/

void SYNTHE_help(void)
{

      Serial.println(F("Synthe  Command Help"));
      Serial.println(F("*************************"));
      //Serial.println(F("l                 Toggle on/off Power Loop Reading"));
      //Serial.println(F("power?            Single power query"));  
      //Serial.println(F("cal_power?        Single power query for AD608 calibration"));      
      Serial.println(F("freq xxx          Set OL freq in [kHz] (6000000..9800000)"));
      Serial.println(F("pll_n xxx         Set N counter (19bits)")); // 
      Serial.println(F("pll_num xxx       Set Numerator counter (32bits)")); //             
      Serial.println(F("pll_den xxx       Set Denominator (32bits)")); //     
      Serial.println(F("power xxx         Set Power PLL A (0...63)")); //            
      Serial.println(F("set_reg xxx       Set 24bits LMX reg. (add+value) ex 0x00201C for reg#0)"));    
      Serial.println(F("read_reg xxx      Read LMX reg. (add) ex. read_reg 0x01 for reg#1)"));   
      Serial.println(F("9                 Read all register of the LMX")); 
      Serial.println(F("debug 1           Set Bootcount to 0"));           
      Serial.println(F("debug 2           Read EEPROM Page 0"));      
      Serial.println(F("reset_default     reset all register to default"));    


}

/*--------------------------------------------------------------------------------
Description    : Restaure values in Flash 

Call           : void AXIS_init(void)  
Input(s)       :               
Output(s)      : 
Return         : 

#define DEFAULT_FREQ            3590000 //3590MHz
#define DEFAULT_PLL_REF         10000   //10MHz
#define DEFAULT_PLL_N           179     // f= (179 +50/100)*10'000
#define DEFAULT_PLL_NUM         100     //U32 4 bytes
#define DEFAULT_PLL_DEN         50      //U32 4 bytes
--------------------------------------------------------------------------------*/
void SYNTHE_hot_start(void)
{
     LONGVAL data_l;
     char string[10];
    // U16 data=0;
     
      //Boot Count
      if (!B_modbus_mode) Serial.println(F("Prefered Data Restored from Flash"));
      EEPROM.get(BOOT_COUNT_ADD, data_l.ui[0]);
      EEPROM.put(BOOT_COUNT_ADD, data_l.ui[0] + 1);
      //Freq 
      EEPROM.get(DEFAULT_FREQ_ADD, data_l.ul[0]);
      pll_freq = data_l.ul[0];
      if (!B_modbus_mode) {
            Serial.print(F("freq="));
            sprintf(string, "%lu", data_l.ul[0]);
            Serial.print(string);
      }
      //PLL_N
      EEPROM.get(DEFAULT_PLL_N_ADD, data_l.ul[0]);
      pll_N = data_l.ul[0];
      if (!B_modbus_mode) {
            Serial.print(F(", PLL_N="));
            sprintf(string, "%lu", data_l.ul[0]);
            Serial.print(string);
      }
      //PLL_N
      EEPROM.get(DEFAULT_PLL_NUM_ADD, data_l.ul[0]);
      pll_num = data_l.ul[0];
      if (!B_modbus_mode) {
            Serial.print(F(", PLL_num="));
            sprintf(string, "%lu", data_l.ul[0]);
            Serial.print(string);
      }
      //PLL_N
      EEPROM.get(DEFAULT_PLL_DEN_ADD, data_l.ul[0]);
      pll_den = data_l.ul[0];
      if (!B_modbus_mode) {
            Serial.print(F(", PLL_den="));
            sprintf(string, "%lu", data_l.ul[0]);
            Serial.println(string);
      }
      EEPROM.get(FLASH_MODBUS_ADD, U8_modbus_slave_add);


}

/*--------------------------------------------------------------------------------
Description    : Cold Start 

Call           : 
Input(s)       :               
Output(s)      : 
Return         : 
#define BOOT_COUNT_ADD              0   //u16, 2byte
#define DEFAULT_FREQ_ADD            2   //u32 4 bytes
#define DEFAULT_PLL_N_ADD          6   //U32 4 bytes
#define DEFAULT_PLL_NUM_ADD         10  //U32 4 bytes
#define DEFAULT_PLL_DEN_ADD         14  //U32 4 bytes
--------------------------------------------------------------------------------*/
void SYNTHE_cold_start(void)
{     
    //  LONGVAL data_l;
      //char string[6];
      //U16 data=0;

      if (!B_modbus_mode) Serial.println(F("Cold Start: Force Default value"));
      EEPROM.put(BOOT_COUNT_ADD, 0); 
      EEPROM.put(DEFAULT_FREQ_ADD, DEFAULT_FREQ);
      EEPROM.put(DEFAULT_PLL_N_ADD, DEFAULT_PLL_N);
      EEPROM.put(DEFAULT_PLL_NUM_ADD, DEFAULT_PLL_NUM);
      EEPROM.put(DEFAULT_PLL_DEN_ADD, DEFAULT_PLL_DEN);
      EEPROM.put(DEFAULT_POWER_ADD, (U8)0);
      EEPROM.put(FLASH_MODBUS_ADD, (U8)DEFAULT_MODBUS_ADD);
      SYNTHE_save_config();

      SYNTHE_hot_start();
}

/* CRC-16/Modbus over the persistent payload at EEPROM addresses 2..19. */
U16 SYNTHE_config_crc(void)
{
      U16 crc = 0xFFFF;
      for (U8 address = DEFAULT_FREQ_ADD; address <= FLASH_MODBUS_ADD; ++address) {
            crc ^= EEPROM.read(address);
            for (U8 bit = 0; bit < 8; ++bit)
                  crc = (crc & 1) ? (crc >> 1) ^ 0xA001 : crc >> 1;
      }
      return crc;
}

void SYNTHE_save_config(void)
{
      const U16 magic = EEPROM_CONFIG_MAGIC;
      EEPROM.put(EEPROM_MAGIC_ADD, magic);
      const U16 crc = SYNTHE_config_crc();
      EEPROM.put(EEPROM_CRC_ADD, crc);
}

boolean SYNTHE_config_valid(void)
{
      U16 magic = 0;
      U16 stored_crc = 0;
      U32 frequency = 0;
      U32 n = 0;
      U32 denominator = 0;
      U8 slave = 0;
      EEPROM.get(EEPROM_MAGIC_ADD, magic);
      EEPROM.get(EEPROM_CRC_ADD, stored_crc);
      EEPROM.get(DEFAULT_FREQ_ADD, frequency);
      EEPROM.get(DEFAULT_PLL_N_ADD, n);
      EEPROM.get(DEFAULT_PLL_DEN_ADD, denominator);
      EEPROM.get(FLASH_MODBUS_ADD, slave);
      return magic == EEPROM_CONFIG_MAGIC &&
             stored_crc == SYNTHE_config_crc() &&
             frequency >= MIN_OL_FREQ_KHZ && frequency <= MAX_OL_FREQ_KHZ &&
             n > 0 && denominator > 0 &&
             slave >= 1 && slave <= 247;
}

/*--------------------------------------------------------------------------------
Description    : Debug fonction 

Call           : 
Input(s)       :               
Output(s)      : 
Return         : 
--------------------------------------------------------------------------------*/
void SYNTHE_debug(U8 debug)
{     
 switch(debug)
      {
            case 0:     //do nothing

            break;

            case 1:     //reset to default bootcount
                  EEPROM.put(BOOT_COUNT_ADD, 0);
                  Serial.println(F("Set bootcount to 0"));
            break;

            case 2:     //read eeprom page
                  SYNTHE_read_eeprom();
            break;

            default:
            break;

      }


}


/*--------------------------------------------------------------------------------
Description    : Debug fonction 

Call           : 
Input(s)       :               
Output(s)      : 
Return         : 
--------------------------------------------------------------------------------*/
void SYNTHE_read_eeprom(void)
{     
      U16 i=0;
      U8 data, linecr;
      char string[15];
      linecr =0;
 
              //    EEPROM.get(BOOT_COUNT_ADD, data_l.ui[0]);
      Serial.println(F("Page 0 EEPROM dump")); 
      EEPROM.get(i, data);
      //sprintf(string,"i=%u 0x%X ",i, data);
      //Serial.print(string);            
      for(i=0; i < 32; i++){  

            EEPROM.get(i, data);
            sprintf(string,"i=%u 0x%X ",i, data);
            Serial.print(string); 

            if(linecr==15){                     //tabulation 16 value per row
                  linecr=0;
                  Serial.println(F(""));
            } else linecr++;

      }
      Serial.println(F(""));


}
/*--------------------------------------------------------------------------------
Description    : Into a 32bits wise word "reg", set bit with "field value" at "field_pos" with "field_size" wide
Call           : U32 set_field(U32 reg, U32 field_value, U8 field_pos, U8 field_size)
Input(s)       :               
Output(s)      : 
Return         : the 32bits wise modified
example:
      uint16_t register_value = 0;  // registre initialisé à 0
      // Modifier le champ A (bits 0-3) avec la valeur 0b1010 (10 en décimal)
      register_value = set_field(register_value, 0b1010, 0, 4);

      test value 0x100080
      0x700000, 0x710000, 0x727802, 0x730000, 0x740000, 0x750000, 0x760000, 0x770000, 0x780000, 0x790000, 0x7A0000, 0x7B0000, 0x7C0000, 0x7D2288
--------------------------------------------------------------------------------*/

U32 set_field(U32 reg, U32 field_value, U8 field_pos, U8 field_size) {
/*
      union DOUBLEVAL{
	      boolean b[16];
            U8 u8[4];
	      U16 u16[2];
            U32 u32;
	    };
*/
	DOUBLEVAL  dump1, dump2, dump3, dump4, mask;  
      char string[10];

      // Créer un masque pour effacer le champ
       //mask.u32 = ((1 << field_size) - 1) << field_pos; // commande splittee pour que ca marche sous Arduino
      mask.u32 = 1 << field_size; //ex. Mask 0000 1000
      mask.u32 = mask.u32 -1; //ex. Mask 0000 0111  
      mask.u32 = mask.u32 << field_pos; //ex. Mask 0111 0000
      
      // Effacer le champ existant
      // reg &= ~mask.u32; // commande splittee pour que ca marche sous Arduino
      dump1.u32 = reg;
      dump1.u32 &= ~mask.u32; // Met à 0 le champ que l'on veut modifier

      // Set la nouvelle valeur
      // reg |= (field_value & ((1 << field_size) - 1)) << field_pos; // commande splittee pour que ca marche sous Arduino
      dump2.u32 =0;
      dump2.u32 |= (field_value & ((1 << field_size) - 1)); // Met à 1 le nouveau champ
      dump3.u32 = dump2.u32 << field_pos;
      
      reg = dump1.u32 | dump3.u32; //Set la valeur dans le bon champs   
      dump4.u32 = reg;
    
     if(U8_Verbose == DEBUG_SET_FIELD || U8_Verbose == DEBUG_ALL)
     {
            Serial.print(F("set_field debug:"));

            Serial.print(F("field_value="));
            sprintf(string,"0x%04X",field_value);
            Serial.print(string);

            Serial.print(F(" field_size="));
            sprintf(string,"0x%02X",field_size);
            Serial.print(string);

            Serial.print(F(" field_pos="));
            sprintf(string,"0x%02X",field_pos);
            Serial.print(string);

            Serial.print(F(" Mask[1-0]="));
            sprintf(string,"0x%04X",mask.u16[1]);
            Serial.print(string); 
            
            Serial.print(F(" "));
            sprintf(string,"0x%04X",mask.u16[0]);
            Serial.println(string);

            Serial.print(F(" dump1.u16[1-0]="));
            sprintf(string,"0x%04X",dump1.u16[1]);
            Serial.print(string);

            Serial.print(F(" "));
            sprintf(string,"0x%04X",dump1.u16[0]);
            Serial.print(string);

            Serial.print(F(" dump2.u16[1-0]="));
            sprintf(string,"0x%04X",dump2.u16[1]);
            Serial.print(string);

            Serial.print(F(" "));
            sprintf(string,"0x%04X",dump2.u16[0]);
            Serial.print(string);

            Serial.print(F(" dump3.u16[1-0]="));
            sprintf(string,"0x%04X",dump3.u16[1]);
            Serial.print(string);

            Serial.print(F(" "));
            sprintf(string,"0x%04X",dump3.u16[0]);
            Serial.println(string);

            Serial.print(F(" dump4.u16[1-0]="));
            sprintf(string,"0x%04X",dump4.u16[1]);
            Serial.print(string);

            Serial.print(F(" "));
            sprintf(string,"0x%04X",dump4.u16[0]);
            Serial.println(string);

     } 

    return reg;
}

/*--------------------------------------------------------------------------------
Description    : Send 24 bits word to LMX via SPI
Call           :  
Input(s)       :               
Output(s)      : 
Return         : 
--------------------------------------------------------------------------------*/
void SYNTHE_sendto_LMX(U32 data)
{
	DOUBLEVAL reg;  

      reg.u32 = data;
      digitalWrite(SYNTHE_CS, LOW); //select LMX
      SPI.transfer(reg.u8[2]);
      SPI.transfer(reg.u8[1]);
      SPI.transfer(reg.u8[0]);
      digitalWrite(SYNTHE_CS, HIGH); //unselect LMX

}

/*--------------------------------------------------------------------------------
Description    : MODBUS INIT
Call           :  
Input(s)       :               
Output(s)      : 
Return         : 
---------------------------------------------------------------------------------- */

void SYNTHE_modbus_init(void)
{
 // Configure TX/RX switch pin
      pinMode(MODBUS_TX_ENABLE, OUTPUT);
      digitalWrite(MODBUS_TX_ENABLE, LOW);

      mb.config(&Serial, 38400, SERIAL_8N1, MODBUS_TX_ENABLE);
      // Set the Slave ID (1-247)
      mb.setSlaveId(U8_modbus_slave_add);
      // OL frequency in kHz, high word first, matching the TRVT Python HMI.
      mb.addHreg(MODBUS_HREG_OL_FREQ_H, (U16)(pll_freq >> 16));
      mb.addHreg(MODBUS_HREG_OL_FREQ_L, (U16)(pll_freq & 0xFFFF));
      mb.addIreg(MODBUS_IREG_OL_FREQ_H, (U16)(pll_freq >> 16));
      mb.addIreg(MODBUS_IREG_OL_FREQ_L, (U16)(pll_freq & 0xFFFF));
      mb.addIreg(MODBUS_IREG_FW_VERSION, RELEASE_LEVEL);
      mb.addIreg(MODBUS_IREG_LOCKED, digitalRead(LOCK_DETECT) == HIGH);
      mb.addIsts(MODBUS_ISTS_LOCKED, digitalRead(LOCK_DETECT) == HIGH);


}

/*--------------------------------------------------------------------------------
Description    : MODBUS INIT
Call           :  
Input(s)       :               
Output(s)      : 
Return         : 
---------------------------------------------------------------------------------- */

void SYNTHE_modbus_publish(void)
{
      mb.task();

      const U32 requested_frequency =
            ((U32)mb.Hreg(MODBUS_HREG_OL_FREQ_H) << 16) |
            (U32)mb.Hreg(MODBUS_HREG_OL_FREQ_L);

      if (requested_frequency != pll_freq) {
            if (requested_frequency >= MIN_OL_FREQ_KHZ && requested_frequency <= MAX_OL_FREQ_KHZ)
                  SYNTHE_set_PLL(SET_FREQ, requested_frequency);
            // Always reflect the accepted value. Invalid commands are rejected atomically.
            mb.Hreg(MODBUS_HREG_OL_FREQ_H, (U16)(pll_freq >> 16));
            mb.Hreg(MODBUS_HREG_OL_FREQ_L, (U16)(pll_freq & 0xFFFF));
      }

      mb.Ireg(MODBUS_IREG_OL_FREQ_H, (U16)(pll_freq >> 16));
      mb.Ireg(MODBUS_IREG_OL_FREQ_L, (U16)(pll_freq & 0xFFFF));
      mb.Ireg(MODBUS_IREG_FW_VERSION, RELEASE_LEVEL);
      mb.Ireg(MODBUS_IREG_LOCKED, digitalRead(LOCK_DETECT) == HIGH);
      mb.Ists(MODBUS_ISTS_LOCKED, digitalRead(LOCK_DETECT) == HIGH);
}
