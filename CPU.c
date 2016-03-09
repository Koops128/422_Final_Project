/***********************************************************************************************
* CPU.c
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
* This C file implements the class and methods for the CPU.
*
************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "Pcb.h"
#include "Fifo.h"
#include "Device.h"
#include "Mutex.h"
#include "PriorityQueue.h"

//defines
#define TIMER_INTERRUPT 1
#define TERMINATE_INTERRUPT 2
#define PRO_CON_INTERRUPT 5
#define IO_REQUEST 3
#define IO_COMPLETION 4
#define TIMER_QUANTUM 500
#define NEW_PROCS		6
#define PC_PROCS	1
#define MR_PROCS	1
#define PRIORITY_LEVELS 16
#define ROUNDS_TO_PRINT 4 // the number of rounds to wait before printing simulation data
#define SIMULATION_END 100000 //the number of instructions to execute before the simulation may end
#define DEADLOCK	0//1			//Whether to do deadlock. 0 - no. 1 - yes.
#define CHECK_DEADLOCK_FREQUENCY 10 //Every number of instructions we run deadlock check
#define BLOCKED_BY_LOCK 	7
#define LOCK_UNBLOCK
#define NUM_MUT_REC_PAIRS 1//5
#define NUM_MUTEXES		  NUM_MUT_REC_PAIRS * 2 //each pair has two mutexes

//Global variables
int currPID; //The number of processes created so far. The latest process has this as its ID.
int timerCount;
int simCounter;
unsigned int sysStackPC;
unsigned int currQuantum;
FifoQueue* newProcesses;
PQPtr readyProcesses;
FifoQueue* terminatedProcesses;
PcbPtr currProcess;
Device* device1;
Device* device2;
MutexPtr mutexes[sizeof(MutexStr) * (PC_PROCS + MR_PROCS * 2)];
CondVarPtr condVars[sizeof(CondVarStr) * PC_PROCS * 2];

typedef enum {
	lockTrap=0,
	unlockTrap=1,
	waitTrap=2,
	signalTrap=3,
	noTrap=4
} ProdConsTrapType;

/*Prepares the waiting process to be executed.*/
void dispatcher() {
	currProcess = pqDequeue(readyProcesses);
	if (currProcess) {
		PCBSetState(currProcess, running);
		sysStackPC = PCBGetPC(currProcess);

		printf("PID %d was dispatched\r\n\r\n", PCBGetID(currProcess));
	} else {
		printf("Ready queue is empty, no process dispatched\r\n\r\n");
	}
}

/*=================================================
 *				Starvation Detection
 *=================================================*/
/*	A process is "starving" if it hasn't run for (numProcessesInQueue)*(priorityLevel)
 *  quantums. Appropriately, more processes in the system means each process is expected
 *  to get less time to run. Also, lower priority level means we expect the process not
 *  to have run for a longer time.
 *  A "baseline" would be equal running time for each process, so we would expect a process
 *  not to have run for (numProcessesInQueue) quantums before promoting it. Multiplying by
 *  the priority level is thus a way to weight this so lower priority processes aren't
 *  expected to get as much CPU time as higher priority ones.
 */
void runStarvationDetector() {
	//sum up number of processes
	int i, numProcs = 0;
	printf("\n---Running S Detector---\n");
	for (i = 0; i < PRIORITY_LEVELS; i++) {
		numProcs += ((readyProcesses->priorityArray)[i])->size;
		printf("Queue %d has %d processes\n", i, ((readyProcesses->priorityArray)[i])->size);
	}

	//Check head of each priority level (besides top) and see if it needs boosting
	//Check head of each priority level (except last) and see if it needs to go back to original level.
	//(Only check heads since going through the entire queues would be a lot of overhead)
	for (i = 0; i < PRIORITY_LEVELS; i++) {
		FifoQueue* fq = (readyProcesses->priorityArray)[i];
		if (fq) {
			PcbPtr pcb = fifoQueuePeek(fq);
			if (pcb) {
				//test if should be demoted
				if (i < (PRIORITY_LEVELS -1) && PCBGetStarveBoostFlag(pcb)) {
					//toggle flag and demote to lower level.
					PCBSetPriority(pcb, i + 1);
					PCBSetStarveBoostFlag(pcb, 0);
				}
				//	Test if should be promoted
				if (i > 0 && !PCBGetStarveBoostFlag(pcb)) {
					int threshold = numProcs * (i+1);
					int cyclesSinceRan = currQuantum - PCBGetLastQuantum(pcb);
					if (cyclesSinceRan > threshold ) {
						PCBSetPriority(pcb, i - 1); //lower priority number = higher priority
						PCBSetStarveBoostFlag(pcb, 1);
						pqEnqueue(readyProcesses, fifoQueueDequeue(fq));

						printf("Starvation detected. After not running for %d cycles, with %d processes in readyqueue, PID %d priority went from %d to %d.\n", cyclesSinceRan, numProcs, PCBGetID(pcb), i, i-1);
					}
				}
			}
		}
	}
	printf("---Exiting S Detector---\n");
}

//Scheduler
void scheduler(int interruptType) {
	//Get new processes into ready queue
	int i;
	for (i = 0; i < newProcesses->size; i++) {
		PcbPtr pcb = fifoQueueDequeue(newProcesses);
		PCBSetState(pcb, ready);
		fifoQueueEnqueue(readyProcesses, pcb);
	}

	switch (interruptType) {
	case TIMER_INTERRUPT :
		if (currProcess/*PCBGetState(currProcess) != blocked && PCBGetState(currProcess) != terminated*/) {
			fifoQueueEnqueue(readyProcesses, currProcess);
			PCBSetState(currProcess, ready);
		}
		dispatcher();
		break;
	case TERMINATE_INTERRUPT :
		fifoQueueEnqueue(terminatedProcesses, currProcess);
		PCBSetState(currProcess, terminated);
		PCBSetTermination(currProcess, time(NULL));

		printf("Process terminated: PID %d at %lu\r\n\r\n", PCBGetID(currProcess), PCBGetTermination(currProcess));

		dispatcher();
		break;
	case IO_REQUEST :
		//set currProccess back to running
		if (currProcess) {
			PCBSetState(currProcess, running);
		} else {
			dispatcher();
		}
		break;
	case IO_COMPLETION :
		if (currProcess) {
			dispatcher();
		}
		break;
	case PRO_CON_INTERRUPT :
		if (currProcess) {
			dispatcher();
		}
		break;
	case BLOCKED_BY_LOCK :
		if (currProcess) {
			PCBSetPC(currProcess, sysStackPC); //save it's pc, but don't put in ready queue; the mutex wait queue is holding it.
			dispatcher();
		}
		break;
	default :
		break;
	}
}

/*Saves the state of the CPU to the currently running PCB.*/
void saveCpuToPcb() {
	PCBSetPC(currProcess, sysStackPC);
}

/*The interrupt service routine for a timer interrupt.*/
void timerIsr() {
	if (currProcess/*PCBGetState(currProcess) != blocked && PCBGetState(currProcess) != terminated*/) {
		saveCpuToPcb();
		PCBSetState(currProcess, interrupted);
	}
	scheduler(TIMER_INTERRUPT);
}

void terminateIsr() {
	//save cpu to pcb??
	PCBSetState(currProcess, interrupted);
	scheduler(TERMINATE_INTERRUPT);
}

int setIOTimer(Device* device) {
	device->counter = (rand() % 3 + 3) * TIMER_QUANTUM;

	return 0;
}

/* I was assigned a IO_ISR Method. I assume we need it to
 * do something, but I dont know how relevant it is now.
 * The interrupt service routine for a IO interrupt
 * Does it still have to save Cpu state to Pcb? */
void IO_ISR(int numIO) {	//IOCompletionHandler
	if (currProcess) {
		saveCpuToPcb();
		PCBSetState(currProcess, interrupted);
	}
	//Get process from waiting queue
	PcbPtr pcb;
	if (numIO == 1) {
		pcb = fifoQueueDequeue(device1->waitQ);
		if (device1->waitQ->size > 0) {
			//reset the timer
			setIOTimer(device1);
		}
	}
	else if (numIO == 2) {
		pcb = fifoQueueDequeue(device2->waitQ);
		if (device2->waitQ->size > 0) {
			//reset the timer
			setIOTimer(device2);
		}
	}
	fifoQueueEnqueue(readyProcesses, pcb);
	PCBSetState(pcb, ready);

	printf("PID %d put in ready queue\r\n\r\n", PCBGetID(pcb));

	//PCBSetState(currProcess, ready);
	//set new io waiting queue process to running
	//currProcess = pcb;
	scheduler(IO_REQUEST);
}

/**Makes a new request to device*/
void IOTrapHandler(Device* d) {
	saveCpuToPcb();
	PCBSetState(currProcess, blocked);
	fifoQueueEnqueue(d->waitQ, currProcess);

	printf("PID %d put in waiting queue, ", PCBGetID(currProcess));

	if (d->waitQ->size == 1) {
		setIOTimer(d);
	}
	scheduler(IO_COMPLETION);
}

/*returns 0 if no context switch, 1 if context switch*/
int ProdConsTrapHandler(ProdConsTrapType trapRequest) {
	saveCpuToPcb();
	PCBSetState(currProcess, blocked);
	PcbPtr returned;
	int contextSwitch = 0; //0 for no context switch, 1 for context switch

	switch(trapRequest) {
		case lockTrap :
			//if lock, then call the pair's mutex for a lock.  If it needs to wait for the lock,
			//then call the scheduler with an interrupt
			if(!MutexLock(mutexes[PCBGetPCData(currProcess)->mutex], currProcess)) {
				scheduler(PRO_CON_INTERRUPT);
				contextSwitch = 1;
			}
			break;
		case unlockTrap :
			//if unlock, then call the pair's mutex to unlock
			MutexUnlock(mutexes[PCBGetPCData(currProcess)->mutex], currProcess);
			PCBSetState(currProcess, ready);
			break;
		case signalTrap :
			//if signal, then call ProConSignal, if a PCB is returned, then put it in the ready queue
			returned = ProConSignal(currProcess);
			if (returned) {
				fifoQueueEnqueue(readyProcesses, returned);
			}
			PCBSetState(currProcess, ready);
			break;
		case waitTrap :
			//if wait, then call ProConWait, if the process needs to wait, then call the scheduler with an interrupt
			if (ProConWait(currProcess)) {
				scheduler(PRO_CON_INTERRUPT);
				contextSwitch = 1;
			}
			break;
		default:
			break;
	}
	return contextSwitch;
}

//returns 0 if there's no io request, nonzero if request was made.
int checkIORequest(int devnum) {
	int requestMade = 0;
	int i;
	if (currProcess) {
		if (devnum == 1) { //look through array 1
			for (i=0; i < NUM_IO_TRAPS; i++) {
				requestMade = PCBGetIO1Trap(currProcess, i) == sysStackPC ? 1: requestMade;
			}
		}
		else if (devnum == 2) { //look through array 2
			for (i=0; i < NUM_IO_TRAPS; i++) {
				requestMade = PCBGetIO2Trap(currProcess, i) == sysStackPC ? 1: requestMade;
			}
		}
	}
	return requestMade;
}

ProdConsTrapType checkProdConsRequest() {
	PCStepsPtr pcSteps = PCBGetPCSteps(currProcess);

	int i;
	if (currProcess) {
		for (i = 0; i < PC_LOCK_UNLOCK; i++) {
			if (pcSteps->lock[i] == sysStackPC)
				return lockTrap;
			else if (pcSteps->unlock[i] == sysStackPC)
				return unlockTrap;
		}
		for (i = 0; i < PC_WAIT; i++) {
			if (pcSteps->wait[i] == sysStackPC)
				return waitTrap;
		}
		for (i = 0; i < PC_SIGNAL; i++) {
			if (pcSteps->signal[i] == sysStackPC)
				return signalTrap;
		}
	}
	return noTrap;
}

int checkIOInterrupt(Device* device) {
	if (device->counter > 0) { //still counting down
		device->counter--;
		return 0;
	} else if (device->counter == 0) {	//we've reached the end of the waiting time, throw interrupt
		device->counter = -1;
		return 1;
	} else {	//this IO device isn't active
		return 0;
	}
}

int timerCheck() {
	if (timerCount > 0) {
		timerCount--;
		return 0;
	} else {
		timerCount = TIMER_QUANTUM;
		return 1;
	}
}

/*=================================================
 *				Deadlock Check
 *=================================================*/

/**
 * Checks if the pcb is locked by another pcb
 * and returns the Mutex owner pcb if it is, null otherwise
 */
PcbPtr isLocked(PcbPtr owner) {
	int i, j;
	for (i = 0; i < NUM_MUTEXES; i++) {
		MutexPtr m = mutexes[i];
		if (m->owner != NULL && MutexHasWaiting(m)) {
			for (j = 0; j < m->waitQ->size; j++) {
				if (fifoQueueContains(m->waitQ, owner) != -1) { //being locked, return mutex owner
					return m->owner;
				}
			}
		}
	}
	return NULL;
}

/*
 * Checks the chain of pcbs being locked by this pcb
 *
 * PcbPtr owner the pcb being checked
 * Returns 1 if pcb is deadlocked, 0 otherwise
 */
int checkLock(PcbPtr owner) {
	PcbPtr parent = isLocked(owner);
	while (parent != NULL) {//check what its locked by repeatedly
		if (owner == parent) { //locked by lock itself is locking
			return 1;
		}
		parent = isLocked(parent);//else check what that pcb is locked by
	}
	return 0; //pcb is not locked, chain is done
}

/**
 * Returns 1 if true 0 otherwise
 */
int deadlockDetect() {
	int i, r;
	for (i = 0; i < NUM_MUTEXES; i++) {
		if (mutexes[i]->owner != NULL) {
			r = checkLock(mutexes[i]->owner);
			if (r == 1) {
				printf("\r\nDeadlock detected for process %d", PCBGetID(mutexes[i]->owner));
				return 1;
			}
		}
	}
	printf("\r\nno deadlock detected\r\n");
	return 0;
}

/*=================================================
 *			Mutex Lock/Unlock Check
 *=================================================*/

/*Returns 1 if the current pcb a) got the lock or b) didn't want a lock. In this case,
 *normal execution can continue.
 *Returns 0 if the current pcb is blocked waiting for the lock. In this case, another process
 *must take over execution.*/
int notBlockedByLock() {
	int notBlocked = 0;
	MutexPtr selected = NULL;
	if (isMutexLockStep(currProcess, 1, sysStackPC)) {
		int index = PCBGetMutexIndex(currProcess, 1) % (NUM_MUTEXES); //Must put in parens since NUM_MUTEXES is an expression, not a value.
		selected = mutexes[index];
	} else if (isMutexLockStep(currProcess, 2, sysStackPC)) {
		int index = PCBGetMutexIndex(currProcess, 2) % (NUM_MUTEXES);
		selected = mutexes[index];
	}
	if (selected) {
		notBlocked = MutexLock(selected, currProcess); //Wants lock; result depends on whether it was blocked.
	} else {
		notBlocked = 1; //Didn't want lock
	}
	return notBlocked;
}

/*If we're on an unlock step for the current pcb, then unlocks that mutex.
 *If there was a pcb waiting for that mutex, then that pcb is returned;
 *otherwise, null is returned.
 *return: PcbPtr -- the PCB that was waiting for the lock; else, null.*/
PcbPtr checkUnlock() {
	PcbPtr unlockedPcb = NULL;
	MutexPtr selected = NULL;
	if (isMutexUnlockStep(currProcess, 1, sysStackPC)) {
		selected = mutexes[PCBGetMutexIndex(currProcess, 1) % (NUM_MUTEXES)];
	} else if (isMutexUnlockStep(currProcess, 2, sysStackPC)) {
		selected = mutexes[PCBGetMutexIndex(currProcess, 2) % (NUM_MUTEXES)];
	}
	if (selected) {
		MutexUnlock(selected, currProcess);
		unlockedPcb = selected->owner; //NULL if none had been waiting
	}
	return unlockedPcb;
}

void printIfInCriticalSection() {
	MutexPtr Mutex1 = mutexes[PCBGetMutexIndex(currProcess, 1)];
	MutexPtr Mutex2 = mutexes[PCBGetMutexIndex(currProcess, 2)];
	if (Mutex1->owner == currProcess && Mutex2->owner == currProcess) {
		printf("~~PID %d is in critical section~~\n", PCBGetID(currProcess));
	}
}

/*Randomly generates between 0 and 5 new processes and enqueues them to the New Processes Queue.*/
void genProcesses() {
	PcbPtr newProc;
	int i;
	// rand() % NEW_PROCS will range from 0 to NEW_PROCS - 1, so we must use rand() % (NEW_PROCS + 1)
	for(i = 0; i < rand() % (NEW_PROCS + 1); i++)
	{
		newProc = PCBAllocateSpace();//(PcbPtr) malloc(sizeof(PcbStr));
		PCBConstructor(newProc, none, NULL);
		if(newProc != NULL)	// Remember to call the destructor when finished using newProc
		{
			currPID++;
			PCBSetID(newProc, currPID);
			PCBSetPriority(newProc, rand() % PRIORITY_LEVELS);
			fifoQueueEnqueue(newProcesses, newProc);

			printf("Process created: PID: %d at %lu\r\n", PCBGetID(newProc), PCBGetCreation(newProc));
			//printf("Process created: %s\r\n", PCBToString(newProc));
		}
	}
}

void genSharedResourcePairs() {
	//TODO
}

void genProducerConsumerPairs() {
	PcbPtr Producer;
	PcbPtr Consumer;
	int i;

	//TODO create and add mutexes and condition variables for these pcbs.
	for (i = 0; i < PC_PROCS; i++) {
		Producer = PCBAllocateSpace();//(PcbPtr) malloc(sizeof(PcbStr));
		Consumer = PCBAllocateSpace();//(PcbPtr) malloc(sizeof(PcbStr));

		PCBConstructor(Producer, producer, Consumer);
		currPID++;
		PCBSetID(Producer, currPID);
		PCBSetPriority(Producer, rand() % PRIORITY_LEVELS);
		fifoQueueEnqueue(newProcesses, Producer);
		printf("Producer process created: PID: %d at %lu\r\n", PCBGetID(Producer), PCBGetCreation(Producer));

		PCBConstructor(Consumer, consumer, Producer);
		currPID++;
		PCBSetID(Consumer, currPID);
		PCBSetPriority(Consumer, rand() % PRIORITY_LEVELS);
		fifoQueueEnqueue(newProcesses, Consumer);
		printf("Consumer process created: PID: %d at %lu\r\n", PCBGetID(Consumer), PCBGetCreation(Consumer));
	}

}

void checkTimerInterrupt() {
	if (timerCheck() == 1) {
		genProcesses();
		if (currProcess) {
			printf("Timer interrupt: PID %d was running, ", PCBGetID(currProcess));
		} else {
			printf("Timer interrupt: no current process is running, ");
		}

		timerIsr();
	}
}

void checkIOInterrupts() {
	//check if there has been an IO interrupt, if so call appropriate ioISR
	if (checkIOInterrupt(device1) == 1) {
		if (currProcess) {
			printf("I/O 1 Completion interrupt: PID %d is running, ", PCBGetID(currProcess));
		} else {
			printf("I/O 1 Completion interrupt: no current process is running, ");
		}
		//call the IO service routine
		IO_ISR(1);
	}

	if (checkIOInterrupt(device2) == 1) {
		if (currProcess) {
			printf("I/O 1 Completion interrupt: PID %d is running, ", PCBGetID(currProcess));
		} else {
			printf("I/O 1 Completion interrupt: no current process is running.");
		}
		//call the IO service routine
		IO_ISR(2);
	}
}

//returns 0 or 1, 1 if the process has been terminated, 0 if not
int checkTermCountAndTermination() {
	if (currProcess && sysStackPC >= PCBGetMaxPC(currProcess)) {
		PCBSetTermCount(currProcess, PCBGetTermCount(currProcess) + 1);
		printf("\r\n");
		//if TERM_COUNT = TERMINATE, then call terminateISR to put this process in the terminated list
		if (PCBGetTermCount(currProcess) == PCBGetTerminate(currProcess)) {

			terminateIsr();
			return 1;	//currProcess has been terminated, we don't want to execute the rest of the loop, instead jump to next iteration
		}
		sysStackPC = 0;
	}
	return 0;
}

/*returns 0 if no trap request, 1 if yes*/
int checkIOTraps() {
	if (currProcess && checkIORequest(1) != 0) {
		printf("I/O trap request: I/O device 1, ");
		IOTrapHandler(device1);
		return 1;
	}

	if (currProcess && checkIORequest(2) != 0) {
		printf("I/O trap request: I/O device 2, ");
		IOTrapHandler(device2);
		return 1;
	}
	return 0;
}


//returns 0 if no context switch, 1 if context switch
int checkPCTraps() {
	ProdConsTrapType trapRequest = checkProdConsRequest();
	if (trapRequest == lockTrap || trapRequest == unlockTrap || trapRequest == waitTrap || trapRequest == signalTrap) {
		return ProdConsTrapHandler(trapRequest);
	}
}

//returns 0 if no context switch, 1 if context switch
int checkMRTraps() {
	if (!notBlockedByLock()) {
		//TODO make an isr for this
		scheduler(BLOCKED_BY_LOCK);
		simCounter++;
		return 1;
		//continue;
	} else {
		PcbPtr wasWaitingPcb = checkUnlock();
		if (wasWaitingPcb) {
			//TODO make an isr for this
			pqEnqueue(readyProcesses, wasWaitingPcb);
		}
		return 0;
	}

	printIfInCriticalSection();
}

void cpu() {
	genProcesses();
	genProducerConsumerPairs();
	genSharedResourcePairs();

		printf("\r\nBegin Simulation:\r\n\r\n");

		simCounter = 0;

		while (simCounter <= SIMULATION_END) {

/****check for timer interrupt, if so, call timerISR()****/
			checkTimerInterrupt();

/****Checking for IO interrupts****/
			checkIOInterrupts();

/****check the current process's PC, if it is MAX_PC, set to 0 and increment TERM_COUNT*****/
			if (checkTermCountAndTermination()) {
				continue;
			}

			sysStackPC++;

/****Checking traps****/
			if (checkIOTraps()) {
				continue;
			}
			RelationshipPtr relationship = PCBGetRelationship(currProcess);
			if ((relationship->mType == producer || relationship->mType == consumer) && checkPCTraps()) {
				continue;
			} else if ((relationship->mType == mutrecA || relationship->mType == mutrecB) && checkMRTraps()) {
				continue;
			}
		}

/****Checking for deadlock****/
		if (simCounter % CHECK_DEADLOCK_FREQUENCY == 0) {
			if (deadlockDetect()) {
				printf(">>>>>Deadlock detected!!!!!!!!!!!!!<<<<<<\r\n");
			}
		}

/****Checking for starvation****/
		//TODO

		//at end
		simCounter++;
}

int main(void) {
	srand(time(NULL));
	currPID = 0;
	sysStackPC = 0;
	timerCount = TIMER_QUANTUM;
	newProcesses = fifoQueueConstructor();
	readyProcesses = pqConstructor();
	terminatedProcesses = fifoQueueConstructor();
	device1 = DeviceConstructor();
	device2 = DeviceConstructor();
	//mutexes = (MutexPtr) malloc(sizeof(MutexStr) * (PC_PROCS + MR_PROCS * 2));

	printf("Sean Markus\r\nWing-Sea Poon\r\nAbigail Smith\r\nTabi Stein\r\n\r\n");

	//An initial process to start with
	currProcess = PCBAllocateSpace();//(PcbPtr) malloc(sizeof(PcbStr));
	PCBConstructor(currProcess, none, NULL);
	if(currProcess != NULL)	// Remember to call the destructor when finished using newProc
	{
		PCBSetID(currProcess, currPID);
		PCBSetPriority(currProcess, rand() % PRIORITY_LEVELS);
		PCBSetState(currProcess, running);
		printf("Process created: PID: %d at %lu\r\n", PCBGetID(currProcess), PCBGetCreation(currProcess));
		cpu();
	}

	//free all the things!
	fifoQueueDestructor(&newProcesses);
	pqDestructor(readyProcesses);
	fifoQueueDestructor(&terminatedProcesses);

	DeviceDestructor(device1);
	DeviceDestructor(device2);

	printf("End of simulation\r\n");
	return 0;
}
