/*! @file
 *
 *  @brief Functions which abstract communication with the computer into commands.
 *
 *  @author Robin Wohlers-Reichel, Joshua Gonsalves
 *  @date 2016-03-23
 */
/*!
**  @addtogroup cmd_module CMD module documentation
**  @{
*/
#ifndef CMD_H
#define CMD_H

#include "types.h"

/*****************************************
 * Packets Transmitted from Tower to PC
 */

/*!
 * The Tower will issue this command upon startup to
 * allow the PC to update the interface application
 * and the Tower. Typically, setup data will also be
 * sent from the Tower to the PC.
 */
#define CMD_TX_TOWER_STARTUP 0x04

/*!
 * Send the result of a flash read operation
 */
#define CMD_TX_FLASH_READ_BYTE 0x08

/*!
 * Send the tower version to the PC.
 */
#define CMD_TX_SPECIAL_TOWER_VERSION 0x09

/*!
 * Send the tower number to the PC.
 */
#define CMD_TX_TOWER_NUMBER 0x0b

/*!
 * Send the current time of the RTC
 */
#define CMD_TX_TIME 0x0c

/*!
 * Send the tower mode to the PC application
 */
#define CMD_TX_TOWER_MODE 0x0d

/*****************************************
 * Packets Transmitted from PC to Tower
 */

 /*!
  * The PC will issue this command upon startup
  * to retrieve the state of the Tower to update
  * the interface application.
  * */
#define CMD_RX_SPECIAL_GET_STARTUP_VALUES 0x04

/*!
 * Program a byte of flash
 */
#define CMD_RX_FLASH_PROGRAM_BYTE 0x07

/*!
 * Read a byte of flash
 */
#define CMD_RX_FLASH_READ_BYTE 0x08

/*!
 * Get the version of the Tower software.
 */
#define CMD_RX_SPECIAL_GET_VERSION 0x09

/*!
 * Get or set the Student ID associated with
 * the Tower software.
 */
#define CMD_RX_TOWER_NUMBER 0x0b

/*!
 * Set the time of the RTC
 */
#define CMD_RX_SET_TIME 0x0c

/*!
 * Get or set the tower mode
 */
#define CMD_RX_TOWER_MODE 0x0d

/*!
 * Packet parameter 1 to get tower number.
 */
#define CMD_TOWER_NUMBER_GET 1

/*!
 * Packet parameter 1 to set tower number.
 */
#define CMD_TOWER_NUMBER_SET 2

/*!
 * Packet parameter 1 to get tower mode.
 */
#define CMD_TOWER_MODE_GET 1

/*!
 * Packet parameter 1 to set tower mode.
 */
#define CMD_TOWER_MODE_SET 2

/*!
 * The lower 2 bytes of 12011146.
 */
#define CMD_SID 0x468A

/*!
 * @brief Set up the tower number and mode flash allocation.
 * @note Requires the flash module to be started.
 */
BOOL CMD_Init();

/*!
 * @brief Send the special startup values.
 * @return BOOL TRUE if the operation succeeded.
 */
BOOL CMD_SpecialGetStartupValues();

/*!
 * @brief Programs a byte of flash at the specified offset.
 * @param offset Offset of the byte from the start of the sector.
 * @param data The byte to write.
 * @note An offset greater than 7 will erase the sector.
 * @return BOOL TRUE if the operation succeeded.
 */
BOOL CMD_FlashProgramByte(const uint8_t offset, const uint8_t data);

/*!
 * @brief Read a byte of the flash and send it over the UART.
 * @param offset Offset of the byte from the start of the sector.
 * @note An offset past the end of the flash will fail.
 * @return BOOL TRUE if the operation succeeded.
 */
BOOL CMD_FlashReadByte(const uint8_t offset);

/*!
 * @brief Saves the tower number to a buffer.
 * @param mode Getting or setting.
 * @param lsb Least significant byte of the Tower number
 * @param msb Most significant byte of the Tower number
 * @return BOOL TRUE if the operation succeeded.
 */
BOOL CMD_TowerNumber(uint8_t mode, uint8_t lsb, uint8_t msb);

/*!
 * @brief Set the tower mode and save to flash.
 * @param mode the mode to set to.
 * @param lsb The least significant byte.
 * @param msb The most significant byte.
 * @return BOOL TRUE if the operation succeeded.
 */
BOOL CMD_TowerMode(const uint8_t mode, const uint8_t lsb, const uint8_t msb);

/*!
 * @brief Sends the startup packet to the computer.
 * @return BOOL TRUE if the operation succeeded.
 */
BOOL CMD_StartupPacket();

/*!
 * @brief Sends the tower version to the computer.
 * @return BOOL TRUE if the operation succeeded.
 */
BOOL CMD_SpecialTowerVersion();

/*!
 * @brief Sends the time to the pc.
 * @param hours The count of hours which have elapsed.
 * @param minutes The count of minutes which have elapsed.
 * @param seconds The number of seconds which have elapsed.
 * @return BOOL TRUE if the operation succeeded.
 */
BOOL CMD_SendTime(const uint8_t hours, const uint8_t minutes, const uint8_t seconds);

/*!
 * @brief Set the time of the RTC.
 * @param hours The count of hours which have elapsed.
 * @param minutes The count of minutes which have elapsed.
 * @param seconds The number of seconds which have elapsed.
 * @return BOOL TRUE if the operation succeeded.
 */
BOOL CMD_SetTime(const uint8_t hours, const uint8_t minutes, const uint8_t seconds);


/*!
** @}
*/

#endif
