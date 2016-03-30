/*! @file
 *
 *  @brief I/O routines for UART communications on the TWR-K70F120M.
 *
 *  Implementation of the UART module, for handling UART communication.
 *
 *  @author Robin Wohlers-Reichel, Joshua Gonsalves
 *  @date 2016-03-23
 */
/*!
**  @addtogroup uart_module UART module documentation
**  @{
*/
#include "UART.h"

#include "MK70F12.h"

BOOL UART_Init(const uint32_t baudRate, const uint32_t moduleClk)
{
  //UART setup

  SIM_SCGC4 |= SIM_SCGC4_UART2_MASK;// Enable clock gate control for UART2.
  SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;// Enable clock gate control for PORTE.
  PORTE_PCR16 |= PORT_PCR_MUX(3);// Set pin control register 16 PORTE bits 8 & 9, to enable the multiplexer alternative 3 (Transmitter).
  PORTE_PCR17 |= PORT_PCR_MUX(3);// Set pin control register 17 PORTE bits 8 & 9, to enable the multiplexer alternative 3 (Receiver).
  UART2_C1 = 0;// Clear UART2 control register 1.
  UART2_C2 |= UART_C2_RE_MASK;// Enable UART2 receive.
  UART2_C2 |= UART_C2_TE_MASK;// Enable UART2 transmit.

  //Set the requested baud rate
  uint16_t setting = (uint16_t)(moduleClk/(baudRate * 16));
  uint8_t setting_high = (setting & 0x1F00) >> 8;
  uint8_t setting_low = (setting & 0xFF);
  UART2_BDH = (UART2_BDH & 0xC0) | (setting_high & 0x1F);
  UART2_BDL = setting_low;

  //Initialise the FIFO buffers
  FIFO_Init(&RxFIFO);
  FIFO_Init(&TxFIFO);

  return bTRUE;
}

BOOL UART_InChar(uint8_t * const dataPtr)
{
  return FIFO_Get(&RxFIFO, dataPtr);
}

BOOL UART_OutChar(const uint8_t data)
{
  return FIFO_Put(&TxFIFO, data);
}

void UART_Poll()
{
  if (UART2_S1 & UART_S1_TDRE_MASK) {
      FIFO_Get(&TxFIFO, &UART2_D);
  }
  if (UART2_S1 & UART_S1_RDRF_MASK) {
      FIFO_Put(&RxFIFO, UART2_D);
  }
}

/*!
** @}
*/
