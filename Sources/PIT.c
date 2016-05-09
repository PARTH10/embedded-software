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

BOOL PIT_Init(const uint32_t moduleClk, void (*userFunction)(void*), void* userArguments)
{

}

void PIT_Set(const uint32_t period, const BOOL restart)
{

}

void PIT_Enable(const BOOL enable)
{

}

void __attribute__ ((interrupt)) PIT_ISR(void)
{

}
