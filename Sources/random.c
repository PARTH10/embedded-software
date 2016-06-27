/*
 * random.c
 *
 *  Created on: 27 Jun 2016
 *      Author: 12011146
 */

#include "random.h"

#include "MK70F12.h"

void Random_Init(void)
{
  SIM_SCGC3 |= SIM_SCGC3_RNGA_MASK;
  RNG_CR |= RNG_CR_GO_MASK | RNG_CR_INTM_MASK | RNG_CR_SLP_MASK;
  (void)Random_Generate();
}

uint32_t Random_Generate()
{
	RNG_CR &= ~RNG_CR_SLP_MASK;
	while ((RNG_SR & RNG_SR_OREG_LVL_MASK) == 0)
	  {
	  }
	uint32_t value = RNG_OR;
	RNG_CR |= RNG_CR_SLP_MASK;
  return value;
}
