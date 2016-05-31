/*! @file
 *
 *  @brief I/O routines for the K70 I2C interface.
 *
 *  This contains the functions for operating the I2C (inter-integrated circuit) module.
 *
 *  @author PMcL
 *  @date 2015-09-17
 */
/*!
**  @addtogroup i2c_module I2C module documentation
**  @{
*/
#ifndef I2C_H
#define I2C_H

// new types
#include "types.h"

/*! @brief Sets up the I2C before first use.
 *
 *  @param aI2CModule is a structure containing the operating conditions for the module.
 *  @param moduleClk The module clock in Hz.
 *  @return BOOL - TRUE if the I2C module was successfully initialized.
 */
BOOL I2C_Init(const uint32_t baudRate, const uint32_t moduleClk);

/*! @brief Selects the current slave device
 *
 * @param slaveAddress The slave device address.
 */
void I2C_SelectSlaveDevice(const uint8_t slaveAddress);

/*! @brief Write a byte of data to a specified register
 *
 * @param registerAddress The register address.
 * @param data The 8-bit data to write.
 * @param waitCompletion Should wait to return until the operation completes.
 */
void I2C_Write(const uint8_t registerAddress, const uint8_t data, const BOOL waitCompletion);

/*! @brief Reads data of a specified length starting from a specified register
 *
 * Uses interrupts as the method of data reception.
 * @param registerAddress The register address.
 * @param destination An array with capacity nbBytes to store the bytes that are read.
 * @param nbBytes The number of bytes to read.
 * @param callback Callback after the operation completes.
 * @param callbackData Data for the callback.
 */
void I2C_IntRead(const uint8_t registerAddress, uint8_t* const destination, const uint8_t nbBytes, void (*callback)(void*), void *callbackData);

/*! @brief Synchronously reads data of a specified length starting from a specified register
 *
 * Uses interrupts as the method of data reception.
 * @param registerAddress The register address.
 * @param destination An array with capacity nbBytes to store the bytes that are read.
 * @param nbBytes The number of bytes to read.
 */
void I2C_PollRead(const uint8_t registerAddress, uint8_t* const destination, const uint8_t nbBytes);

/*!
 * @brief Interrupt service for the I2C module.
 */
void __attribute__ ((interrupt)) I2C_ISR(void);

#endif
/*!
** @}
*/
