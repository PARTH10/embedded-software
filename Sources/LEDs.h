/*! @file
 *
 *  @brief Routines to access the LEDs on the TWR-K70F120M.
 *
 *  This contains the functions for operating the LEDs.
 *
 *  @author PMcL
 *  @date 2015-08-15
 */
/*!
**  @addtogroup cmd_module CMD module documentation
**  @{
*/
#ifndef LEDS_H
#define LEDS_H

// new types
#include "types.h"

#define LED_ORANGE_MASK 0x800
#define LED_ORANGE_SHIFT 11
#define LED_YELLOW_MASK 0x10000000
#define LED_YELLOW_SHIFT 28
#define LED_GREEN_MASK 0x20000000
#define LED_GREEN_SHIFT 29
#define LED_BLUE_MASK 0x400
#define LED_BLUE_SHIFT 10

/*! @brief LED to pin mapping on the TWR-K70F120M
 *
 */
typedef enum
{
  LED_ORANGE = LED_ORANGE_MASK,
  LED_YELLOW = LED_YELLOW_MASK,
  LED_GREEN = LED_GREEN_MASK,
  LED_BLUE = LED_BLUE_MASK
} TLED;

/*! @brief Sets up the LEDs before first use.
 *
 *  @return BOOL - TRUE if the LEDs were successfully initialized.
 */
BOOL LEDs_Init(void);
 
/*! @brief Turns an LED on.
 *
 *  @param color The color of the LED to turn on.
 *  @note Assumes that LEDs_Init has been called.
 */
void LEDs_On(const TLED color);
 
/*! @brief Turns off an LED.
 *
 *  @param color THe color of the LED to turn off.
 *  @note Assumes that LEDs_Init has been called.
 */
void LEDs_Off(const TLED color);

/*! @brief Toggles an LED.
 *
 *  @param color THe color of the LED to toggle.
 *  @note Assumes that LEDs_Init has been called.
 */
void LEDs_Toggle(const TLED color);

/*!
** @}
*/

#endif
