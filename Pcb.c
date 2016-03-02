/***********************************************************************************************
* Pcb.c
*
* Programming Team:
* Sean Markus
* Wing-Sea Poon
* Abigail Smith
* Tabi Stein
*
* Date: 1/22/16
*
* Description:
* This C file implements the class and methods for the process control block.
*
************************************************************************************************/

#include "Pcb.h"
#include "Mutex.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

const char* stateNames[] = {"Created","Running","Ready","Interrupted","Blocked","Terminated"};

typedef struct PCB {
	int PID;
	int priority;
	ProConPtr *myPC;
	//MR myMR; TODO
	int starveFlag;
	State state;
	unsigned int PC;
	unsigned int maxPC;
	unsigned long int creation;
	unsigned long int termination;
	unsigned int terminate;
	unsigned int term_count;
	unsigned int IO_1_Traps[NUM_IO_TRAPS];
	unsigned int IO_2_Traps[NUM_IO_TRAPS];
} PcbStr;

PcbPtr ProducerConsumerPCBConstructor(ProConPtr procon){
	PcbStr* pcb = PCBConstructor();
	pcb->myPC = procon;
	//pcb->myMR = NULL;
	return pcb;
}


PcbPtr ProducerPCBConstructor(ProConPtr procon) {
	PcbStr* pcb = PCBConstructor();
	//todo stuff
	return pcb;
}
PcbPtr ConsumerPCBConstructor(ProConPtr procon) {
	PcbStr* pcb = PCBConstructor();
	//todo stuff
	return pcb;
}


unsigned int PCBGetIO1Trap(PcbStr* pcb, int index) {
	if (index < NUM_IO_TRAPS) {
		return pcb->IO_1_Traps[index];
	} else {
		return -1;
	}
}

unsigned int PCBGetIO2Trap(PcbStr* pcb, int index) {
	if (index < NUM_IO_TRAPS) {
		return pcb->IO_2_Traps[index];
	} else {
		return -1;
	}
}

char* StateToString(State state) {
	int len = strlen(stateNames[state]);
	char* string = malloc(sizeof(char) * len + 1);
	sprintf(string, "%s", stateNames[state]); //auto appends null at end
	return string;
}

void PCBSetPriority(PcbStr* pcb, int priority) {
	pcb->priority = priority;

}

void PCBSetID(PcbStr* pcb, int id) {
	pcb->PID = id;
}

void PCBSetState(PcbStr* pcb, State newState) {
	pcb->state = newState;
}

/**
 * Sets the PC for this PCB.
 */
void PCBSetPC(PcbStr* pcb, unsigned int newPC) {
	pcb->PC = newPC;
}

void PCBSetTermination(PcbStr* pcb, unsigned long newTermination) {
	pcb->termination = newTermination;
}

void PCBSetTerminate(PcbStr* pcb, int newTerminate) {
	pcb->terminate = newTerminate;
}

void PCBSetTermCount(PcbStr* pcb, unsigned int newTermCount) {
	pcb->term_count = newTermCount;
}

/**
 * Returns PC of this PCB.
 */
unsigned int PCBGetPC(PcbStr* pcb) {
	return pcb->PC;
}

int PCBGetPriority(PcbStr* pcb) {
	return pcb->priority;
}

int PCBGetID(PcbStr* pcb) {
	return pcb->PID;
}

State PCBGetState(PcbStr* pcb) {
	return pcb->state;
}

unsigned int PCBGetMaxPC(PcbStr* pcb) {
	return pcb->maxPC;
}

unsigned long PCBGetCreation(PcbStr* pcb) {
	return pcb->creation;
}

unsigned long PCBGetTermination(PcbStr* pcb) {
	return pcb->termination;
}

int PCBGetTerminate(PcbStr* pcb) {
	return pcb->terminate;
}

unsigned int PCBGetTermCount(PcbStr* pcb) {
	return pcb->term_count;
}

/*
 * Partitions (maxVal - minVal) into n non-overlapping partitions.
 * Sets storage[i] to a random number from the corresponding partition.
 * 
 * Ex.: n = 8, minVal = 0, maxVal = 2000
 * Partition Size = 250
 * partition[0] = 0 to 249
 * partition[1] = 250 to 499
 * ...
 * partition[7] = 1750 to 1999
 */
void genTraps(int n, unsigned int* storage, int minVal, int maxVal) {
	int partitionSize = (maxVal - minVal) / n;	// truncate if the division results in a double
	int i;
	for(i = 0; i < n; i++) {
		storage[i] = (rand() % (partitionSize)) + (i * partitionSize);
	}
}

PcbPtr PCBConstructor(){
	PcbStr* pcb = (PcbStr*) malloc(sizeof(PcbStr));
	pcb->PC = 0;
	pcb->PID = 1;
	pcb->priority = 1;
	pcb->state = created;
	pcb->creation = time(NULL);
	pcb->maxPC = 2000;
	pcb->terminate = rand()%10;	//ranges from 0-10
	pcb->term_count = 0;

	//genIOArrays(pcb);

	unsigned int* allTraps = malloc(sizeof(unsigned int) * NUM_IO_TRAPS * 2);
	genTraps(NUM_IO_TRAPS * 2, allTraps, 0, pcb->maxPC);

	int i;
	for (i = 0; i < NUM_IO_TRAPS; i++) {
		pcb->IO_1_Traps[i] = allTraps[i*2];		//grab even indices
		pcb->IO_2_Traps[i] = allTraps[(i*2) + 1]; //grab odd indices
	}

	free(allTraps);

	return pcb;
}


/**Need at most 5 chars for each 8 traps, plus 8 spaces before each, or 48*/
char* TrapsToString(PcbStr* pcb) {
	char* arrStr = (char*) malloc(sizeof(char) * (48 + 10 + 1));
	arrStr[0] = '\0';

	char dev1[sizeof(char) * (20 + 4 + 1)];
	dev1[0] = '\0';
	char dev2[sizeof(char) * (20 + 4 + 1)];
	dev2[0] = '\0';

	int i;
	for (i = 0; i < NUM_IO_TRAPS; i++) {
		char buffer[sizeof(char) * (5+2)]; //5 is max number of digits in unsigned int, + null + space
		sprintf(buffer, " %d", pcb->IO_1_Traps[i]);
		strncat(dev1,buffer,7);
		char buffer2[sizeof(char) * (5+2)];
		sprintf(buffer2, " %d", pcb->IO_2_Traps[i]);
		strncat(dev2,buffer2,7);
	}
	sprintf(arrStr, "d1[%s ] d2[%s ]", dev1, dev2);
	return arrStr;
}


char *PCBToString(PcbStr* pcb) {
	if (pcb == NULL)
		return NULL;

	char * emptyStr = (char*) malloc(sizeof(char) * 1000);
	emptyStr[199] = '\0';
	char* stateString = StateToString(pcb->state);
	char* trapString = TrapsToString(pcb);
	int lenNeeded = sprintf(emptyStr, "ID: %d, Priority: %d, State: %s, PC: %d"
			", MAX_PC %d"
			", CREATION %lu"
			", TERMINATE %d"
			", TERM_COUNT %d"
			", \n	Traps: (%s)"
							,pcb->PID, pcb->priority, stateString, pcb->PC
							,pcb->maxPC
							,pcb->creation
							,pcb->terminate
							, pcb->term_count
							, trapString
							);
	free(stateString);
	free(trapString);
	char * retString = (char *) malloc(sizeof(char) * lenNeeded);
	sprintf(retString, "%s", emptyStr);
	free(emptyStr);
	return retString;
}

void PCBDestructor(PcbPtr pcb) {
	free (pcb);
	pcb = NULL;	//Only locally sets the pointer to null
}

//ADDING PRODUCER CONSUMER OBJECT AND FUNCTIONS HERE
typedef struct ProducerConsumer {
	Mutex* mutex;
	PcbPtr Producer;
	PcbPtr Consumer;
	int isWaiting; //0 for false, 1 for true;
				   //at most one process in this pair will be waiting at any given time
	int bufavail;  //both can access
	unsigned int* ProducerMutexLock;//[PRO_LOCK_UNLOCK];
	unsigned int* ConsumerMutexLock;//[CON_LOCK_UNLOCK];
	unsigned int* ProducerMutexUnlock;//[PRO_LOCK_UNLOCK];
	unsigned int* ConsumerMutexUnlock;//[CON_LOCK_UNLOCK];
	unsigned int* ProducerCondVarWait;//[PRO_WAIT];
	unsigned int* ConsumerCondVarWait;//[CON_WAIT];
	unsigned int* ProducerCondVarSignal;//[PRO_SIGNAL];
	unsigned int* ConsumerCondVarSignal;//[CON_SIGNAL];
} ProducerConsumerStr;

ProConPtr ProducerConsumerConstructor(PcbPtr prod, PcbPtr cons,
									unsigned int proLock[], unsigned int conLock[],
									unsigned int proUnlock[], unsigned int conUnlock[],
									unsigned int proWait[], unsigned int conWait[],
									unsigned int proSig[], unsigned int conSig[]) {

	ProducerConsumerStr* procon = (ProducerConsumerStr*) malloc(sizeof(ProducerConsumerStr));
	procon->mutex = (Mutex*) malloc(sizeof(Mutex));
	procon->Producer = prod;
	procon->Consumer = cons;
	procon->ProducerMutexLock = proLock;
	procon->ConsumerMutexLock = conLock;
	procon->ProducerMutexUnlock = proUnlock;
	procon->ConsumerMutexUnlock = conUnlock;
	procon->ProducerCondVarWait = proWait;
	procon->ConsumerCondVarWait = conWait;
	procon->ProducerCondVarSignal = proSig;
	procon->ConsumerCondVarSignal = conSig;
	procon->bufavail = MAX_SHARED_SIZE;
	procon->isWaiting = 0;

	return procon;
}

/* this is different than waiting for a mutex.
 * This should unlock the mutex it’s holding… it should be holding one since a wait should only be inside a critical section,
 * then return an int (1 or 0) saying whether this PCB needs to wait or not.
 * If it returns a “yes” - 1, then the CPU should take it out of the running state, and put into blocked...
 * but NOT put back into ready queue.  This PCB will get put back into ready queue when whatever signal
 * it’s waiting on is set by the other PCB in this pair.*/

int Wait(ProducerConsumerStr* procon, PcbPtr waiter) {
	if ((waiter == procon->Producer && procon->bufavail == 0)
		|| (waiter == procon->Consumer && procon->bufavail == MAX_SHARED_SIZE)) {

		procon->isWaiting = 1;
		MutexUnlock(procon->mutex, waiter);
		return 1;
	}
	return 0;
}

/* The way this will work, is this is doing the “producing” and “consuming” work while at the same time signaling.
 * It will check if the passed in PCB is the producer, if so, then it will first check
 * if bufavail is at it’s max AND if the WAITING variable is true.
 * If so, then we know that the consumer is waiting, so we want to return the consumer process.
 * So we set the return PcbPtr to consumer.  Then we decrement bufavail to do the “producing” work.
 * In the opposite situation, if the passed in PCB is the consumer, and if bufavail is at 0 AND the WAITING is true,
 * then we know the producer is waiting, so set the return PcbPtr to the producer.  Then increment the bufavail
 * to do the “consuming” work. And then the CPU should know what to do with what is given back.
 * If what is returned is a null pointer, then do nothing.
 * Otherwise, we know we need to put whatever was returned back into the ready queue, because it’s no longer waiting.*/

PcbPtr Signal(ProducerConsumerStr* procon, PcbPtr signaler) {
	PcbPtr toReturn = NULL;
	if (signaler == procon->Producer) {
		if (procon->bufavail == MAX_SHARED_SIZE && procon->isWaiting) {
			procon->isWaiting = 0;
			toReturn = procon->Consumer;
		}
		procon->bufavail --;
	} else {
		if (procon->bufavail == 0 && procon->isWaiting) {
			procon->isWaiting = 0;
			toReturn = procon->Producer;
		}
		procon->bufavail ++;
	}
	return toReturn;
}


////test pcb
//int main(void) {
//	srand(time(NULL));
//	PcbStr* pcb = PCBConstructor(0);
//	printf("%s", PCBToString(pcb));
//	return 0;
//}
