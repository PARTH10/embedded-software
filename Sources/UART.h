/*! @file
 *
 *  @brief I/O routines for UART communications on the TWR-K70F120M.
 *
 *  This contains the functions for operating the UART (serial port).
 *
 *  @author PMcL
 *  @date 2015-07-23
 */
/*!
**  @addtogroup uart_module UART module documentation
**  @{
*/
#ifndef UART_H
#define UART_H

// new types
#include "types.h"

#include "FIFO.h"
#include "OS.h"

TFIFO RxFIFO, TxFIFO;

/*! @brief Sets up the UART interface before first use.
 *
 *  @param baudRate The desired baud rate in bits/sec.
 *  @param moduleClk The module clock rate in Hz.
 *  @param callback Function to call when the operation completes.
 *  @return BOOL - TRUE if the UART was successfully initialized.
 *  @note The callback must be set.
 */
BOOL UART_Init(const uint32_t baudRate, const uint32_t moduleClk, void(*callback)(void));
 
/*! @brief Get a character from the receive FIFO if it is not empty.
 *
 *  @param dataPtr A pointer to memory to store the retrieved byte.
 *  @return BOOL - TRUE if the receive FIFO returned a character.
 *  @note Assumes that UART_Init has been called.
 */
BOOL UART_InChar(uint8_t * const dataPtr);
 
/*! @brief Put a byte in the transmit FIFO if it is not full.
 *
 *  @param data The byte to be placed in the transmit FIFO.
 *  @return BOOL - TRUE if the data was placed in the transmit FIFO.
 *  @note Assumes that UART_Init has been called.
 */
BOOL UART_OutChar(const uint8_t data);

/*! @brief Interrupt service routine for the UART.
 *
 *  @note Assumes the transmit and receive FIFOs have been initialized.
 */
void __attribute__ ((interrupt)) UART_ISR(void);

/*!
** @}
*/

#endif
