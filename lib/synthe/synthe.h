/* *******************************************************************************
	NAME 			      : synthe.h
	DESCRIPTION		  : Synthe Main part related to Synthe LMX2572 programming	  
	AUTHOR			    : Philippe Borghini        
  COMMENTS        :
--------------------------------------------------------------------------------
DATE			VERSION	REVISOR 	DESCRIPTION				
--------------------------------------------------------------------------------
13.01.2025 	V0.1 	PHB		Initialization

-------------------------------------------------------------------------------- */

#include "tdef.h"

/* ******************************************************************************
			     DECLARATIONS / DEFINITIONS			      	   	    
****************************************************************************** */

//
#define VERSION "\n\rSynthe LMX25x2 Rel 0.4 F5jwf nov. 2025"
#define RELEASE_LEVEL 04
#define FALSE 0
#define TRUE  1


/* Compilation Options*/
//#define VERSION_LMX2572             //6.4GHz Synthetizer
#define VERSION_LMX2592           //9.8GHZ Synthetizer


//Hardware definition

#define LED_LIVE PIN_A6 //PA6 pin 4
#define SPARE_IO PIN_A5 //PA5 pin3
#define DATAOUT MOSI//PA1 pin 11
#define DATAIN  MISO //PA2 pin 12
#define SPICLOCK  SCK //PA3 pin13
#define SYNTHE_CS PIN_PA7 //Chip select pin 5
#define MODBUS_TX_ENABLE PIN_PA4 //Direction TX/RX MODBUS pin PA4 pin2


//Debug
#define DEBUG_NONE 0x00
#define DEBUG_ALL 0xff
#define DEBUG_SPI_PROG 0x01
#define DEBUG_INIT_PREFERED 0x02
#define DEBUG_SET_VALUE 0x03
#define DEBUG_SET_FIELD 0x04
#define DEBUG_MODBUS 0x05

//Default Values
//Move to synthe.cpp
//#define DEFAULT_FREQ            (U32)35900000          //3590MHz
//#define DEFAULT_PLL_PD          (U32)20000            //Phase detector freq 20MHz
//#define DEFAULT_PLL_N           (U32)179 //59 //179            // f= (179 +50/100)*20'000
//#define DEFAULT_PLL_NUM         (U32)0x8000 //0x54614 //0x8000             //U32 4 bytes
#define DEFAULT_PLL_DEN         (U32)0x00010000     //U32 4 bytes
#define DEFAULT_MODBUS_ADD      (U8)20  //Default Modbus Slave Add


//EEPROM Reservation for preference data !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!name to be changed
#define BOOT_COUNT_ADD              0   //u16, 2byte
#define DEFAULT_FREQ_ADD            2   //u32 4 bytes
#define DEFAULT_PLL_N_ADD           6   //U32 4 bytes
#define DEFAULT_PLL_NUM_ADD         10  //U32 4 bytes
#define DEFAULT_PLL_DEN_ADD         14  //U32 4 bytes
#define DEFAULT_POWER_ADD           18  //U8
#define FLASH_MODBUS_ADD            19  //U8

//PLL SET REGISTER
#define SET_FREQ      0x01
#define SET_PLL_N     0x02
#define SET_PLL_NUM   0x03
#define SET_PLL_DEN   0x04







// Modbus definition
//Address Modbus Registre READ
#define MODBUS_ADD_REG_R_RELEASE 	    101  // Release level Synthe ADD     		
		
//Address Modbus Registre WRITE
#define MODBUS_ADD_REG_W_FREQ_L 		    201  // Frequency    		
#define MODBUS_ADD_REG_W_FREQ_H		      202  // Frequency    	  		

		
//Address Modbus Registre READ/WRITE
#define MODBUS_ADD_REG_RW_UPDATE1 		301  //Register have been touched by Master



typedef union {
  uint64_t ull;
  S32 l[2];
  U32 ul[2];
  S16 i[4];
  U16 ui[4];
  S8 b[8];
  U8 ub[8];
} LONGVAL;

typedef union {
  boolean b[32];
  U8 u8[4];
  U16 u16[2];
  U32 u32;
} DOUBLEVAL;

/* Global Variable */
extern U32 U32_Time;
extern U8 U8_Verbose;
extern U8 U8_Debugg;
extern U32 pll_freq;
extern U32 pll_N;
extern U32 pll_num;
extern U32 pll_den;
extern U8 U8_modbus_slave_add;



/* Global prototypes decalaration */
extern void SYNTHE_display_page0(void);
extern void SYNTHE_init(void);
extern void SYNTHE_cold_start(void);
extern void SYNTHE_hot_start(void);
extern void SYNTHE_help(void);
extern void SYNTHE_SEQUENCER(void);
extern void SYNTHE_status(void);
extern void SYNTHE_debug(U8 debug);
extern void SYNTHE_prog_reg(U8 reg_nr, U16 value);
extern U16 SYNTHE_read_reg(U8 reg_nr);
extern void SYNTHE_read_all_reg(void);
extern void SYNTHE_reset_default(void);
extern void SYNTHE_LMX25XX_init_prefered(void);
extern void SYNTHE_set_power(U8 power);
extern void SYNTHE_set_PLL(U8 parameter, U32 value);
//extern void SYNTHE_modbus_init(void);
//extern void SYNTHE_modbus_publish(void);

/*-----------------------
LMX Registers definition
------------------------*/

#ifdef VERSION_LMX2572
//Default Registers Values for Synth LMX2572
/*
//Parameter from Design 1
#define LMX_REGISTER_TABLE_SIZE 126
///Design1;FVCO=3590M, Fpd=20M
#define DEFAULT_FREQ            (U32)35900000          //3590MHz
#define DEFAULT_PLL_PD          (U32)20000            //Phase detector freq 20MHz
#define DEFAULT_PLL_N           (U32)179 //59 //179            // f= (179 +50/100)*20'000
#define DEFAULT_PLL_NUM         (U32)0x8000 //0x54614 //0x8000             //U32 4 bytes
#define DEFAULT_PLL_DEN         (U32)0x00010000     //U32 4 bytes
PROGMEM const U32 LMX25xx_Register_Prefered[LMX_REGISTER_TABLE_SIZE] = {
    0x00221C, 0x010808, 0x020500, 0x030782, 0x040A43, 0x0530C8, 0x06C802, 0x0700B2, 0x082000, 0x091004, 0x0A10F8, 0x0BB018, 0x0C5001, 0x0D4000, 0x0E1878, 0x0F060E,	//0..15
    0x100080, 0x1100EC, 0x120064, 0x1327B7, 0x144848, 0x150409, 0x160001, 0x17007C, 0x18071A, 0x190624, 0x1A0808, 0x1B0002, 0x1C0488, 0x1D0000, 0x1E18A6, 0x1FC3E6,	//16..31
    0x2005BF, 0x211E01, 0x220010, 0x230004, 0x2400B3, 0x250305, 0x260001, 0x270000, 0x280000, 0x290000, 0x2A0000, 0x2B0032, 0x2C3FA2, 0x2DCE22, 0x2E07F0, 0x2F0300,	//32..47
    0x3003E0, 0x314180, 0x320080, 0x330080, 0x340421, 0x350000, 0x360000, 0x370000, 0x380000, 0x390020, 0x3A9001, 0x3B0001, 0x3C03E8, 0x3D00A8, 0x3E00AF, 0x3F0000,	//48..63
    0x401388, 0x410000, 0x4201F4, 0x430000, 0x4403E8, 0x450000, 0x46C350, 0x470081, 0x480001, 0x49003F, 0x4A0000, 0x4B0800, 0x4C000C, 0x4D0000, 0x4E02CF, 0x4F0180,	//64..79
    0x500000, 0x510000, 0x52FFFF, 0x53FFFF, 0x540001, 0x55EC00, 0x560000, 0x570000, 0x580000, 0x590000, 0x5A0000, 0x5B0000, 0x5C0000, 0x5D0000, 0x5E0000, 0x5F0000,	//80..95
    0x600000, 0x610000, 0x62005C, 0x630A3D, 0x6407D0, 0x650000, 0x660000, 0x670000, 0x6807D0, 0x694440, 0x6A0007, 0x6B0000, 0x6C0000, 0x6D0000, 0x6E0000, 0x6F0000,	//96..111
    0x700000, 0x710000, 0x727802, 0x730000, 0x740000, 0x750000, 0x760000, 0x770000, 0x780000, 0x790000, 0x7A0000, 0x7B0000, 0x7C0000, 0x7D2288	//112..125
}; */



//Parameter from Design 2
#define LMX_REGISTER_TABLE_SIZE 126
//Design2;FVCO=3590M, Fpd=60M, N=59, FNUM=54614 (0xd556), Pout REG44(0x3fa2)
#define DEFAULT_FREQ            (U32)35900000          //3590MHz
#define DEFAULT_PLL_PD          (U32)60000            //Phase detector freq 60MHz
#define DEFAULT_PLL_N           (U32)59 //179            // f= (179 +50/100)*20'000
#define DEFAULT_PLL_NUM         (U32)0x54614 //0x8000             //U32 4 bytes
#define DEFAULT_PLL_DEN         (U32)0x00010000     //U32 4 bytes
PROGMEM const U32 LMX25XX_Register_Prefered[LMX_REGISTER_TABLE_SIZE] = {
    0x00209C, 0x010808, 0x020500, 0x030782, 0x040A43, 0x0530C8, 0x06C802, 0x0700B2, 0x082000, 0x090004, 0x0A1378, 0x0BB018, 0x0C5001, 0x0D4000, 0x0E1878, 0x0F060E,	//0..15
    0x100080, 0x1100EC, 0x120064, 0x1327B7, 0x144848, 0x150409, 0x160001, 0x17007C, 0x18071A, 0x190624, 0x1A0808, 0x1B0002, 0x1C0488, 0x1D0000, 0x1E18A6, 0x1FC3E6,	//16..31
    0x2005BF, 0x211E01, 0x220010, 0x230004, 0x24003B, 0x250405, 0x260001, 0x270000, 0x280000, 0x290000, 0x2A0000, 0x2BD556, 0x2C3FA2, 0x2DCE22, 0x2E07F0, 0x2F0300,	//32..47
    0x3003E0, 0x314180, 0x320080, 0x330080, 0x340421, 0x350000, 0x360000, 0x370000, 0x380000, 0x390020, 0x3A9001, 0x3B0001, 0x3C03E8, 0x3D00A8, 0x3E00AF, 0x3F0000,	//48..63
    0x401388, 0x410000, 0x4201F4, 0x430000, 0x4403E8, 0x450000, 0x46C350, 0x470081, 0x480001, 0x49003F, 0x4A0000, 0x4B0800, 0x4C000C, 0x4D0000, 0x4E02CF, 0x4F0180,	//64..79
    0x500000, 0x510000, 0x52FFFF, 0x53FFFF, 0x540001, 0x55EC00, 0x560000, 0x570000, 0x580000, 0x590000, 0x5A0000, 0x5B0000, 0x5C0000, 0x5D0000, 0x5E0000, 0x5F0000,	//80..95
    0x600000, 0x610000, 0x62005C, 0x630A3D, 0x6407D0, 0x650000, 0x660000, 0x670000, 0x6807D0, 0x694440, 0x6A0007, 0x6B0000, 0x6C0000, 0x6D0000, 0x6E0000, 0x6F0000,	//96..111
    0x700000, 0x710000, 0x727802, 0x730000, 0x740000, 0x750000, 0x760000, 0x770000, 0x780000, 0x790000, 0x7A0000, 0x7B0000, 0x7C0000, 0x7D2288	//112..125
};
#endif

#ifdef VERSION_LMX2592
/*
//Default Registers Values for Synth LMX2592 Design x
#define LMX_REGISTER_TABLE_SIZE 46
//Design1;FVCO=7200M, Fpd=20M, N=90, FNUM=0
#define DEFAULT_FREQ            (U32)7200000          //7200000MHz
#define DEFAULT_PLL_PD          (U32)20000            //Phase detector freq 20MHz
#define DEFAULT_PLL_N           (U32)90             // f= 4 * (90 + 0/32768)*20MHz
#define DEFAULT_PLL_NUM         (U32)0x0             //U32 4 bytes
#define DEFAULT_PLL_DEN         (U32)0x8000   //U32 4 bytes
PROGMEM const U32 LMX25XX_Register_Prefered[LMX_REGISTER_TABLE_SIZE] = {
   0x00221C, 0x010808, 0x020500, 0x041943, 0x0728B2, 0x081084, 0x090B02, 0x0A10D8, 0x0B0018, 0x0C7001, 0x0D4000, 0x0E039C, 0x130965, 0x14012C, 0x162300, 0x178842,	//0..15
    0x180509, 0x190000, 0x1C2924, 0x1D0084, 0x1E0035, 0x1F0401, 0x20210A, 0x212A0A, 0x22C3EF, 0x23021B, 0x240BFF, 0x255000, 0x2600B4, 0x278204, 0x280000, 0x298000,	//16..31
    0x2A0000, 0x2B0000, 0x2C0000, 0x2D0000, 0x2E10A3, 0x2F08CF, 0x3003FC, 0x3B0000, 0x3D0001, 0x3E0000, 0x400177, 0x440089, 0x450000, 0x460000	//32..45
};

*/

///*
//Default Registers Values for Synth LMX2592 Design_4
//Design1;FVCO=7200M, Fpd=20M, N=90, FNUM=0
#define DEFAULT_FREQ            (U32)7200000          //7200000MHz
#define DEFAULT_PLL_PD          (U32)20000            //Phase detector freq 20MHz
#define DEFAULT_PLL_N           (U32)90             // f= 4 * (90 + 0/80'000)*20MHz
#define DEFAULT_PLL_NUM         (U32)0x0             //U32 4 bytes
#define DEFAULT_PLL_DEN         (U32)0x13880   //U32 4 bytes, 80'000, Resolution 1Hz

#define LMX_REGISTER_TABLE_SIZE 46
PROGMEM const U32 LMX25XX_Register_Prefered[LMX_REGISTER_TABLE_SIZE] = {
    0x00221C, 0x010808, 0x020500, 0x041943, 0x0728B2, 0x081084, 0x090B02, 0x0A10D8, 0x0B0018, 0x0C7001, 0x0D4000, 0x0E0840, 0x130965, 0x14012C, 0x162300, 0x178842,	//0..15
    0x180509, 0x190000, 0x1C2924, 0x1D0084, 0x1E0035, 0x1F0401, 0x20210A, 0x212A0A, 0x22C3CF, 0x230219, 0x24F3FF, 0x255000, 0x2600B4, 0x278204, 0x280001, 0x293880,	//16..31
    0x2A0000, 0x2B0000, 0x2C0000, 0x2D0000, 0x2E10A3, 0x2F08CF, 0x3003FD, 0x3B0000, 0x3D0001, 0x3E0000, 0x400177, 0x440089, 0x450000, 0x460000	//32..45
};



//*/



#endif