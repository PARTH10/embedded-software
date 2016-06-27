/*
 * switch.c
 *
 *  Created on: 23 Jun 2016
 *      Author: 12011146
 */

#include "switch.h"

#include "MK70F12.h"

#include "OS.h"

static void (*S1Callback)(void *);
static void *S1Argument;
static void (*S2Callback)(void *);
static void *S2Argument;

void Switch_Init(void (*call1)(void*), void *arg1, void (*call2)(void*), void *arg2)
{
	S1Callback = call1;
	S1Argument = arg1;

	S2Callback = call2;
	S2Argument = arg2;

	SIM_SCGC5 |= SIM_SCGC5_PORTD_MASK | SIM_SCGC5_PORTE_MASK;
	PORTD_PCR0 |= PORT_PCR_MUX(1) | PORT_PCR_IRQC(10) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
	PORTE_PCR26 |= PORT_PCR_MUX(1) | PORT_PCR_IRQC(10) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;

	NVICISER2 = (1 << 26);
	NVICICPR2 = (1 << 26);
}

//Switch 1
void __attribute__ ((interrupt)) PORTD_ISR(void)
{
	OS_ISREnter();
	if (!(PORTD_PCR0 & PORT_PCR_ISF_MASK))
	{
		OS_ISRExit();
		return;
	}
	PORTD_PCR0 |= PORT_PCR_ISF_MASK;
	if (S1Callback)
	{
		(*S1Callback)(S1Argument);
	}
	OS_ISRExit();
}

//Switch 2
void __attribute__ ((interrupt)) PORTE_ISR(void)
{
	OS_ISREnter();
	if (!(PORTE_PCR26 & PORT_PCR_ISF_MASK))
	{
		OS_ISRExit();
		return;
	}
	PORTE_PCR26 |= PORT_PCR_ISF_MASK;
	if (S2Callback)
	{
		(*S2Callback)(S2Argument);
	}
	OS_ISRExit();
}

