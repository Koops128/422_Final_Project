#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "Pcb.h"
#include "Fifo.h"
#include "PriorityQueue.h"
#include "Mutex.h"

//typedef enum interruptType {
//	TIMER, TERMINATE, IO_REQUEST, IO_COMPLETION, LOCK_BLOCK, LOCK_UNBLOCK
//} interruptType;

#define TIMER_INTERRUPT 	1
#define TERMINATE_INTERRUPT 2
#define IO_REQUEST 			3
#define IO_COMPLETION 		4
#define BLOCKED_BY_LOCK 	5
#define LOCK_UNBLOCK 		6

#define NUM_MUT_REC_PAIRS 1//5
#define NUM_MUTEXES		  NUM_MUT_REC_PAIRS * 2 //each pair has two mutexes

#define NEW_PROCS		6		// max num new processes to make per quantum
#define TIMER_QUANTUM 	10//500
#define ROUNDS_TO_PRINT 4 		// the number of rounds to wait before printing simulation data
#define SIMULATION_END 	1000//100000 	// the number of instructions to execute before the simulation may end

#define DEADLOCK	0//1			//Whether to do deadlock. 0 - no. 1 - yes.
#define CHECK_DEADLOCK_FREQUENCY 10 //Every number of instructions we run deadlock check

/*==========================================
 * 			Global Variables
 *==========================================*/


int currPID; 				/*The number of processes created so far. The latest process has this as its ID.*/
int timerCount;
unsigned int sysStackPC;
unsigned int currQuantum;
PcbPtr currProcess;

MutexPtr mutexes[NUM_MUTEXES];

/**		Queues		**/
FifoQueue* newProcesses;
PQPtr readyProcesses;
//FifoQueue* readyProcesses;
FifoQueue* terminatedProcesses;


/*=================================================
 *					IO Device Type
 *=================================================*/

typedef struct {
	FifoQueue* waitQ;
	int counter;
} Device;

Device* IODeviceConstructor() {
	Device* device = (Device*) malloc(sizeof(Device));
	device->waitQ = fifoQueueConstructor();
	device->counter = -1;
	return device;
}

void IODeviceDestructor(Device* device) {
	fifoQueueDestructor(&device->waitQ);
	free(device);
}

Device* device1;
Device* device2;


/*Prepares the waiting process to be executed.*/
void dispatcher() {
	currProcess = pqDequeue(readyProcesses);
	if (currProcess) {
		PCBSetState(currProcess, running);
		sysStackPC = PCBGetPC(currProcess);
		PCBSetLastQuantum(currProcess, currQuantum);
		if (PCBGetStarveBoostFlag(currProcess)) {
			PCBSetStarveBoostFlag(currProcess, 0);
			PCBSetPriority(currProcess, PCBGetPriority(currProcess) + 1);
		}
		printf("PID %d was dispatched\n\n", PCBGetID(currProcess));
	} else {
		//currProcess = NULL;
		printf("Ready queue is empty, no process dispatched\n\n");
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


void scheduler(int interruptType) {
	//Get new processes into ready queue
	int i;
	for (i = 0; i < newProcesses->size; i++) {
		PcbPtr pcb = fifoQueueDequeue(newProcesses);
		PCBSetState(pcb, ready);
		pqEnqueue(readyProcesses, pcb);
		//fifoQueueEnqueue(readyProcesses, pcb);
	}
	runStarvationDetector();

	switch (interruptType) {
	case TIMER_INTERRUPT :
		if (currProcess) {
			pqEnqueue(readyProcesses, currProcess);
			//fifoQueueEnqueue(readyProcesses, currProcess);
			PCBSetState(currProcess, ready);
		}
		dispatcher();
		break;
	case TERMINATE_INTERRUPT :
		fifoQueueEnqueue(terminatedProcesses, currProcess);
		PCBSetState(currProcess, terminated);
		PCBSetTermination(currProcess, time(NULL));

		printf("Process terminated: PID %d at %lu\n\n", PCBGetID(currProcess), PCBGetTermination(currProcess));

		dispatcher();
		break;
	case IO_REQUEST :
		if (currProcess) {
			dispatcher();
		}
		break;
	case IO_COMPLETION :
		if (currProcess) {
			PCBSetState(currProcess, running);
		} else {
			dispatcher(); // executed when no processes are running,
						  // because they had all been waiting for IO.
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



/*=================================================
 *				Process Generation
 *=================================================*/

/*Randomly generates between 0 and <NEW_PROCS> new processes and enqueues them to the New Processes Queue.*/
void genProcesses() {
	int i;
	// rand() % NEW_PROCS will range from 0 to NEW_PROCS - 1, so we must use rand() % (NEW_PROCS + 1)
	for(i = 0; i < rand() % (NEW_PROCS + 1); i++)
	{
		//PcbPtr newProc = PCBAllocateSpace();
		PcbPtr newProc = PCBConstructor(PCBAllocateSpace(), none, NULL);
		if(newProc != NULL)	// Remember to call the destructor when finished using newProc
		{
			currPID++;
			PCBSetID(newProc, currPID);
			PCBSetPriority(newProc, rand() % PRIORITY_LEVELS);
			PCBSetState(newProc, created);
			PCBSetLastQuantum(newProc, currQuantum);
			fifoQueueEnqueue(newProcesses, newProc);

//			printf("Process created: PID: %d at %lu\n", PCBGetID(newProc), PCBGetCreation(newProc));
			printf("Process created: PID: %d, Priority %d at %lu\n", PCBGetID(newProc), PCBGetPriority(newProc), PCBGetCreation(newProc));
		}
	}
}

/*Creates <NUM_MUT_REC_PAIRS> mutual resource user pairs and enqueues in new queue.*/
void genMutualResourceUsers() {
//	unsigned int lock1Steps[NUM_MUTEX_STEPS] = {20, 40, 60, 80}; //All the same, for simplicity sake and ease of analysis
//	unsigned int lock2Steps[NUM_MUTEX_STEPS] = {21, 41, 61, 81};
//	unsigned int unlock2Steps[NUM_MUTEX_STEPS] = {23, 43, 63, 83};
//	unsigned int unlock1Steps[NUM_MUTEX_STEPS] = {24, 44, 64, 84}; //All the same, for simplicity sake and ease of analysis

	/* Use of the following makes deadlock more likely because there are more
	 * instructions where a timer interrupt could occur and the other process
	 * could get a lock on the other lock.*/
	unsigned int lock1Steps[NUM_MUTEX_STEPS] = {20, 40, 60, 80}; //All the same, for simplicity sake and ease of analysis
	unsigned int lock2Steps[NUM_MUTEX_STEPS] = {25, 45, 65, 85};
	unsigned int unlock2Steps[NUM_MUTEX_STEPS] = {30, 50, 70, 90};
	unsigned int unlock1Steps[NUM_MUTEX_STEPS] = {35, 55, 75, 95}; //All the same, for simplicity sake and ease of analysis


	int i;
	for (i = 0; i < NUM_MUT_REC_PAIRS; i++) {
		int priority = rand() % PRIORITY_LEVELS;
		PcbPtr Ai = PCBAllocateSpace();
		PcbPtr Bi = PCBAllocateSpace();
		PCBConstructor(Ai, mutrecA, Bi);
		PCBConstructor(Bi, mutrecB, Ai);
		PCBSetLastQuantum(Ai, currQuantum);
		PCBSetLastQuantum(Bi, currQuantum);
		PCBSetID(Ai, ++currPID);
		PCBSetID(Bi, ++currPID);
		PCBSetPriority(Ai, priority); //Both at same priority to ensure progress. Not sure if this is a hack.
		PCBSetPriority(Bi, priority);
		int mutex1Idx = 2 * i;
		int mutex2Idx = (2 * i) + 1;
		mutexes[mutex1Idx] = MutexConstructor(mutex1Idx);
		mutexes[mutex2Idx] = MutexConstructor(mutex2Idx);
		PCBSetMutexIndex(Ai, 1, mutex1Idx);
		PCBSetMutexIndex(Ai, 2, mutex2Idx);

		if (DEADLOCK) {
			PCBSetMutexIndex(Bi, 1, mutex2Idx); //make mutex 2 be the first it tries to get a lock on
			PCBSetMutexIndex(Bi, 2, mutex1Idx);
		} else {
			PCBSetMutexIndex(Bi, 1, mutex1Idx);
			PCBSetMutexIndex(Bi, 2, mutex2Idx);
		}

		PCBSetMutexLockSteps(Ai, 1, lock1Steps);
		PCBSetMutexLockSteps(Bi, 1, lock1Steps);
		PCBSetMutexLockSteps(Ai, 2, lock2Steps);
		PCBSetMutexLockSteps(Bi, 2, lock2Steps);
		PCBSetMutexUnlockSteps(Ai, 2, unlock2Steps);
		PCBSetMutexUnlockSteps(Bi, 2, unlock2Steps);
		PCBSetMutexUnlockSteps(Ai, 1, unlock1Steps);
		PCBSetMutexUnlockSteps(Bi, 1, unlock1Steps);

		initializeTrapArray(Ai);
		initializeTrapArray(Bi);


		printf("Mutual resource user Process created: PID: %d Priority %d at %lu\n", PCBGetID(Ai), PCBGetPriority(Ai), PCBGetCreation(Ai));
		printf("Mutual resource user Process created: PID: %d Priority %d at %lu\n", PCBGetID(Bi), PCBGetPriority(Bi), PCBGetCreation(Bi));

		fifoQueueEnqueue(newProcesses, Ai);
		fifoQueueEnqueue(newProcesses, Bi);
	}

//
//	PcbPtr A0, B0; //, A1, B1, A2, B2, A3, B3, A4, B4;
//	A0 = PCBConstructor(MUTREC);
//	B0 = PCBConstructor(MUTREC);
//	MutexPtr mutex01 = MutexConstructor(0);
//	MutexPtr mutex02 = MutexConstructor(1);
//	mutexes[0] = mutex01;
//	mutexes[1] = mutex02;
//	PCBSetID(A0, ++currPID);
//	PCBSetID(B0, ++currPID);
//	PCBSetMutexIndex(A0, 1, 0);
//	PCBSetMutexIndex(B0, 1, 0);
//
//	PCBSetMutexIndex(A0, 2, 1);
//	PCBSetMutexIndex(B0, 2, 1);
//	unsigned int A0_1LockSteps[NUM_MUTEX_STEPS] = {20, 40, 60, 80};
//	unsigned int B0_1LockSteps[NUM_MUTEX_STEPS] = {20, 40, 60, 80};
//	PCBSetMutexLockSteps(A0, 1, A0_1LockSteps);
//	PCBSetMutexLockSteps(B0, 1, B0_1LockSteps);
//
//	unsigned int A0_2LockSteps[NUM_MUTEX_STEPS] = {21, 41, 61, 81};
//	unsigned int B0_2LockSteps[NUM_MUTEX_STEPS] = {21, 41, 61, 81};
//	PCBSetMutexLockSteps(A0, 2, A0_2LockSteps);
//	PCBSetMutexLockSteps(B0, 2, B0_2LockSteps);
//
//	unsigned int A0_2UnlockSteps[NUM_MUTEX_STEPS] = {23, 43, 63, 83};
//	unsigned int B0_2UnlockSteps[NUM_MUTEX_STEPS] = {23, 43, 63, 83};
//	PCBSetMutexUnlockSteps(A0, 2, A0_2UnlockSteps);
//	PCBSetMutexUnlockSteps(B0, 2, B0_2UnlockSteps);
//
//	unsigned int A0_1UnlockSteps[NUM_MUTEX_STEPS] = {24, 44, 64, 84};
//	unsigned int B0_1UnlockSteps[NUM_MUTEX_STEPS] = {24, 44, 64, 84};
//	PCBSetMutexUnlockSteps(A0, 1, A0_1UnlockSteps);
//	PCBSetMutexUnlockSteps(B0, 1, B0_1UnlockSteps);
//
//	PCBSetPriority(A0, 0);
//	PCBSetPriority(B0, 0);
//
//	initializeTrapArray(A0);
//	initializeTrapArray(B0);
//
//	printf("Mutrec Process created: PID: %d at %lu\n", PCBGetID(A0), PCBGetCreation(A0));
//	printf("Mutrec Process created: PID: %d at %lu\n", PCBGetID(B0), PCBGetCreation(B0));
//
//	fifoQueueEnqueue(newProcesses, A0);
//	fifoQueueEnqueue(newProcesses, B0);
}

/*Saves the state of the CPU to the currently running PCB.*/
void saveCpuToPcb() {
	PCBSetPC(currProcess, sysStackPC);
}

/*=================================================
 *					Termination
 *=================================================*/

void terminateIsr() {
	//save cpu to pcb??
	PCBSetState(currProcess, interrupted);
	scheduler(TERMINATE_INTERRUPT);
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

/*=================================================
 *			Timer Check and Interrupt
 *=================================================*/

int setIOTimer(Device* device) {
	device->counter = (rand() % 3 + 3) * TIMER_QUANTUM;

	return 0;
}

/*The interrupt service routine for a timer interrupt.*/
void timerIsr() {
	if (currProcess) {
		saveCpuToPcb();
		PCBSetState(currProcess, interrupted);
	}
	scheduler(TIMER_INTERRUPT);
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

/*Checks timer, calling timerIsr if necessary and incrementing currQuantum counter.*/
void checkTimerInterrupt() {
	if (timerCheck() == 1) {
		currQuantum++;
		printf("\r\n===================Quantum %d=====================\r\n", currQuantum);
		//TODO comment back in (commented out for simplicity to see how stuff works)
		genProcesses();
		if (currProcess) {
			printf("Timer interrupt: PID %d was running, ", PCBGetID(currProcess));
		} else {
			printf("Timer interrupt: no current process is running, ");
		}
		timerIsr();
	}
}

/*=================================================
 *			IO Completion Check and Interrupt
 *=================================================*/

void IO_ISR(int numIO) { //IOCompletionHandler
	if (currProcess) {
		saveCpuToPcb();
		PCBSetState(currProcess, interrupted);
	}
	//Get process from IO waiting queue
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
	pqEnqueue(readyProcesses, pcb);
	//fifoQueueEnqueue(readyProcesses, pcb);
	PCBSetState(pcb, ready);

	printf("PID %d put in ready queue\r\n", PCBGetID(pcb));
	scheduler(IO_COMPLETION);
}

/*Checks if the process that is currently getting IO is done getting IO.*/
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

void checkIOInterrupts() {
	if (checkIOInterrupt(device1) == 1) {
		if (currProcess) {
			printf("I/O 1 Completion interrupt: PID %d is running, ",
					PCBGetID(currProcess));
		} else {
			printf("I/O 1 Completion interrupt: no current process is running, ");
		}
		IO_ISR(1);
	}

	if (checkIOInterrupt(device2) == 1) {
		if (currProcess) {
			printf("I/O 1 Completion interrupt: PID %d is running, ",
					PCBGetID(currProcess));
		} else {
			printf(
					"I/O 1 Completion interrupt: no current process is running.");
		}
		IO_ISR(2);
	}
}


/*=================================================
 *			IO Request Check and Handler
 *=================================================*/

/**Makes a new request to device*/
void IOTrapHandler(Device* d) {
	saveCpuToPcb();
	PCBSetState(currProcess, blocked);
	fifoQueueEnqueue(d->waitQ, currProcess);

	printf("PID %d put in waiting queue, ", PCBGetID(currProcess));

	if (d->waitQ->size == 1) {
		setIOTimer(d);
	}
	scheduler(IO_REQUEST);
	//dispatcher();
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

void checkIOTraps() {
	if (currProcess && checkIORequest(1) != 0) {
		printf("I/O trap request: I/O device 1, ");
		IOTrapHandler(device1);
	}

	if (currProcess && checkIORequest(2) != 0) {
		printf("I/O trap request: I/O device 2, ");
		IOTrapHandler(device2);
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


void cpu() {
	//TODO comment back in when done testing mut rec
	//genProcesses();
	genMutualResourceUsers();

	printf("\nBegin Simulation:\n\n");

	int simCounter = 0;

	while (simCounter <= SIMULATION_END) {

		checkTimerInterrupt();
		checkIOInterrupts(); /*Ok to do before checking for termination, since this does not advance us forward an instruction.*/

		/******************************************
		 *		Checking for Termination
		 ******************************************/
		/* Before we increment the SysStack, we need to know if the process is at its max PC (so we don't go beyond it). */
		if(checkTermCountAndTermination()){
			continue;
			/*We have just loaded a new process; If we did rest of iteration, we might risk
			executing lines of this process, without first checking if it's at its max pc.*/
		}

		/** Now that we know the current process is not at the end,
		 * we can safely increment the sys stack pc.  **/
		sysStackPC++;

		checkIOTraps();

		//TODO delete this when done debugging.
		if (currProcess) {
			printf("Current Process (PID: %d, Priority: %d) PC: %d\r\n", PCBGetID(currProcess), PCBGetPriority(currProcess), sysStackPC);
		}

		/******************************************
		 *		Checking Mutual Resource User
		 ******************************************/
		if (PCBgetPairType(currProcess) == mutrecA || PCBgetPairType(currProcess) == mutrecB) {
			if (!notBlockedByLock()) {
				//TODO make an isr for this
				scheduler(BLOCKED_BY_LOCK);
				simCounter++;
				continue;
			} else {
				PcbPtr wasWaitingPcb = checkUnlock();
				if (wasWaitingPcb) {
					//TODO make an isr for this
					pqEnqueue(readyProcesses, wasWaitingPcb);
				}
			}

			printIfInCriticalSection();

		}


		if (simCounter % CHECK_DEADLOCK_FREQUENCY == 0) {
			if (deadlockDetect()) {
				printf(">>>>>Deadlock detected!!!!!!!!!!!!!<<<<<<\r\n");
			}
		}

		//at end
		simCounter++;
	}
}

int main(void) {
	srand(time(NULL));
	currPID = 0;
	sysStackPC = 0;
	timerCount = TIMER_QUANTUM;
	newProcesses = fifoQueueConstructor();
	readyProcesses = pqConstructor();
	terminatedProcesses = fifoQueueConstructor();
	device1 = IODeviceConstructor();
	device2 = IODeviceConstructor();

	currQuantum = 0;

	//An initial process to start with
	currProcess = PCBConstructor(PCBAllocateSpace(), none, NULL);
	if(currProcess != NULL)
	{
		PCBSetID(currProcess, currPID);
		PCBSetPriority(currProcess, rand() % PRIORITY_LEVELS);
		PCBSetState(currProcess, running);
		PCBSetLastQuantum(currProcess, currQuantum);
		printf("Process created: PID: %d at %lu\n", PCBGetID(currProcess), PCBGetCreation(currProcess));
		cpu();
	}

	//free all the things!
	fifoQueueDestructor(&newProcesses);
	pqDestructor(readyProcesses);
	fifoQueueDestructor(&terminatedProcesses);

	IODeviceDestructor(device1);
	IODeviceDestructor(device2);

	printf("End of simulation\n");
	return 0;
}
