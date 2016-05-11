/* ###################################################################
**     Filename    : main.c
**     Project     : Lab1
**     Processor   : MK70FN1M0VMJ12
**     Version     : Driver 01.01
**     Compiler    : GNU C Compiler
**     Date/Time   : 2015-07-20, 13:27, # CodeGen: 0
**     Abstract    :
**         Main module.
**         This module contains user's application code.
**     Settings    :
**     Contents    :
**         No public methods
**
** ###################################################################*/
/*!
** @file main.c
** @version 1.0
** @brief
**         Main module.
**         This module contains user's application code.
*/         
/*!
**  @addtogroup main_module main module documentation
**  @{
*/         
/* MODULE main */


// CPU mpdule - contains low level hardware initialization routines
#include "Cpu.h"
#include "Events.h"
#include "INT_UART2_RX_TX.h"
#include "INT_RTC_Seconds.h"
#include "INT_PIT0.h"
#include "INT_FTM0.h"
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"

#include "cmd.h"
#include "flash.h"
#include "LEDs.h"
#include "packet.h"
#include "PIT.h"
#include "RTC.h"
#include "timer.h"
#include "UART.h"

const static uint32_t BAUD_RATE = 115200;
const static uint32_t MODULE_CLOCK = CPU_BUS_CLK_HZ;

void BlueOff(void *arguments)
{
	LEDs_Toggle(LED_BLUE);
}

const static TTimer PacketTimer = {0,
																 24414,
																 TIMER_FUNCTION_OUTPUT_COMPARE,
																 TIMER_OUTPUT_DISCONNECT,
																 &BlueOff,
																 (void *)0};

/*!
 * @brief Handle incoming packets
 */
void HandlePacket()
{
	BOOL error = bTRUE;
	uint8_t data;
	//mask out the ack, otherwise it goes to default
	switch (Packet_Command & ~PACKET_ACK_MASK)
	{
	case CMD_RX_SPECIAL_GET_STARTUP_VALUES:
		error = !CMD_SpecialGetStartupValues();
		break;
	case CMD_RX_FLASH_PROGRAM_BYTE:
		error = !CMD_FlashProgramByte(Packet_Parameter1, Packet_Parameter3);
		break;
	case CMD_RX_FLASH_READ_BYTE:
		error = !CMD_FlashReadByte(Packet_Parameter1);
		break;
	case CMD_RX_SPECIAL_GET_VERSION:
		error = !CMD_SpecialTowerVersion();
		break;
	case CMD_RX_TOWER_NUMBER:
		error = !CMD_TowerNumber(Packet_Parameter1, Packet_Parameter2, Packet_Parameter3);
		break;
	case CMD_RX_TOWER_MODE:
		error = !CMD_TowerMode(Packet_Parameter1, Packet_Parameter2, Packet_Parameter3);
		break;
	case CMD_RX_SET_TIME:
		error = !CMD_SetTime(Packet_Parameter1, Packet_Parameter2, Packet_Parameter3);
	default:
		break;
	}

	if (Packet_Command & PACKET_ACK_MASK)
	{
		uint8_t maskedPacket = 0;
		if (error)
		{
			maskedPacket = Packet_Command & ~PACKET_ACK_MASK;
		}
		else
		{
			maskedPacket = Packet_Command | PACKET_ACK_MASK;
		}
		Packet_Put(maskedPacket, Packet_Parameter1, Packet_Parameter2,
				Packet_Parameter3);
	}
}

void RtcCallback(void *arguments)
{
	uint8_t h, m, s;
	RTC_Get(&h, &m, &s);
	CMD_SendTime(h, m, s);
	LEDs_Toggle(LED_YELLOW);
}

void PitCallback(void *arguments)
{
	LEDs_Toggle(LED_GREEN);
}

/*lint -save  -e970 Disable MISRA rule (6.3) checking. */
/*!
 * @brief The entry point into the program.
 */
int main(void)
/*lint -restore Enable MISRA rule (6.3) checking. */
{
  /* Write your local variable definition here */

  /*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
	PE_low_level_init();
  /*** End of Processor Expert internal initialization.                    ***/

  /* Write your code here */

  //Initialize all the modules
  LEDs_Init();

  PIT_Init(MODULE_CLOCK, &PitCallback, (void *)0);
  PIT_Set(500000000, bFALSE);
  PIT_Enable(bTRUE);

  Packet_Init(BAUD_RATE, MODULE_CLOCK);
  Flash_Init();
  CMD_Init();

  //Best to do this one last
  RTC_Init(&RtcCallback, (void *)0);

  Timer_Init();
  Timer_Set(&PacketTimer);

  CMD_SpecialGetStartupValues();

  LEDs_On(LED_ORANGE);

	for (;;)
	{
		if (Packet_Get())
		{
			LEDs_On(LED_BLUE);
			Timer_Start(&PacketTimer);
			HandlePacket();
		}
	}

  /*** Don't write any code pass this line, or it will be deleted during code generation. ***/
  /*** RTOS startup code. Macro PEX_RTOS_START is defined by the RTOS component. DON'T MODIFY THIS CODE!!! ***/
  #ifdef PEX_RTOS_START
    PEX_RTOS_START();                  /* Startup of the selected RTOS. Macro is defined by the RTOS component. */
  #endif
  /*** End of RTOS startup code.  ***/
  /*** Processor Expert end of main routine. DON'T MODIFY THIS CODE!!! ***/
  for(;;){}
  /*** Processor Expert end of main routine. DON'T WRITE CODE BELOW!!! ***/
} /*** End of main routine. DO NOT MODIFY THIS TEXT!!! ***/

/* END main */
/*!
** @}
*/
/*
** ###################################################################
**
**     This file was created by Processor Expert 10.5 [05.21]
**     for the Freescale Kinetis series of microcontrollers.
**
** ###################################################################
*/
