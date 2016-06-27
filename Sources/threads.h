/*
 * threads.h
 *
 *  Created on: 9 Jun 2016
 *      Author: 12011146
 */

#ifndef SOURCES_THREADS_H_
#define SOURCES_THREADS_H_

#define THREAD_STACK_SIZE 100

//helper macros
#define STATIC_STACK(name) static uint32_t name[THREAD_STACK_SIZE] __attribute__ ((aligned(0x08)))

#define CREATE_THREAD(fn,data,stack,prio) OS_ThreadCreate(fn,data,&stack[THREAD_STACK_SIZE - 1],prio)

//Thread priorities
#define TP_INITTHREAD     0
#define TP_UART_RECEIVE   1
#define TP_UART_TRANSMIT  2

#define TP_PACKETTHREAD   6
#define TP_RTCTHREAD      4

//use only by current mode
#define TP_MODETHREAD    8

#define TP_EVENTTHREAD    9
#define TP_MAINTHREAD    10

#endif /* SOURCES_THREADS_H_ */
