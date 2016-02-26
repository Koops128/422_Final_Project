/*
 * Mutex.c
 *
 *  Created on: Feb 19, 2016
 *      Author: Abigail
 */

#include <stdio.h>
#include "Mutex.h"
#include "Pcb.h"
#include "Fifo.h"

Mutex* MutexConstructor() {
	Mutex* mutex = (Mutex*) malloc(sizeof(Mutex));
	mutex->waitQ = fifoQueueConstructor();

	return mutex;
}

void MutexDestructor(Mutex* mutex) {
	fifoQueueDestructor(&mutex->waitQ);
	PCBDestructor(mutex->owner);

	free(mutex);
	mutex = NULL;	//locally
}

void MutexLock(Mutex* mutex, PcbPtr pcb) {
	//TODO
}

void MutexUnlock(Mutex* mutex, PcbPtr pcb) {
	//TODO
}
