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
#define TP_INITTHREAD 0
#define TP_MAINTHREAD 4




#endif /* SOURCES_THREADS_H_ */
