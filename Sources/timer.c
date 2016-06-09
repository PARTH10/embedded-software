/*! @file
 *
 *  @brief Routines for setting up the flexible timer module (FTM) on the TWR-K70F120M.
 *
 *  This contains the functions for operating the flexible timer module (FTM).
 *
 *  @author Josh Gonsalves, Robin Wohlers-Reichel
 *  @date 2015-09-04
 */
/*!
**  @addtogroup timer_module Timer module documentation
**  @{
*/
#include "timer.h"
#include "MK70F12.h"
#include "OS.h"

#include "LEDs.h"

#define PIT_CHANNEL_COUNT 8

static TTimer const *TimerCache[PIT_CHANNEL_COUNT] = {0};

BOOL Timer_Init()
{
	SIM_SCGC6 |= SIM_SCGC6_FTM0_MASK;

	FTM0_SC |= FTM_SC_CLKS(FTM_SC_CLKS_FIXED_FREQUENCY_CLOCK);
}

BOOL Timer_Set(const TTimer* const aTimer)
{
	//We have 8 channels
	if (aTimer->channelNb >= PIT_CHANNEL_COUNT)
	{
		return bFALSE;
	}

	//Only support simple output compare
	if (aTimer->timerFunction != TIMER_FUNCTION_OUTPUT_COMPARE)
	{
		return bFALSE;
	}

	//Need a function
	if (!aTimer->userFunction)
	{
		return bFALSE;
	}
	TimerCache[aTimer->channelNb] = aTimer;
}

BOOL Timer_Start(const TTimer* const aTimer)
{
	if (TimerCache[aTimer->channelNb] != aTimer)
	{
		return bFALSE;
	}
	FTM0_CnSC(aTimer->channelNb) = (FTM_CnSC_MSA_MASK | FTM_CnSC_CHIE_MASK);
	FTM0_CnV(aTimer->channelNb) = FTM0_CNT + aTimer->initialCount;
}

void __attribute__ ((interrupt)) FTM0_ISR(void)
{
	OS_ISREnter();
  uint32_t status = FTM0_STATUS;
	for (size_t i = 0; i < PIT_CHANNEL_COUNT; i++)
	{
		//if there is no timer for this channel, continue
		//if there is no status for this channel, continue
		if (TimerCache[i] && (status & (1 << i)))
		{
			FTM0_CnSC(i) &= ~FTM_CnSC_CHF_MASK;
			(TimerCache[i]->userFunction)(TimerCache[i]->userArguments);
		}
	}
	OS_ISRExit();
}

/*!
** @}
*/
