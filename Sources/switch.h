/*
 * switch.h
 *
 *  Created on: 23 Jun 2016
 *      Author: 12011146
 */

#ifndef SOURCES_SWITCH_H_
#define SOURCES_SWITCH_H_

void Switch_Init(void (*call1)(void*), void *arg1, void (*call2)(void*), void *arg2);
void __attribute__ ((interrupt)) PORTD_ISR(void);
void __attribute__ ((interrupt)) PORTE_ISR(void);

#endif /* SOURCES_SWITCH_H_ */
