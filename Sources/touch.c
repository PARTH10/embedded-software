/*
 * touch.c
 *
 *  Created on: 23 Jun 2016
 *      Author: 12011146
 */

#include "touch.h"

#include "OS.h"
#include "Cpu.h"
#include "types.h"
#include "LEDs.h"

#include "MK70F12.h"

#define TSI0_BLUE_CNTR ((uint16_t)(TSI0_CNTR9 >> 16))
#define TSI0_GREEN_CNTR ((uint16_t)(TSI0_CNTR7 >> 16))
#define TSI0_YELLOW_CNTR ((uint16_t)(TSI0_CNTR9))
#define TSI0_ORANGE_CNTR ((uint16_t)(TSI0_CNTR5 >> 16))

#define TSI0_BLUE_CNTR_THRESHOLD Calibration[TOUCH_BLUE_SHIFT]
#define TSI0_GREEN_CNTR_THRESHOLD Calibration[TOUCH_GREEN_SHIFT]
#define TSI0_YELLOW_CNTR_THRESHOLD Calibration[TOUCH_YELLOW_SHIFT]
#define TSI0_ORANGE_CNTR_THRESHOLD Calibration[TOUCH_ORANGE_SHIFT]

#define THRESHOLD_CALIBRATION_FACTOR 1.1f

static OS_ECB *Semaphore;
volatile uint8_t State;
static uint16_t Calibration[4];
static uint8_t CalibrationSamplesRemaining = 10;

void Touch_Init()
{
	Semaphore = OS_SemaphoreCreate(0);
	SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTB_MASK | SIM_SCGC5_TSI_MASK;

	/*
	 * TSI Settings
	 * GENCS:
	 * 	Prescaler 4 -> 4
	 * 	Scan Count 10 -> 9
	 * SCANC:
	 * 	Charge current 32uA -> 15
	 * 	External charge 10uA -> 8
	 * 	Scan period modulus -> 10
	 *	Clock source LPO -> 0x0
	 *	Active mode prescaler 2 -> 1
	 * PEN:
	 *  Enable 5,7,8,9.
	 */
	TSI0_GENCS |= TSI_GENCS_PS(4) | TSI_GENCS_NSCN(9);
	TSI0_SCANC |= TSI_SCANC_REFCHRG(15) | TSI_SCANC_EXTCHRG(8) | TSI_SCANC_SMOD(10) | TSI_SCANC_AMCLKS(0x0) | TSI_SCANC_AMPSC(1);
	TSI0_PEN |= TSI_PEN_PEN5_MASK | TSI_PEN_PEN7_MASK | TSI_PEN_PEN8_MASK | TSI_PEN_PEN9_MASK;

	/*
	 * Default values for the PORTB multiplexers are fine.
	 * PORTA Pin 4 must be changed
	 */
	PORTA_PCR4 | PORT_PCR_MUX(0);

	/*
	 * Default GPIO direction is input (GPIO(a,b)_PDDR. 59.2.6 in manual.
	 */

	/*
	 *
	 */
	TSI0_GENCS |= TSI_GENCS_TSIEN_MASK | TSI_GENCS_ESOR_MASK | TSI_GENCS_STM_MASK;
}

void Touch_EnableScan(BOOL enable)
{
	if (enable)
	{
		TSI0_GENCS |= TSI_GENCS_TSIIE_MASK;
	}
	else
	{
		TSI0_GENCS &= ~TSI_GENCS_TSIIE_MASK;
	}
}

OS_ERROR Touch_Wait(uint16_t timeout, uint8_t *values)
{
	OS_ERROR error = OS_SemaphoreWait(Semaphore, timeout);
	*values = State;
	return error;
}

void __attribute__ ((interrupt)) TSI_ISR(void)
{
	OS_ISREnter();
	if (!(TSI0_GENCS & TSI_GENCS_EOSF_MASK))
	{
		OS_ISRExit();
		return;
	}
	TSI0_GENCS |= TSI_GENCS_EOSF_MASK;

	if (CalibrationSamplesRemaining > 0)
	{
		CalibrationSamplesRemaining--;
		Calibration[TOUCH_BLUE_SHIFT] = TSI0_BLUE_CNTR * THRESHOLD_CALIBRATION_FACTOR;
		Calibration[TOUCH_GREEN_SHIFT] = TSI0_GREEN_CNTR * THRESHOLD_CALIBRATION_FACTOR;
		Calibration[TOUCH_YELLOW_SHIFT] = TSI0_YELLOW_CNTR * THRESHOLD_CALIBRATION_FACTOR;
		Calibration[TOUCH_ORANGE_SHIFT] = TSI0_ORANGE_CNTR * THRESHOLD_CALIBRATION_FACTOR;

		//Still calibrating. Don't return any data yet.
		OS_ISRExit();
		return;
	}

	uint8_t tempMask = 0;

	if (TSI0_BLUE_CNTR > TSI0_BLUE_CNTR_THRESHOLD)
	{
		//note the equals, not |= for the first statement
		tempMask = TOUCH_BLUE_MASK;
	}

	if (TSI0_GREEN_CNTR > TSI0_GREEN_CNTR_THRESHOLD)
	{
		tempMask |= TOUCH_GREEN_MASK;
	}

	if (TSI0_YELLOW_CNTR > TSI0_YELLOW_CNTR_THRESHOLD)
	{
		tempMask |= TOUCH_YELLOW_MASK;
	}

	if (TSI0_ORANGE_CNTR > TSI0_ORANGE_CNTR_THRESHOLD)
	{
		tempMask |= TOUCH_ORANGE_MASK;
	}

//	uint8_t a = (State & TOUCH_YELLOW_MASK);
//	uint8_t b = (tempMask & TOUCH_YELLOW_MASK);
//	uint8_t c = !a;

	uint8_t bRising = (!(State & TOUCH_BLUE_MASK) && (tempMask & TOUCH_BLUE_MASK));
	uint8_t gRising = (!(State & TOUCH_GREEN_MASK) && (tempMask & TOUCH_GREEN_MASK));
	uint8_t yRising = (!(State & TOUCH_YELLOW_MASK) && (tempMask & TOUCH_YELLOW_MASK));
	uint8_t oRising = (!(State & TOUCH_ORANGE_MASK) && (tempMask & TOUCH_ORANGE_MASK));

	State = tempMask;

	if (bRising || gRising || yRising || oRising)
	{
		OS_SemaphoreSignal(Semaphore);
	}

//	if ((State & 0x0F) != tempMask)
//	{
//		State = (State << 4) | (tempMask & 0xF);
//		OS_SemaphoreSignal(Semaphore);
//	}

	OS_ISRExit();
}
