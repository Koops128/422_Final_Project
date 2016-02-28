// /*
//  * ProducerConsumer.c
//  *
//  *  Programming Team:
//  * Sean Markus
//  * Wing-Sea Poon
//  * Abigail Smith
//  * Tabi Stein
//  *
//  * Placeholder file based off of the Outline
//  */
// #include <stdio.h>
// #include <stdlib.h>
// #include "ProducerConsumer.h"
// #include "Mutex.h"

// typedef struct {
// 	Mutex* mutex;
// 	PcbPtr Producer;
// 	PcbPtr Consumer;
// 	int isWaiting; //0 for false, 1 for true;
// 				   //at most one process in this pair will be waiting at any given time
// 	int bufavail;  //both can access
// 	unsigned int ProducerMutexLock[PRO_LOCK_UNLOCK];
// 	unsigned int ConsumerMutexLock[CON_LOCK_UNLOCK];
// 	unsigned int ProducerMutexUnlock[PRO_LOCK_UNLOCK];
// 	unsigned int ConsumerMutexUnlock[CON_LOCK_UNLOCK];
// 	unsigned int ProducerCondVarWait[PRO_WAIT];
// 	unsigned int ConsumerCondVarWait[CON_WAIT];
// 	unsigned int ProducerCondVarSignal[PRO_SIGNAL];
// 	unsigned int ConsumerCondVarSignal[CON_SIGNAL];
// } ProducerConsumer;

// ProConPtr ProducerConsumerConstructor(PcbPtr prod, PcbPtr cons,
// 									unsigned int proLock[], unsigned int conLock[],
// 									unsigned int proUnlock[], unsigned int conUnlock[],
// 									unsigned int proWait[], unsigned int conWait[],
// 									unsigned int proSig[], unsigned int conSig[]) {

// 	ProducerConsumer* procon = (ProducerConsumer*) malloc(sizeof(ProducerConsumer));
// 	procon->mutex = (Mutex*) malloc(sizeof(Mutex));
// 	procon->Producer = prod;
// 	procon->Consumer = cons;
// 	procon->ProducerMutexLock = proLock;
// 	procon->ConsumerMutexLock = conLock;
// 	procon->ProducerMutexUnlock = proUnlock;
// 	procon->ConsumerMutexUnlock = conUnlock;
// 	procon->ProducerCondVarWait = proWait;
// 	procon->ConsumerCondVarWait = conWait;
// 	procon->ProducerCondVarSignal = proSig;
// 	procon->ConsumerCondVarSignal = conSig;
// 	procon->bufavail = MAX_SHARED_SIZE;
// 	procon->isWaiting = 0;

// 	return procon;
// }

// /* this is different than waiting for a mutex.
//  * This should unlock the mutex it’s holding… it should be holding one since a wait should only be inside a critical section,
//  * then return an int (1 or 0) saying whether this PCB needs to wait or not.
//  * If it returns a “yes” - 1, then the CPU should take it out of the running state, and put into blocked...
//  * but NOT put back into ready queue.  This PCB will get put back into ready queue when whatever signal
//  * it’s waiting on is set by the other PCB in this pair.*/

// int Wait(ProducerConsumer* procon, PcbPtr waiter) {
// 	if ((waiter == procon->Producer && procon->bufavail == 0)
// 		|| (waiter == procon->Consumer && procon->bufavail == MAX_SHARED_SIZE)) {

// 		procon->isWaiting = 1;
// 		MutexUnlock(procon->mutex, waiter);
// 		return 1;
// 	}
// 	return 0;
// }

// /* The way this will work, is this is doing the “producing” and “consuming” work while at the same time signaling.
//  * It will check if the passed in PCB is the producer, if so, then it will first check
//  * if bufavail is at it’s max AND if the WAITING variable is true.
//  * If so, then we know that the consumer is waiting, so we want to return the consumer process.
//  * So we set the return PcbPtr to consumer.  Then we decrement bufavail to do the “producing” work.
//  * In the opposite situation, if the passed in PCB is the consumer, and if bufavail is at 0 AND the WAITING is true,
//  * then we know the producer is waiting, so set the return PcbPtr to the producer.  Then increment the bufavail
//  * to do the “consuming” work. And then the CPU should know what to do with what is given back.
//  * If what is returned is a null pointer, then do nothing.
//  * Otherwise, we know we need to put whatever was returned back into the ready queue, because it’s no longer waiting.*/

// PcbPtr Signal(ProducerConsumer* procon, PcbPtr signaler) {
// 	PcbPtr toReturn = NULL;
// 	if (signaler == procon->Producer) {
// 		if (procon->bufavail == MAX_SHARED_SIZE && procon->isWaiting) {
// 			procon->isWaiting = 0;
// 			toReturn = procon->Consumer;
// 		}
// 		procon->bufavail --;
// 	} else {
// 		if (procon->bufavail == 0 && procon->isWaiting) {
// 			procon->isWaiting = 0;
// 			toReturn = procon->Producer;
// 		}
// 		procon->bufavail ++;
// 	}
// 	return toReturn;
// }
