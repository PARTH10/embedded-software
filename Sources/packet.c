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

uint8_t packet_position = 0;

uint8_t packet_checksum;

uint8_t Packet_Command,
	Packet_Parameter1,
	Packet_Parameter2,
	Packet_Parameter3;

const uint8_t PACKET_ACK_MASK = 0x80;

uint8_t PacketTest() {
  uint8_t calc_checksum = Packet_Command ^ Packet_Parameter1 ^ Packet_Parameter2 ^ Packet_Parameter3;
  uint8_t ret_val = calc_checksum == packet_checksum;
}

BOOL Packet_Init(const uint32_t baudRate, const uint32_t moduleClk)
{
  UART_Init(baudRate, moduleClk);
}

BOOL Packet_Get(void)
{
  uint8_t uartData;
  if (!UART_InChar(&uartData)) {
      return bFALSE;
  }
  switch(packet_position) {
    case 0:
      Packet_Command = uartData;
      packet_position++;
      return bFALSE;
    case 1:
      Packet_Parameter1 = uartData;
      packet_position++;
      return bFALSE;
    case 2:
      Packet_Parameter2 = uartData;
      packet_position++;
      return bFALSE;
    case 3:
      Packet_Parameter3 = uartData;
      packet_position++;
      return bFALSE;
    case 4:
      packet_checksum = uartData;
      if (PacketTest()) {
	  packet_position = 0;
	  return bTRUE;
      }
      Packet_Command = Packet_Parameter1;
      Packet_Parameter1 = Packet_Parameter2;
      Packet_Parameter2 = Packet_Parameter3;
      Packet_Parameter3 = packet_checksum;
      return bFALSE;
    default:
      //reset the counter
      packet_position = 0;
      return bFALSE;
  }
  return bFALSE;
}

BOOL Packet_Put(const uint8_t command, const uint8_t p1, const uint8_t p2, const uint8_t p3)
{
  UART_OutChar(command);
  UART_OutChar(p1);
  UART_OutChar(p2);
  UART_OutChar(p3);
  UART_OutChar(command ^ p1 ^ p2 ^ p3);
  return bTRUE;
}

void Packet_Handle()
{
  BOOL error = bTRUE;
  //mask out the ack, otherwise it goes to default
  switch(Packet_Command & ~PACKET_ACK_MASK) {
    case CMD_RX_GET_SPECIAL_START_VAL:
      CMD_TX_Startup_Packet();
      CMD_TX_Special_Tower_Version();
      CMD_TX_Tower_Number();
      error = bFALSE;
      break;
    case CMD_RX_GET_VERSION:
      CMD_TX_Special_Tower_Version();
      error = bFALSE;
      break;
    case CMD_RX_TOWER_NUMBER:
      if (Packet_Parameter1 == CMD_TOWER_NUMBER_GET) {
	  CMD_TX_Tower_Number();
      } else if (Packet_Parameter1 == CMD_TOWER_NUMBER_SET) {
	  CMD_RX_Tower_Number(Packet_Parameter2, Packet_Parameter3);
      }
      error = bFALSE;
      break;
    default:
      break;
  }
  if (Packet_Command & PACKET_ACK_MASK) {
      uint8_t maskedPacket = 0;
      if (error == bTRUE) {
	  maskedPacket = Packet_Command & ~PACKET_ACK_MASK;
      } else {
	  maskedPacket = Packet_Command | PACKET_ACK_MASK;
      }
      Packet_Put(maskedPacket, Packet_Parameter1, Packet_Parameter2, Packet_Parameter3);
  }
}

/*!
** @}
*/
