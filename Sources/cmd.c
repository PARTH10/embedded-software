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

#include "flash.h"
#include "packet.h"
#include "RTC.h"
#include "types.h"

/*
 * Tower software version V1.0
 */
const uint8_t TOWER_VERSION_H = 1;
const uint8_t TOWER_VERISON_L = 0;

static uint16union_t volatile *TowerNumber;
static uint16union_t volatile *TowerMode;

BOOL CMD_Init()
{
	BOOL allocNumber = Flash_AllocateVar((volatile void **) &TowerNumber, sizeof(uint16union_t));
	BOOL allocMode = Flash_AllocateVar((volatile void **) &TowerMode, sizeof(uint16union_t));
	if (allocNumber == bTRUE && allocMode == bTRUE)
	{
		if (TowerNumber->l == 0xFFFF)
		{
			Flash_Write16((uint16_t volatile *) TowerNumber, CMD_SID);
		}
		if (TowerMode->l == 0xFFFF)
		{
			Flash_Write16((uint16_t volatile *) TowerMode, 0x1);
		}
		return bTRUE;
	}
	return bFALSE;
}

BOOL CMD_SpecialGetStartupValues()
{
	if (!Packet_Put(CMD_TX_TOWER_STARTUP, 0x0, 0x0, 0x0))
	{
		return bFALSE;
	}
	if (!CMD_SpecialTowerVersion())
	{
		return bFALSE;
	}
	if (!Packet_Put(CMD_TX_TOWER_NUMBER, 1, TowerNumber->s.Lo, TowerNumber->s.Hi))
	{
		return bFALSE;
	}
	if (!Packet_Put(CMD_TX_TOWER_MODE, 0x1, TowerMode->s.Lo, TowerMode->s.Hi))
	{
		return bFALSE;
	}
	return bTRUE;
}

BOOL CMD_FlashProgramByte(const uint8_t offset, const uint8_t data)
{
	if (offset > 8)// Change 8 to FLASH_DATA_SIZE for dynamicness
	{
		return bFALSE;
	}
	if (offset == 8)// Change 8 to FLASH_DATA_SIZE for dynamicness
	{
		return Flash_Erase();
	}
	uint8_t *address = (uint8_t *)(FLASH_DATA_START + offset);
	return Flash_Write8(address, data);
}

BOOL CMD_FlashReadByte(const uint8_t offset)
{
  if (offset > (FLASH_DATA_SIZE - 1))
  {
  	return bFALSE;
  }
  uint8_t data = _FB(FLASH_DATA_START + offset);
  return Packet_Put(CMD_TX_FLASH_READ_BYTE, offset, 0x0, data);
}

BOOL CMD_TowerNumber(uint8_t mode, uint8_t lsb, uint8_t msb)
{
	if (mode == CMD_TOWER_NUMBER_GET)
	{
		return Packet_Put(CMD_TX_TOWER_NUMBER, 1, TowerNumber->s.Lo, TowerNumber->s.Hi);
	}
	else if (mode == CMD_TOWER_NUMBER_SET)
	{
		uint16union_t temp;
		temp.s.Hi = msb;
		temp.s.Lo = lsb;
		return Flash_Write16((uint16_t volatile *) TowerNumber, temp.l);
	}
	return bFALSE;
}

BOOL CMD_TowerMode(const uint8_t mode, const uint8_t lsb, const uint8_t msb)
{
	if (mode == CMD_TOWER_MODE_GET)
	{
		return Packet_Put(CMD_TX_TOWER_MODE, 0x1, TowerMode->s.Lo, TowerMode->s.Hi);
	}
	else if (mode == CMD_TOWER_NUMBER_SET)
	{
		uint16union_t temp;
		temp.s.Hi = msb;
		temp.s.Lo = lsb;
		return Flash_Write16((uint16_t volatile *) TowerMode, temp.l);
	}
	return bFALSE;
}

BOOL CMD_StartupPacket()
{
  return Packet_Put(CMD_TX_TOWER_STARTUP, 0x0, 0x0, 0x0);
}

BOOL CMD_SpecialTowerVersion()
{
  return Packet_Put(CMD_TX_SPECIAL_TOWER_VERSION, 'v', TOWER_VERSION_H, TOWER_VERISON_L);
}

BOOL CMD_SendTime(const uint8_t hours, const uint8_t minutes, const uint8_t seconds)
{
	return Packet_Put(CMD_TX_TIME, hours, minutes, seconds);
}

BOOL CMD_SetTime(const uint8_t hours, const uint8_t minutes, const uint8_t seconds)
{
	if (hours > 23 || minutes > 59 || seconds > 59)
	{
		return bFALSE;
	}
	RTC_Set(hours, minutes, seconds);
	return bTRUE;
}

/*!
** @}
*/
