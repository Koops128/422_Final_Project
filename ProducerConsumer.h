/*
 * ProducerConsumer.h
 *
 * Programming Team:
 * Sean Markus
 * Wing-Sea Poon
 * Abigail Smith
 * Tabi Stein
 *
 */

#ifndef PRODUCERCONSUMER_H_
#define PRODUCERCONSUMER_H_

#include "Pcb.h"

#define PRO_LOCK_UNLOCK 1
#define CON_LOCK_UNLOCK 1
#define PRO_WAIT 1
#define CON_WAIT 1
#define PRO_SIGNAL 1
#define CON_SIGNAL 1
#define MAX_SHARED_SIZE 5

typedef struct ProducerConsumer* ProConPtr;

ProConPtr ProducerConsumerConstructor(PcbPtr prod, PcbPtr cons,
		unsigned int proLock[], unsigned int conLock[],
		unsigned int proUnlock[], unsigned int conUnlock[],
		unsigned int proWait[], unsigned int conWait[],
		unsigned int proSig[], unsigned int conSig[]);

int Wait(ProConPtr procon, PcbPtr waiter);

PcbPtr Signal(ProConPtr procon, PcbPtr signaler);


#endif /* PRODUCERCONSUMER_H_ */
