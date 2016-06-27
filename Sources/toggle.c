/*
 * toggle.c
 *
 *  Created on: 26 Jun 2016
 *      Author: 12011146
 */

#include "OS.h"
#include "LEDs.h"
#include "touch.h"
#include "types.h"
#include "threads.h"

static void (*Callback)(void *);

/*static void SetLeds(uint8_t values)
{
	if (!(values & TOUCH_ORANGE_MASK) && (values & DELTA_ORANGE_MASK))
	{
		LEDs_Toggle(LED_ORANGE);
	}

	if (!(values & TOUCH_YELLOW_MASK) && (values & DELTA_YELLOW_MASK))
	{
		LEDs_Toggle(LED_YELLOW);
	}

	if (!(values & TOUCH_GREEN_MASK) && (values & DELTA_GREEN_MASK))
	{
		LEDs_Toggle(LED_GREEN);
	}

	if (!(values & TOUCH_BLUE_MASK) && (values & DELTA_BLUE_MASK))
	{
		LEDs_Toggle(LED_BLUE);
	}
}*/

static void SetLeds(uint8_t values)
{
	if (values & TOUCH_ORANGE_MASK)
	{
		LEDs_Toggle(LED_ORANGE);
	}

	if (values & TOUCH_YELLOW_MASK)
	{
		LEDs_Toggle(LED_YELLOW);
	}

	if (values & TOUCH_GREEN_MASK)
	{
		LEDs_Toggle(LED_GREEN);
	}

	if (values & TOUCH_BLUE_MASK)
	{
		LEDs_Toggle(LED_BLUE);
	}
}

void Toggle_Init(void (*Finished)(void *))
{
	Callback = Finished;
}

/*!
 * @brief Stack for the rtc thread.
 */
STATIC_STACK(ToggleThreadStack);

/*!
 * @brief Thread handling the RTC
 */
void ToggleThread(void *data)
{
	LEDs_Off(LED_BLUE);
	LEDs_Off(LED_GREEN);
	LEDs_Off(LED_YELLOW);
	LEDs_Off(LED_ORANGE);
	Touch_EnableScan(bTRUE);
	uint8_t values = 0;
	for (;;)
	{
		//At least 5s, so 501 * 10ms > 5s
		if (Touch_Wait(501, &values) == OS_TIMEOUT)
		{
			break;
		}
		SetLeds(values);
	}
	LEDs_Off(LED_BLUE);
	LEDs_Off(LED_GREEN);
	LEDs_Off(LED_YELLOW);
	LEDs_Off(LED_ORANGE);
	Touch_EnableScan(bFALSE);
	(*Callback)((void *)0);
	OS_ERROR error = OS_ThreadDelete(OS_PRIORITY_SELF);
}

void Toggle_AssumeControl()
{
	OS_ERROR error = CREATE_THREAD(ToggleThread, NULL, ToggleThreadStack, TP_MODETHREAD);
}

