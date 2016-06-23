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

#include "types.h"

#include "threads.h"

#include "MK70F12.h"

/*!
 * @brief Called when a byte arrives. Must be set in the init.
 */
static void (*ReceiveCallback)(void);

/*!
 * @brief Signaled in the ISR when a byte is received.
 */
static OS_ECB *RxSemaphore;

/*!
 * @brief Signaled in the ISR when the hardware is ready to transmit.
 */
static OS_ECB *TxSemaphore;

/*!
 * @brief Stack for the receive thread.
 */
STATIC_STACK(ReceiveThreadStack);

/*!
 * @brief The receive thread.
 */
void ReceiveThread(void *data)
{
	for (;;)
	{
		OS_SemaphoreWait(RxSemaphore, 0);
		FIFO_Put(&RxFIFO, UART2_D);
		ReceiveCallback();
	}
}

/*!
 * @brief Stack for the transmit thread.
 */
STATIC_STACK(TransmitThreadStack);

/*!
 * @brief Thread which handles transmissions of bytes over the UART.
 */
void TransmitThread(void *data)
{
	for (;;)
	{
		OS_SemaphoreWait(TxSemaphore, 0);
		if (!(UART2_S1 & UART_S1_TDRE_MASK))
		{
			continue;
		}
		if (FIFO_Get(&TxFIFO, &UART2_D) == bFALSE)
		{
			UART2_C2 &= ~UART_C2_TCIE_MASK;
		}
	}
}

BOOL UART_Init(const uint32_t baudRate, const uint32_t moduleClk, void (*callback)(void))
{
	ReceiveCallback = callback;

	RxSemaphore = OS_SemaphoreCreate(0);
	TxSemaphore = OS_SemaphoreCreate(0);

	CREATE_THREAD(ReceiveThread, NULL, ReceiveThreadStack, TP_UART_RECEIVE);
	CREATE_THREAD(TransmitThread, NULL, TransmitThreadStack, TP_UART_TRANSMIT);

	//Initialize the FIFO buffers
	FIFO_Init(&RxFIFO);
	FIFO_Init(&TxFIFO);

	//UART setup

	SIM_SCGC4 |= SIM_SCGC4_UART2_MASK; // UART2 Clock gate control

	SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK; // Enable clock gate control for PORTE.

	PORTE_PCR16 |= PORT_PCR_MUX(3); // Set pin control register 16 PORTE bits 8 & 9, to enable the multiplexer alternative 3 (Transmitter).

	PORTE_PCR17 |= PORT_PCR_MUX(3); // Set pin control register 17 PORTE bits 8 & 9, to enable the multiplexer alternative 3 (Receiver).

//  UART2_C1 |= UART_C1_LOOPS_MASK; //Loop Mode Select
//  UART2_C1 |= UART_C1_UARTSWAI_MASK; //UART Stops in Wait Mode
//  UART2_C1 |= UART_C1_RSRC_MASK; //Receiver Source Select
//  UART2_C1 |= UART_C1_M_MASK; //9-bit or 8-bit Mode Select
//  UART2_C1 |= UART_C1_WAKE_MASK; //Receiver Wakeup Method Select
//  UART2_C1 |= UART_C1_ILT_MASK; //Idle Line Type Select
//  UART2_C1 |= UART_C1_PE_MASK; //Parity Enable
//  UART2_C1 |= UART_C1_PT_MASK; //Parity Type

//  UART2_C2 |= UART_C2_TIE_MASK; //Transmitter Interrupt or DMA Transfer Requests
	UART2_C2 |= UART_C2_TCIE_MASK; //Transmission Complete Interrupt Enable
	UART2_C2 |= UART_C2_RIE_MASK; //Receiver Full Interrupt or DMA Transfer Enable
//  UART2_C2 |= UART_C2_ILIE_MASK; //Idle Line Interrupt Enable
	UART2_C2 |= UART_C2_RE_MASK; // Enable UART2 receive.
	UART2_C2 |= UART_C2_TE_MASK; // Enable UART2 transmit.
//  UART2_C2 |= UART_C2_RWU_MASK; //Receiver Wakeup Control
//  UART2_C2 |= UART_C2_SBK_MASK; //Send Break

	//Set the requested baud rate
	uint16union_t setting;
	setting.l = (uint16_t) (moduleClk / (baudRate * 16));
	UART2_BDH |= (uint8_t) (setting.s.Hi & 0x1F);
	UART2_BDL = (uint8_t) setting.s.Lo;

	uint8_t fine_adjust = (uint8_t) ((moduleClk * 2) / baudRate) % 32;
	UART2_C4 = (fine_adjust & 0x1F);

	return bTRUE;
}

BOOL UART_InChar(uint8_t * const dataPtr)
{
	return FIFO_Get(&RxFIFO, dataPtr);
}

BOOL UART_OutChar(const uint8_t data)
{
	if (FIFO_Put(&TxFIFO, data))
	{
		UART2_C2 |= UART_C2_TCIE_MASK;
		return bTRUE;
	}
	return bFALSE;
}

void __attribute__ ((interrupt)) UART_ISR(void)
{
	OS_ISREnter();
	(UART2_S1 & UART_S1_TDRE_MASK) ? OS_SemaphoreSignal(TxSemaphore) : 0;
	(UART2_S1 & UART_S1_RDRF_MASK) ? OS_SemaphoreSignal(RxSemaphore) : 0;
	OS_ISRExit();
}

/*!
 ** @}
 */
