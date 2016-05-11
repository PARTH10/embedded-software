/*! @file
 *
 *  @brief Routines for controlling the Real Time Clock (RTC) on the TWR-K70F120M.
 *
 *  This contains the functions for operating the real time clock (RTC).
 *
 *  @author Josh Gonsalves, Robin Wohlers-Reichel
 *  @date 2016-04-27
 */
/*!
**  @addtogroup rtc_module RTC module documentation
**  @{
*/
#include "MK70F12.h"
#include <stdint.h>
#include <types.h>
#include "RTC.h"
#include "LEDs.h"

static void (*Callback)(void *);
static void *Arguments;
static BOOL Initialized = bFALSE;

BOOL RTC_Init(void (*userFunction)(void*), void* userArguments)
{
	Callback = userFunction;
	Arguments = userArguments;

	RTC_IER |= RTC_IER_TSIE_MASK; //Sets the interrupt
	RTC_IER &= ~RTC_IER_TAIE_MASK; //Disable Time Alarm Interrupt
	RTC_IER &= ~RTC_IER_TOIE_MASK; //Time Overflow Interrupt Interrupt
	RTC_IER &= ~RTC_IER_TIIE_MASK; //Time Invalid Interrupt

	//Clear the error if the invalid timer flag is set.
	if (RTC_SR & RTC_SR_TIF_MASK)
	{
		RTC_TSR = 0;
	}

	RTC_LR &= ~RTC_LR_CRL_MASK; // Lock control register
	RTC_SR |= RTC_SR_TCE_MASK; //Init timer control

	Initialized = bTRUE;
	return bTRUE;
}

void RTC_Set(const uint8_t hours, const uint8_t minutes, const uint8_t seconds)
{
	uint32_t timeSeconds = ((hours % 24) * 3600) + ((minutes % 60) * 60) + (seconds % 60);
	RTC_SR &= ~RTC_SR_TCE_MASK;
	RTC_TSR = timeSeconds;
  RTC_SR |= RTC_SR_TCE_MASK;
}

void RTC_Get(uint8_t* const hours, uint8_t* const minutes, uint8_t* const seconds)
{
	uint32_t currentTime = RTC_TSR;
	*hours = currentTime / 3600;
  *minutes = currentTime / 60 % 60;
	*seconds = currentTime % 60;
}

void __attribute__ ((interrupt)) RTC_ISR(void)
{
	//Don't try to run code at 0x0
	if (Initialized == bFALSE)
	{
		return;
	}
	(*Callback)(Arguments);
}

/*!
** @}
*/

