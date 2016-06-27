/*
 * game.c
 *
 *  Created on: 26 Jun 2016
 *      Author: 12011146
 */

#include "game.h"

#include "LEDs.h"
#include "OS.h"
#include "touch.h"
#include "threads.h"
#include "types.h"

#define SIMON_SEQUENCE_LENGTH 32

static uint8_t SimonSequence[SIMON_SEQUENCE_LENGTH];

static void (*Callback)(void *);

uint8_t power(uint8_t base, uint8_t exp)
{
	uint8_t result = 1;
	while (exp)
	{
		result *= base;
		exp--;
	}
	return result;
}

static uint8_t GetTouchOrZero(uint8_t mask)
{
	mask &= 0xF;
	if (mask == TOUCH_BLUE_MASK)
	{
		return mask;
	}
	if (mask == TOUCH_GREEN_MASK)
	{
		return mask;
	}
	if (mask == TOUCH_YELLOW_MASK)
	{
		return mask;
	}
	if (mask == TOUCH_ORANGE_MASK)
	{
		return mask;
	}
	return 0;
}

static void PopulateSequence()
{
	for (int i = 0; i < SIMON_SEQUENCE_LENGTH; i++)
	{
		SimonSequence[i] = power(2, (i % 4));
	}
}

static void SetLeds(uint8_t values)
{
	LEDs_Off(LED_BLUE);
	LEDs_Off(LED_GREEN);
	LEDs_Off(LED_YELLOW);
	LEDs_Off(LED_ORANGE);
	if (values & TOUCH_ORANGE_MASK)
	{
		LEDs_On(LED_ORANGE);
	}

	if (values & TOUCH_YELLOW_MASK)
	{
		LEDs_On(LED_YELLOW);
	}

	if (values & TOUCH_GREEN_MASK)
	{
		LEDs_On(LED_GREEN);
	}

	if (values & TOUCH_BLUE_MASK)
	{
		LEDs_On(LED_BLUE);
	}
}

static void SaveHighscore(uint8_t highscore)
{

}

static void EndGame(uint8_t currentPosition)
{
	SaveHighscore(currentPosition);
	SetLeds(0xF);
	OS_TimeDelay(26);
	SetLeds(0x0);
	OS_TimeDelay(26);
	SetLeds(0xF);
	OS_TimeDelay(26);
	SetLeds(0x0);
	OS_TimeDelay(26);
	SetLeds(0x0);
}

static void PlayGame()
{
	for (uint8_t MaxPosition = 0; MaxPosition < SIMON_SEQUENCE_LENGTH; MaxPosition++)
	{
		SetLeds(0);
		//Play the sequence
		for (uint8_t CurrentPosition = 0; CurrentPosition <= MaxPosition; CurrentPosition++)
		{
			SetLeds(SimonSequence[CurrentPosition]);
			OS_TimeDelay(101);
		}

		SetLeds(0);

		//Listen for the response
		for (uint8_t CurrentPosition = 0; CurrentPosition <= MaxPosition; CurrentPosition++)
		{
			uint8_t values = 0;
			if (Touch_Wait(201, &values) == OS_TIMEOUT)
			{
				//Time out
				EndGame(CurrentPosition);
				return;
			}
//			values = GetTouchOrZero(values);
			SetLeds(values);
			if (values != SimonSequence[CurrentPosition])
			{
				EndGame(CurrentPosition);
				return;
			}
		}
	}
}

STATIC_STACK(GameStack);

void GameThread(void *data)
{
	PopulateSequence();
	SetLeds(0);
	Touch_EnableScan(bTRUE);
	PlayGame();
	Touch_EnableScan(bFALSE);
	(*Callback)((void *)0);
	OS_ThreadDelete(OS_PRIORITY_SELF);
}

void Game_Init(void (*Finished)(void *))
{
	Callback = Finished;
}

void Game_AssumeControl()
{
	OS_ERROR error = CREATE_THREAD(GameThread, NULL, GameStack, TP_MODETHREAD);
}
