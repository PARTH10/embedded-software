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
#include "INT_I2C0.h"
#include "INT_PORTB.h"
#include "INT_SysTick.h"
#include "INT_PendableSrvReq.h"
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"

#include "accel.h"
#include "cmd.h"
#include "flash.h"
#include "I2C.h"
#include "LEDs.h"
#include "median.h"
#include "OS.h"
#include "packet.h"
#include "PIT.h"
#include "RTC.h"
#include "threads.h"
#include "timer.h"
#include "UART.h"

const static uint32_t BAUD_RATE = 115200;

/*!
 * @brief The module clock passed to submodules.
 */
const static uint32_t MODULE_CLOCK = CPU_BUS_CLK_HZ;

/*!
 * @brief Callback after the UART timer expires.
 */
void BlueOff(void *arguments)
{
  LEDs_Off(LED_BLUE);
}

/*!
 * @brief If asserted an accelerometer read should be scheduled.
 */
static uint8_t PendingAccelReadFlag = 0;

/*!
 * @brief Sets the PendingAccelReadFlag flag.
 */
void QueueAccelRead(void *argument)
{
  PendingAccelReadFlag = 1;
}

/*!
 * @brief If asserted there is new accelerometer data available in AccReadData[3].
 */
static uint8_t NewAccelDataFlag = 0;

/*!
 * @brief Contains the freshest accelerometer data.
 */
static uint8_t AccReadData[3] =
{ 0 };

/*!
 * @brief Callback after the accelerometer data is updated.
 */
void AccelReadCallback(void *data)
{
  NewAccelDataFlag = 1;
}

/*!
 * @brief Shifts the elements of an array one to the left.
 * @param array The array to shift.
 * @param len The length of the array.
 * @param newVal The new value to insert at index 0.
 */
void ShiftArray(uint8_t * const array, const size_t len, const uint8_t newVal)
{
  for (size_t i = (len - 1); i > 0; i--)
  {
    array[i] = array[i - 1];
  }
  array[0] = newVal;
}

/*!
 * @brief The last bytes of accelerometer data which were sent.
 */
static uint8_t AccelSendHistory[3] =
{ 0 };

/*!
 * @brief The last bytes of x read from the accelerometer in poll mode.
 */
static uint8_t AccelXHistory[3] =
{ 0 };

/*!
 * @brief The last bytes of y read from the accelerometer in poll mode.
 */
static uint8_t AccelYHistory[3] =
{ 0 };

/*!
 * @brief The last bytes of z read from the accelerometer in poll mode.
 */
static uint8_t AccelZHistory[3] =
{ 0 };

/*!
 * @brief Run on the main thread to handle new accelerometer data.
 */
void HandleNewAccelData()
{
  if (Accel_GetMode() == ACCEL_INT)
  {
    Packet_Put(0x10, AccReadData[0], AccReadData[1], AccReadData[2]);
    return;
  }

  //Shift history
  ShiftArray(AccelXHistory, 3, AccReadData[0]);
  ShiftArray(AccelYHistory, 3, AccReadData[1]);
  ShiftArray(AccelZHistory, 3, AccReadData[2]);

  uint8_t xMed = Median_Filter3(AccelXHistory[0], AccelXHistory[1], AccelXHistory[2]);
  uint8_t yMed = Median_Filter3(AccelYHistory[0], AccelYHistory[1], AccelYHistory[2]);
  uint8_t zMed = Median_Filter3(AccelZHistory[0], AccelZHistory[1], AccelZHistory[2]);

  if ((xMed != AccelSendHistory[0]) | (yMed != AccelSendHistory[1]) | (zMed != AccelSendHistory[2]))
  {
    AccelSendHistory[0] = xMed;
    AccelSendHistory[1] = yMed;
    AccelSendHistory[2] = zMed;
    (void) CMD_SendAccelerometerValues(AccelSendHistory);
  }
}

/*!
 * @brief FTM Timer run after a packet arrives.
 */
const static TTimer PacketTimer =
{ 0, 24414, TIMER_FUNCTION_OUTPUT_COMPARE, TIMER_OUTPUT_DISCONNECT, &BlueOff, (void *) 0 };

/*!
 * @brief Asserted if the "AccTimer" is running.
 */
static uint8_t AccTimerRunningFlag = 0;

/*!
 * @brief Called when the 1Hz timer expires, or when the
 *        accelerometer ISR fires.
 */
void AccTimerCallback(void *arguments)
{
  AccTimerRunningFlag = 0;
  QueueAccelRead((void *) 0);
}

/*!
 * @brief Timer to run the 1Hz accelerometer polling.
 */
const static TTimer AccTimer =
{ 1, 48828, TIMER_FUNCTION_OUTPUT_COMPARE, TIMER_OUTPUT_DISCONNECT, &AccTimerCallback, (void *) 0 };

/*!
 * @brief Accelerometer module setup structure.
 */
const static TAccelSetup AccelSetup =
{ CPU_BUS_CLK_HZ, &AccTimerCallback, (void *) 0, &AccelReadCallback, AccReadData };

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
    break;
  case CMD_RX_PROTOCOL_MODE:
    error = !CMD_ProtocolMode(Packet_Parameter1, Packet_Parameter2, Packet_Parameter3);
    break;
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
    Packet_Put(maskedPacket, Packet_Parameter1, Packet_Parameter2, Packet_Parameter3);
  }
}

static OS_ECB *RtcSemaphore;

STATIC_STACK(RtcThreadStack);

void RtcThread(void *data)
{
  for (;;)
  {
    OS_SemaphoreWait(RtcSemaphore, 0);
    uint8_t h, m, s;
    RTC_Get(&h, &m, &s);
    CMD_SendTime(h, m, s);
    LEDs_Toggle(LED_YELLOW);
  }
}

/*!
 * @brief Runs every second in response to the RTC interrupt.
 */
void RtcCallback(void *arguments)
{
  OS_SemaphoreSignal(RtcSemaphore);
//  uint8_t h, m, s;
//  RTC_Get(&h, &m, &s);
//  CMD_SendTime(h, m, s);
//  LEDs_Toggle(LED_YELLOW);
}

/*!
 * @brief Callback when the PIT expires.
 */
void PitCallback(void *arguments)
{
  LEDs_Toggle(LED_GREEN);
}

STATIC_STACK(PacketThreadStack);

void PacketThread(void *data)
{
  for (;;)
  {
    if (!Packet_Semaphore)
    {
      OS_TimeDelay(10);
      continue;
    }

    OS_SemaphoreWait(Packet_Semaphore, 0);
    LEDs_On(LED_BLUE);
    Timer_Start(&PacketTimer);
    HandlePacket();

    /*
     * If there is a new packet available,
     *  turn on the blue LED, start the timer
     *  to turn the LED off and finally
     *  handle the packet.
     *
     *  Packet_Get blocks until a packet is available.
     */
//    if (Packet_Get())
//    {
//      LEDs_On(LED_BLUE);
//      Timer_Start(&PacketTimer);
//      HandlePacket();
//    }
  }
}

STATIC_STACK(MainThreadStack);

void MainThread(void *data)
{
  for (;;)
  {
    OS_TimeDelay(10);
    /*
     * If there is a new packet available,
     *  turn on the blue LED, start the timer
     *  to turn the LED off and finally
     *  handle the packet.
     */
//    if (Packet_Get())
//    {
//      LEDs_On(LED_BLUE);
//      Timer_Start(&PacketTimer);
//      HandlePacket();
//    }
    /*
     * If the accelerometer 1Hz timer is *NOT* running and
     *  we are in poll mode, resta rt the timer.
     * Also reset the flag.
     */
    if (!AccTimerRunningFlag && (Accel_GetMode() == ACCEL_POLL))
    {
      AccTimerRunningFlag = 1;
      Timer_Start(&AccTimer);
    }

    /*
     * If there is a pending accelerometer read,
     *  read the XYZ values from the accelerometer
     *  and clear the flag.
     */
    if (PendingAccelReadFlag)
    {
      PendingAccelReadFlag = 0;
      Accel_ReadXYZ(AccReadData);
    }

    /*
     * If there is accelerometer data available,
     *  clear the flag and handle the data.
     */
    if (NewAccelDataFlag)
    {
      NewAccelDataFlag = 0;
      HandleNewAccelData();
    }
  }
}

static OS_ECB *InitSemaphore;

STATIC_STACK(InitThreadStack);

void InitThread(void *data)
{
  for (;;)
  {
    OS_SemaphoreWait(InitSemaphore, 0);
    //Initialize all the modules
    LEDs_Init();

    I2C_Init(100000, MODULE_CLOCK);
    Accel_Init(&AccelSetup);

    PIT_Init(MODULE_CLOCK, &PitCallback, (void *) 0);
    PIT_Set(500000000, bFALSE);
    PIT_Enable(bTRUE);

    Packet_Init(BAUD_RATE, MODULE_CLOCK);
    Flash_Init();
    CMD_Init();

    //Best to do this one last
//    RTC_Init(&RtcCallback, (void *) 0);
    RTC_Init(OS_SemaphoreSignal, (void *) RtcSemaphore);

    Timer_Init();
    Timer_Set(&PacketTimer);
    Timer_Set(&AccTimer);

    CMD_SpecialGetStartupValues();

    LEDs_On(LED_ORANGE);
  }
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
  OS_Init(CPU_CORE_CLK_HZ);

  InitSemaphore = OS_SemaphoreCreate(1);
  RtcSemaphore = OS_SemaphoreCreate(0);

  CREATE_THREAD(InitThread, NULL, InitThreadStack, TP_INITTHREAD);
  CREATE_THREAD(MainThread, NULL, MainThreadStack, TP_MAINTHREAD);
  CREATE_THREAD(PacketThread, NULL, PacketThreadStack, TP_PACKETTHREAD);
  CREATE_THREAD(RtcThread, NULL, RtcThreadStack, TP_RTCTHREAD);

  OS_Start();

  /*** Don't write any code pass this line, or it will be deleted during code generation. ***/
  /*** RTOS startup code. Macro PEX_RTOS_START is defined by the RTOS component. DON'T MODIFY THIS CODE!!! ***/
#ifdef PEX_RTOS_START
  PEX_RTOS_START(); /* Startup of the selected RTOS. Macro is defined by the RTOS component. */
#endif
  /*** End of RTOS startup code.  ***/
  /*** Processor Expert end of main routine. DON'T MODIFY THIS CODE!!! ***/
  for (;;)
  {
  }
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
