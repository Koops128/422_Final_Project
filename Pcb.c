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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>


const char* stateNames[] = {"Created","Running","Ready","Interrupted","Blocked","Terminated"};

typedef struct PCB {
	int PID;
	int priority;
	State state;
	unsigned int PC;
	unsigned int maxPC;
	unsigned long int creation;
	unsigned long int termination;
	unsigned int terminate;
	unsigned int term_count;
	unsigned int IO_1_Traps[NUM_IO_TRAPS];
	unsigned int IO_2_Traps[NUM_IO_TRAPS];

	RelationshipStr mRelation; //Contains step numbers data

	union { //Contains mutex and/or condition variable data
		MRDataPtr MutRecData;
		PCDataPtr ProConData;
	};

} PcbStr;

/*********************************************************************************/
/*                          	 I/O Related			                         */
/*********************************************************************************/

/********************
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


/*Creates traps specifically outside of lock/unlock bounds.
 *Stole Abby's code.*/
void genMutrecTraps(PcbStr* pcb, unsigned int storage[NUM_IO_TRAPS * 2]) {
	int minVal = 0;
	int maxVal = pcb->maxPC;
	int partitionSize = (maxVal - minVal) / (NUM_IO_TRAPS * 2);
	int i, j;
	for(i = 0; i < (NUM_IO_TRAPS * 2); i++) {
		int randNum;
		int isOk;
		do {
			randNum = (rand() % (partitionSize)) + (i * partitionSize);

			isOk = 1;
			j = 0;
			for (j = 0; j < NUM_MUTEX_STEPS; j++) {
				MRStepsPtr theStepStr = pcb->mRelation.mrSteps; //struct that holds all step arrays
				if ( (randNum >= ((theStepStr)->lock1)[j])  &&	(randNum <= ((theStepStr)->unlock1)[j]) &&
					 (randNum >= ((theStepStr)->lock2)[j])  &&	(randNum <= ((theStepStr)->unlock2)[j]) ) {
					isOk = 0;
				}
			}
		} while(!isOk);
		storage[i] = randNum;
	}
}


/*If the PCB is a mutual resource user, please call this after initializing the lock and unlock step arrays.*/
void initializeTrapArray(PcbStr* pcb) {
	unsigned int* allTraps = malloc(sizeof(unsigned int) * NUM_IO_TRAPS * 2);

	/****Create the traps in a way specific to the pairType****/
	switch (pcb->mRelation.mType) {
	case none:
		genTraps(NUM_IO_TRAPS * 2, allTraps, 0, pcb->maxPC);
		break;
	case mutrecA:
		genMutrecTraps(pcb, allTraps);
		break;
	case mutrecB:
		genMutrecTraps(pcb, allTraps);
		break;
	//TODO prodCon case
	default :
		break;
	}

	/**Load up the trap array with results.**/
	int i;
	for (i = 0; i < NUM_IO_TRAPS; i++) {
		pcb->IO_1_Traps[i] = allTraps[i*2];			//grab even indices
		pcb->IO_2_Traps[i] = allTraps[(i*2) + 1]; 	//grab odd indices
	}
	free(allTraps);
}

/*********************************************************************************/
/*                       Constructor/Destructor			                         */
/*********************************************************************************/

PcbPtr PCBAllocateSpace() {
	return (PcbStr*) malloc(sizeof(PcbStr));
}

/*Automatically initializes IO trap array if the pcb is not a synchronization pair.
 *Otherwise, initializeTrapArray must be manually called.
 *Expects a preallocated PCB.
 *Returns a reference to the pcb sent in incase convenient.*/
PcbPtr PCBConstructor(PcbPtr pcb, RelationshipType theType, PcbPtr partner) {
	//general
	pcb->PC = 0;
	pcb->PID = 1;
	pcb->priority = 1;
	pcb->state = created;
	pcb->creation = time(NULL);
	pcb->maxPC = 2000;
	pcb->terminate = rand()%10;	//ranges from 0-10
	pcb->term_count = 0;

	//relationship
	pcb->mRelation.mType = theType;
	pcb->mRelation.mPartner = partner;

	//allocate appropriate memory
	if (theType == mutrecA || theType == mutrecB) {
		pcb->MutRecData = malloc(sizeof(MRDataPtr));
		pcb->mRelation.mrSteps = malloc(sizeof(MutRecStepsStr));
	}

	if (theType != mutrecA && theType != mutrecB) {
		initializeTrapArray(pcb);
	}

	return pcb;
}

void PCBDestructor(PcbPtr pcb) {
	free (pcb);
	pcb = NULL;	//Only locally sets the pointer to null
}

/*********************************************************************************/
/*                          	 Mutex Related			                         */
/*********************************************************************************/

/*The mutex<mutexNum>Index of this pcb will be set to index.
 *(Serves as an index into an array of mutexes outside of this file).*/
void PCBSetMutexIndex(PcbStr* pcb, int mutexNum, int index) {
	if (mutexNum == 1) {
		pcb->MutRecData->mutex1 = index;
	} else {
		pcb->MutRecData->mutex2 = index;
	}
}

int PCBGetMutexIndex(PcbStr* pcb, int mutexNum) {
	if (mutexNum == 1) {
		return pcb->MutRecData->mutex1;
	} else {
		return pcb->MutRecData->mutex2;
	}
}


void PCBSetMutexLockSteps(PcbStr* pcb, int mutexNum, unsigned int theSteps[NUM_MUTEX_STEPS]) {
	if (pcb) {
		MRStepsPtr theStepStr = pcb->mRelation.mrSteps;
		int i;
		unsigned int* theArr = mutexNum == 1? theStepStr->lock1 : theStepStr->lock2;
		for (i = 0; i < NUM_MUTEX_STEPS; i++) {
			theArr[i] = theSteps[i];
		}
	}
}

void PCBSetMutexUnlockSteps(PcbStr* pcb, int mutexNum, unsigned int theSteps[NUM_MUTEX_STEPS]) {
	if (pcb) {
		MRStepsPtr theStepStr = pcb->mRelation.mrSteps;
		int i;
		unsigned int* theArr = mutexNum == 1? theStepStr->unlock1 : theStepStr->unlock2;
		for (i = 0; i < NUM_MUTEX_STEPS; i++) {
			theArr[i] = theSteps[i];
		}
	}
}

/*
 *Returns 1 if the given int is a step where this pcb requests a lock on the specified mutex.
 *mutexNum: which mutex's steps to look at (1 or 2)
 *theStep: the number to check if it's a step.
 */
int isMutexLockStep(PcbStr* pcb, int mutexNum, unsigned int theStep) {
	int i;
	int requestMade = 0;
	if (pcb) {
		if (mutexNum == 1) { //look through array 1
			unsigned int* steps = (pcb->mRelation).mrSteps->lock1;
			for (i=0; i < NUM_MUTEX_STEPS; i++) {
				requestMade = steps[i] == theStep ? 1: requestMade;
			}
		} else { //look through array 2
			unsigned int* steps = pcb->mRelation.mrSteps->lock2;
			for (i=0; i < NUM_IO_TRAPS; i++) {
				requestMade = steps[i] == theStep ? 1: requestMade;
			}
		}
	}
	return requestMade;
}

/*mutexNum : 1 or 2
 *theStep : the number to check whether it's an unlock step
 */
int isMutexUnlockStep(PcbStr* pcb, int mutexNum, unsigned int theStep) {
	int i;
	int requestMade = 0;
	if (pcb) {
		if (mutexNum == 1) { //look through array 1
			unsigned int* steps = pcb->mRelation.mrSteps->unlock1;
			for (i=0; i < NUM_MUTEX_STEPS; i++) {
				requestMade = steps[i] == theStep ? 1: requestMade;
			}
		} else { //look through array 2
			unsigned int* steps = pcb->mRelation.mrSteps->unlock2;
			for (i=0; i < NUM_IO_TRAPS; i++) {
				requestMade = steps[i] == theStep ? 1: requestMade;
			}
		}
	}
	return requestMade;
}
//
///*Returns true if the PCB is in its step where it prints.*/
//int isInCriticalSection(PcbStr* pcb) {
//	if (pcb) {
//
//	} else {
//		return 0;
//	}
//}

/*********************************************************************************/
/*                         Relationship Related			                         */
/*********************************************************************************/
RelationshipType PCBgetPairType(PcbStr* pcb) {
	if (pcb) {
		return pcb->mRelation.mType;
	} else {
		return none; //Just the most generic thing I can think of returning if pcb is null.
	}
}

/*********************************************************************************/
/*                          	 I/O Related			                         */
/*********************************************************************************/

//TODO merge into one method with a choice.
unsigned int PCBGetIO1Trap(PcbStr* pcb, int index) {
	if (pcb) {
		return pcb->IO_1_Traps[index % NUM_IO_TRAPS];
	} else {
		return -1;
	}
}

unsigned int PCBGetIO2Trap(PcbStr* pcb, int index) {
	if (pcb) {
		return pcb->IO_2_Traps[index % NUM_IO_TRAPS];
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

/*********************************************************************************/
/*                          		Setters			              		         */
/*********************************************************************************/


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

/*********************************************************************************/
/*                          		Getters			              		         */
/*********************************************************************************/


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
	if (pcb) {
		return pcb->PID;
	} else {
		return -1;
	}
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

/*********************************************************************************/
/*                          		String			              		         */
/*********************************************************************************/



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

/*********************************************************************************/
/*                         			Tests				                         */
/*********************************************************************************/



////test pcb
//int main(void) {
//	srand(time(NULL));
//	PcbStr* pcb = PCBConstructor(0);
//	printf("%s", PCBToString(pcb));
//	return 0;
//}
