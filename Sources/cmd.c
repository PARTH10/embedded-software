/*! @file
 *
 *  @brief Implementations of functions which abstract communication with the computer into commands.
 *
 *  @author Robin Wohlers-Reichel, Joshua Gonsalves
 *  @date 2016-03-23
 */
/*!
**  @addtogroup cmd_module CMD module documentation
**  @{
*/
#include "cmd.h"

#include "packet.h"

/*
 * Tower software version V1.0
 */
const uint8_t TOWER_VERSION_H = 1;
const uint8_t TOWER_VERISON_L = 0;

uint8_t towerNumberLsb = (CMD_SID & 0x00FF);
uint8_t towerNumberMsb = (CMD_SID & 0xFF00) >> 8;

void CMD_TX_Startup_Packet()
{
  Packet_Put(CMD_TX_TOWER_STARTUP, 0x0, 0x0, 0x0);
}

void CMD_TX_Special_Tower_Version()
{
  Packet_Put(CMD_TX_TOWER_VERSION, 'v', TOWER_VERSION_H, TOWER_VERISON_L);
}

void CMD_TX_Tower_Number()
{
  Packet_Put(CMD_TX_TOWER_NUMBER, 1, towerNumberLsb, towerNumberMsb);
}

void CMD_RX_Tower_Number(uint8_t lsb, uint8_t msb)
{
  towerNumberLsb = lsb;
  towerNumberMsb = msb;
}

/*!
** @}
*/
