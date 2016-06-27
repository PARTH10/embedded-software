/*
 * touch.h
 *
 *  Created on: 23 Jun 2016
 *      Author: 12011146
 */

#ifndef SOURCES_TOUCH_H_
#define SOURCES_TOUCH_H_

#define TOUCH_BLUE_MASK    0x01
#define TOUCH_GREEN_MASK   0x02
#define TOUCH_YELLOW_MASK  0x04
#define TOUCH_ORANGE_MASK  0x08

#define TOUCH_BLUE_SHIFT   0
#define TOUCH_GREEN_SHIFT  1
#define TOUCH_YELLOW_SHIFT 2
#define TOUCH_ORANGE_SHIFT 3

#include "OS.h"
#include "types.h"

typedef enum
{
  // Touch Down event
	TOUCH_DOWN = 0x1,
  // Touch Up event
  TOUCH_UP = 0x2
} TOUCH_EVENT;

/*!
 * @param callback Called when the value of Touch_State changes.
 * @param argument Argument passed to the callback function.
 * @note The callback runs in an ISR context.
 */
void Touch_Init();
OS_ERROR Touch_Wait(uint16_t timeout, uint8_t *values);
void Touch_EnableScan(BOOL enable);
uint8_t Touch_State();

void __attribute__ ((interrupt)) TSI_ISR(void);

#endif /* SOURCES_TOUCH_H_ */
