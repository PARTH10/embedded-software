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

 /*!
  * The PC will issue this command upon startup
  * to retrieve the state of the Tower to update
  * the interface application.
  * */
#define CMD_RX_GET_SPECIAL_START_VAL 0x4

/*!
 * Get the version of the Tower software.
 */
#define CMD_RX_GET_VERSION 0x9

/*!
 * Get or set the Student ID associated with
 * the Tower software.
 */
#define CMD_RX_TOWER_NUMBER 0xb

/*!
 * The Tower will issue this command upon startup to
 * allow the PC to update the interface application
 * and the Tower. Typically, setup data will also be
 * sent from the Tower to the PC.
 */
#define CMD_TX_TOWER_STARTUP 0x4

/*!
 * Send the tower version to the PC.
 */
#define CMD_TX_TOWER_VERSION 0x9

/*!
 * Send the tower number to the PC.
 */
#define CMD_TX_TOWER_NUMBER 0xb

/*!
 * Packet parameter 1 to get tower number.
 */
#define CMD_TOWER_NUMBER_GET 1

/*!
 * Packet parameter 1 to set tower number.
 */
#define CMD_TOWER_NUMBER_SET 2

/*!
 * The lower 2 bytes of 12011146.
 */
#define CMD_SID 0x468A

/*!
 * @brief Sends the startup packet to the computer.
 */
void CMD_TX_Startup_Packet();

/*!
 * @brief Sends the tower version to the computer.
 */
void CMD_TX_Special_Tower_Version();

/*!
 * @brief Sends the tower number to the computer.
 */
void CMD_TX_Tower_Number();

/*!
 * @brief Saves the tower number to a buffer.
 * @param lsb Least significant byte of the Tower number
 * @param msb Most significant byte of the Tower number
 */
void CMD_RX_Tower_Number(uint8_t lsb, uint8_t msb);

/*!
** @}
*/

#endif
