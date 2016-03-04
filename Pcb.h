/***********************************************************************************************
* Pcb.h
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
* This header file defines the class and methods for the process control block implementation.
*
************************************************************************************************/

#ifndef PCB_H_
#define PCB_H_

//#include "ProdConsObj.h"

#define NUM_IO_TRAPS 4
#define MAX_PC 200

//PRODUCER CONSUMER DEFINES
#define PC_LOCK_UNLOCK 2
#define PC_WAIT 1
#define PC_SIGNAL 1
#define MAX_SHARED_SIZE 5

//SHARED RESOURCE DEFINES
#define SR_LOCK_UNLOCK 2

typedef enum {
	created=0,
	running=1,
	ready=2,
	interrupted=3,
	blocked=4,
	terminated=5
} State;

typedef enum {
	none=0,
	producer=1,
	consumer=2,
	mutrecA=3,
	mutrecB=4
} RelationshipType;


typedef struct PCB* PcbPtr;

typedef struct Mutex* MutexPtr;

unsigned int PCBGetIO1Trap(PcbPtr pcb, int index);
unsigned int PCBGetIO2Trap(PcbPtr pcb, int index);

/*Returns a string value for the given state.*/
char* StateToString(State state);

/**
 * Sets a new priority for this PCB.
 */
void PCBSetPriority(PcbPtr pcb, int priority);

/**
 * Sets a new ID for this PCB.
 */
void PCBSetID(PcbPtr pcb, int id);

/**
 * Sets the state for this PCB.
 */
void PCBSetState(PcbPtr pcb, State newState);

/**
 * Sets the PC for this PCB.
 */
void PCBSetPC(PcbPtr pcb, unsigned int newPC);

//void PCBSetMaxPC(PcbPtr pcb, unsigned int newMaxPC);

//void PCBSetCreation(PcbPtr pcb, unsigned int newCreation);

void PCBSetTermination(PcbPtr pcb, unsigned long newTermination);

void PCBSetTerminate(PcbPtr pcb, int newTerminate);

void PCBSetTermCount(PcbPtr pcb, unsigned int newTermCount);

/**
 * Returns PC of this PCB.
 */
unsigned int PCBGetPC(PcbPtr pcb);

/**
 * Returns the value of the priority for this PCB.
 */
int PCBGetPriority(PcbPtr pcb);

/**
 * Returns the value of the ID for this PCB.
 */
int PCBGetID(PcbPtr pcb);

/**
 * Returns the state of this PCB.
 */
State PCBGetState(PcbPtr pcb);

unsigned int PCBGetMaxPC(PcbPtr pcb);

unsigned long PCBGetCreation(PcbPtr pcb);

unsigned long PCBGetTermination(PcbPtr pcb);

int PCBGetTerminate(PcbPtr pcb);

unsigned int PCBGetTermCount(PcbPtr pcb);

PcbPtr PCBConstructor(PcbPtr thisPcb, RelationshipType theType, PcbPtr partner);

PcbPtr PCBAllocateSpace();

/**
 * Returns a string representation of this PCB.
 */
char *PCBToString(PcbPtr pcb);

/**
 * Deallocates all memory references that are kept within the PCB, and then frees the PCB passed in.
 */
void PCBDestructor(PcbPtr pcb);

int ProConWait(PcbPtr waiter);

PcbPtr ProConSignal(PcbPtr signaler);

//MUTEX STUFF
MutexPtr MutexConstructor();

void MutexDestructor(MutexPtr mutex);

void MutexLock(MutexPtr mutex, PcbPtr pcb);

void MutexUnlock(MutexPtr mutex, PcbPtr pcb);

#endif /* PCB_H_ */
