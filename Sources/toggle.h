/*
 * toggle.h
 *
 *  Created on: 26 Jun 2016
 *      Author: 12011146
 */

#ifndef SOURCES_TOGGLE_H_
#define SOURCES_TOGGLE_H_

#include "types.h"

void Toggle_Init(void (*Finished)(void *));
void Toggle_AssumeControl();

#endif /* SOURCES_TOGGLE_H_ */
