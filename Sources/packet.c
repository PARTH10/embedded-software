/*! @file
 *
 *  @brief Routines to implement packet encoding and decoding for the serial port.
 *
 *  Implementation of the packet module, handles 5 byte packets.
 *
 *  @author Robin Wohlers-Reichel, Joshua Gonsalves
 *  @date 2016-03-23
 */
/*!
**  @addtogroup packet_module Packet module documentation
**  @{
*/
#include "packet.h"

#include "cmd.h"
#include "UART.h"

static uint8_t Position = 0;

static uint8_t Checksum;

uint8_t Packet_Command,
	Packet_Parameter1,
	Packet_Parameter2,
	Packet_Parameter3;

const uint8_t PACKET_ACK_MASK = 0x80;

uint8_t PacketTest()
{
  uint8_t calc_checksum = Packet_Command ^ Packet_Parameter1 ^ Packet_Parameter2 ^ Packet_Parameter3;
  uint8_t ret_val = calc_checksum == Checksum;
}

BOOL Packet_Init(const uint32_t baudRate, const uint32_t moduleClk)
{
  UART_Init(baudRate, moduleClk);
}

BOOL Packet_Get(void)
{
  uint8_t uartData;
  if (!UART_InChar(&uartData))
  {
      return bFALSE;
  }
	switch (Position)
	{
	case 0:
		Packet_Command = uartData;
		Position++;
		return bFALSE;
	case 1:
		Packet_Parameter1 = uartData;
		Position++;
		return bFALSE;
	case 2:
		Packet_Parameter2 = uartData;
		Position++;
		return bFALSE;
	case 3:
		Packet_Parameter3 = uartData;
		Position++;
		return bFALSE;
	case 4:
		Checksum = uartData;
		if (PacketTest())
		{
			Position = 0;
			return bTRUE;
		}
		Packet_Command = Packet_Parameter1;
		Packet_Parameter1 = Packet_Parameter2;
		Packet_Parameter2 = Packet_Parameter3;
		Packet_Parameter3 = Checksum;
		return bFALSE;
	default:
		//reset the counter
		Position = 0;
		return bFALSE;
	}
  return bFALSE;
}

BOOL Packet_Put(const uint8_t command, const uint8_t p1, const uint8_t p2, const uint8_t p3)
{
	if (!UART_OutChar(command))
	{
		return bFALSE;
	}
	if (!UART_OutChar(p1))
	{
		return bFALSE;
	}
	if (!UART_OutChar(p2))
	{
		return bFALSE;
	}
	if (!UART_OutChar(p3))
	{
		return bFALSE;
	}
	if (!UART_OutChar(command ^ p1 ^ p2 ^ p3))
	{
		return bFALSE;
	}
	return bTRUE;
}

/*!
** @}
*/
