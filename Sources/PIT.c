/*! @file
 *
 *  @brief Routines for controlling Periodic Interrupt Timer (PIT) on the TWR-K70F120M.
 *
 *  This contains the functions for operating the periodic interrupt timer (PIT).
 *
 *  @author Josh Gonsalves, Robin Wohlers-Reichel
 *  @date 2016-04-27
 */

#include "PIT.h"
#include "MK70F12.h"

#define NS_IN_1_SEC 1000000000

static void (*Callback)(void *);
static void *Arguments;
static BOOL Initialized = bFALSE;
static uint32_t Clock;

BOOL PIT_Init(const uint32_t moduleClk, void (*userFunction)(void*), void* userArguments)
{
	Callback = userFunction;
	Arguments = userArguments;
	Clock = moduleClk;
  Initialized = bTRUE;

  SIM_SCGC6 |= SIM_SCGC6_PIT_MASK;
//  Where's my super suit?
  PIT_MCR = PIT_MCR_FRZ_MASK;//Clear the MDIS bit in one swoop!
  PIT_TCTRL0 |= PIT_TCTRL_TIE_MASK;
}

void PIT_Set(const uint32_t period, const BOOL restart)
{
  uint32_t hz = NS_IN_1_SEC / period;
	uint32_t cycleCount = Clock / hz;
	uint32_t triggerValue = cycleCount - 1;

	PIT_LDVAL0 = PIT_LDVAL_TSV(triggerValue);

	if (restart) {
		PIT_TCTRL0 &= ~PIT_TCTRL_TEN_MASK;
		PIT_TCTRL0 |= PIT_TCTRL_TEN_MASK;
	}
}

void PIT_Enable(const BOOL enable)
{
	if (enable)
	{
		PIT_TCTRL0 |= PIT_TCTRL_TEN_MASK;
	}
	else
	{
		PIT_TCTRL0 &= ~PIT_TCTRL_TEN_MASK;
	}
}

void __attribute__ ((interrupt)) PIT_ISR(void)
{
	//Don't try to run code at 0x0
	if (Initialized == bFALSE)
	{
		return;
	}
	(*Callback)(Arguments);
	PIT_TFLG0 = PIT_TFLG_TIF_MASK;
}
