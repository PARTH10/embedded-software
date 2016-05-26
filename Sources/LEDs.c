/*! @file
 *
 *  @brief Implementations of functions which control the LEDs.
 *
 *  @author Robin Wohlers-Reichel, Joshua Gonsalves
 *  @date 2016-03-23
 */
/*!
**  @addtogroup leds_module LEDs module documentation
**  @{
*/
#include "LEDs.h"

#include "MK70F12.h"

BOOL LEDs_Init(void)
{
  PORTA_PCR10 |= PORT_PCR_MUX(1); //Initializes led
  PORTA_PCR11 |= PORT_PCR_MUX(1);
  PORTA_PCR28 |= PORT_PCR_MUX(1);
  PORTA_PCR29 |= PORT_PCR_MUX(1);

  //Set LEDs as outputs
  GPIOA_PDDR |= LED_ORANGE_MASK;//(1<<11); //sets led line as an output
  GPIOA_PDDR |= LED_YELLOW_MASK;
  GPIOA_PDDR |= LED_GREEN_MASK;
  GPIOA_PDDR |= LED_BLUE_MASK;

  //Turn port A on
  SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK;

  //Turn off all LEDs
  GPIOA_PSOR |= LED_ORANGE_MASK;//THIS LINE CHANGES PORTE_PCR18
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

/*!
** @}
*/
