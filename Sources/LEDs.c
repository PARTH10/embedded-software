/*
 * LEDs.c
 *
 *  Created on: 6 Apr 2016
 *      Author: 11848759
 */
#include "LEDs.h"

#include "MK70F12.h"

/*!
 * @brief Initalises the LEDS, sets their wires as OP
 *
 */
BOOL LEDs_Init(void)
{
  //TODO: comment here
  PORTA_PCR10 |= PORT_PCR_MUX(1); //initalises led
  PORTA_PCR11 |= PORT_PCR_MUX(1);
  PORTA_PCR28 |= PORT_PCR_MUX(1);
  PORTA_PCR29 |= PORT_PCR_MUX(1);

  //Set LEDs as outputs
  GPIOA_PDDR |= LED_ORANGE_MASK;//(1<<11); //sets led line as an output
  GPIOA_PDDR |= LED_YELLOW_MASK;
  GPIOA_PDDR |= LED_GREEN_MASK;
  GPIOA_PDDR |= LED_BLUE_MASK;

  SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK;

  GPIOA_PSOR |= LED_ORANGE_MASK;
  GPIOA_PSOR |= LED_YELLOW_MASK;
  GPIOA_PSOR |= LED_GREEN_MASK;
  GPIOA_PSOR |= LED_BLUE_MASK;

  return bTRUE;
}

void LEDs_On(const TLED color)
{
  GPIOA_PCOR |= color;
}

void LEDs_Off(const TLED color)
{
  GPIOA_PSOR |= color;
}

void LEDs_Toggle(const TLED color)
{
  GPIOA_PTOR |= color;
}

