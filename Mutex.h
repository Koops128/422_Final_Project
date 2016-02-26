/*
 * Mutex.h
 *
 *  Created on: Feb 19, 2016
 *      Author: Abigail
 */

#ifndef MUTEX_H_
#define MUTEX_H_
#include "Pcb.h"
#include "Fifo.h"

typedef struct {
	PcbPtr owner;
	FifoQueue * waitQ;
}Mutex;

Mutex* MutexConstructor();

void MutexDestructor(Mutex* mutex);

void MutexLock(Mutex* mutex, PcbPtr pcb);

void MutexUnlock(Mutex* mutex, PcbPtr pcb);

#endif /* MUTEX_H_ */
