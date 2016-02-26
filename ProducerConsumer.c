/*
 * ProducerConsumer.c
 *
 *  Programming Team:
 * Sean Markus
 * Wing-Sea Poon
 * Abigail Smith
 * Tabi Stein
 *
 * Placeholder file based off of the Outline
 */

#include "ProducerConsumer.h"
#include "Pcb.h"
#include "Mutex.h"

typedef struct ProducerConsumer {
	Mutex mutex;
	unsigned int ProducerMutexLock[1];
	unsigned int ConsumerMutexLock[1];
	unsigned int ProducerMutexUnlock[1];
	unsigned int ConsumerMutexUnlock[1];
	unsigned int ProducerCondVarWait[1];
	unsigned int ConsumerCondVarWait[1];
	unsigned int ProducerCondVarSignal[1];
	unsigned int ConsumerCondVarSignal[1];
	PcbPtr Producer;
	PcbPtr Consumer;
	int sharedData; //both can access
} ProCon;
