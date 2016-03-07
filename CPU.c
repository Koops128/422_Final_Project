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

//defines
#define TIMER_INTERRUPT 1
#define TERMINATE_INTERRUPT 2
#define IO_REQUEST 3
#define IO_COMPLETION 4
#define TIMER_QUANTUM 500
#define NEW_PROCS		6
#define PRIORITY_LEVELS 16
#define ROUNDS_TO_PRINT 4 // the number of rounds to wait before printing simulation data
#define SIMULATION_END 100000 //the number of instructions to execute before the simulation may end

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

//Global variables
int currPID; //The number of processes created so far. The latest process has this as its ID.
//int timerCount; //the counter for timer interrupts
/*int io1Count; //the counter for I/O 1
int io2Count; //the counter for I/O 2*/
int timerCount;
unsigned int sysStackPC;
FifoQueue* newProcesses;
FifoQueue* readyProcesses;
FifoQueue* terminatedProcesses;
PcbPtr currProcess;
Device* device1;
Device* device2;


/*Prepares the waiting process to be executed.*/
void dispatcher() {
	currProcess = fifoQueueDequeue(readyProcesses);
	//if (readyProcesses->size > 0) {
	if (currProcess) {
		//currProcess = fifoQueueDequeue(readyProcesses);
		PCBSetState(currProcess, running);
		sysStackPC = PCBGetPC(currProcess);

		printf("PID %d was dispatched\r\n\r\n", PCBGetID(currProcess));
	} else {
		//currProcess = NULL;
		printf("Ready queue is empty, no process dispatched\r\n\r\n");
	}
}

//Scheduler
void scheduler(int interruptType) {
	//Get new processes into ready queue
	int i;
	for (i = 0; i < newProcesses->size; i++) {
		PcbPtr pcb = fifoQueueDequeue(newProcesses);
		PCBSetState(pcb, ready);
		fifoQueueEnqueue(readyProcesses, pcb);
		//fprintf(outFilePtr, "%s\r\r\n", PCBToString(pcb));
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
		if (currProcess/*PCBGetState(currProcess) != blocked && PCBGetState(currProcess) != terminated*/) {
			PCBSetState(currProcess, running);
		} else {
			dispatcher();
		}
		break;
	case IO_COMPLETION :
		if (currProcess/*PCBGetState(currProcess) != blocked && PCBGetState(currProcess) != terminated*/) {
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
	if (currProcess/*PCBGetState(currProcess) != blocked && PCBGetState(currProcess) != terminated*/) {
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
	//put current process into ready queue
	//fifoQueueEnqueue(readyProcesses, currProcess);
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
	//dispatcher();
}

//returns 0 if there's no io request, nonzero if request was made.
int checkIORequest(int devnum) {
	int requestMade = 0;
	int i;
	if (currProcess/*PCBGetState(currProcess) != blocked && PCBGetState(currProcess) != terminated*/) {
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

/*Randomly generates between 0 and 5 new processes and enqueues them to the New Processes Queue.*/
void genProcesses() {
	PcbPtr newProc;
	int i;
	// rand() % NEW_PROCS will range from 0 to NEW_PROCS - 1, so we must use rand() % (NEW_PROCS + 1)
	for(i = 0; i < rand() % (NEW_PROCS + 1); i++)
	{
		newProc = PCBConstructor();
		if(newProc != NULL)	// Remember to call the destructor when finished using newProc
		{
			currPID++;
			PCBSetID(newProc, currPID);
			PCBSetPriority(newProc, rand() % PRIORITY_LEVELS);
			PCBSetState(newProc, created);
			fifoQueueEnqueue(newProcesses, newProc);

			printf("Process created: PID: %d at %lu\r\n", PCBGetID(newProc), PCBGetCreation(newProc));
			//printf("Process created: %s\r\n", PCBToString(newProc));
		}
	}
}

void cpu() {
	genProcesses();

		printf("\r\nBegin Simulation:\r\n\r\n");

		int simCounter = 0;

		while (simCounter <= SIMULATION_END) {

			//check for timer interrupt, if so, call timerISR()
			if (timerCheck() == 1) {
				//printf("Timer interrupt: PID %d was running, ", PCBGetID(currProcess));
//				if (PCBGetState(currProcess) != terminated) {
				genProcesses();
				if (currProcess/*PCBGetState(currProcess) != blocked && PCBGetState(currProcess) != terminated*/) {
					printf("Timer interrupt: PID %d was running, ", PCBGetID(currProcess));
				} else {
					printf("Timer interrupt: no current process is running, ");
				}

				timerIsr();
			}

/****Checking for IO interrupts*****/
			//check if there has been an IO interrupt, if so call appropriate ioISR
			if (checkIOInterrupt(device1) == 1) {
				if (currProcess/*PCBGetState(currProcess) != blocked && PCBGetState(currProcess) != terminated*/) {
					printf("I/O 1 Completion interrupt: PID %d is running, ", PCBGetID(currProcess));
				} else {
					printf("I/O 1 Completion interrupt: no current process is running, ");
				}
				//call the IO service routine
				IO_ISR(1);
			}

			if (checkIOInterrupt(device2) == 1) {
				if (currProcess/*PCBGetState(currProcess) != blocked && PCBGetState(currProcess) != terminated*/) {
					printf("I/O 1 Completion interrupt: PID %d is running, ", PCBGetID(currProcess));
				} else {
					printf("I/O 1 Completion interrupt: no current process is running.");
				}
				//call the IO service routine
				IO_ISR(2);
			}

			//check the current process's PC, if it is MAX_PC, set to 0 and increment TERM_COUNT
			if (currProcess/*PCBGetState(currProcess) != blocked && PCBGetState(currProcess) != terminated*/
					&& sysStackPC >= PCBGetMaxPC(currProcess)) {
				PCBSetTermCount(currProcess, PCBGetTermCount(currProcess) + 1);
				printf("\r\n");
				//if TERM_COUNT = TERMINATE, then call terminateISR to put this process in the terminated list
				if (PCBGetTermCount(currProcess) == PCBGetTerminate(currProcess)) {

					terminateIsr();
					continue;	//currProcess has been terminated, we don't want to execute the rest of the loop, instead jump to next iteration
				}
				//PCBSetPC(currProcess, 0);
				sysStackPC = 0;
			}

			sysStackPC++;

/***Checking traps*****/
			if (currProcess/*PCBGetState(currProcess) != blocked && PCBGetState(currProcess) != terminated*/
					&& checkIORequest(1) != 0) {
				printf("I/O trap request: I/O device 1, ");
				IOTrapHandler(device1);
			}

			if (currProcess/*PCBGetState(currProcess) != blocked && PCBGetState(currProcess) != terminated*/
					&& checkIORequest(2) != 0) {
				printf("I/O trap request: I/O device 2, ");
				IOTrapHandler(device2);
			}


			//at end
			simCounter++;
		}
}

int main(void) {
	srand(time(NULL));
	currPID = 0;
	sysStackPC = 0;
	/*io1Count = -1;
	io2Count = -1;*/
	timerCount = TIMER_QUANTUM;
	newProcesses = fifoQueueConstructor();
	readyProcesses = fifoQueueConstructor();
	terminatedProcesses = fifoQueueConstructor();
	device1 = IODeviceConstructor();
	device2 = IODeviceConstructor();

	printf("Sean Markus\r\nWing-Sea Poon\r\nAbigail Smith\r\nTabi Stein\r\n\r\n");

	//An initial process to start with
	currProcess = PCBConstructor();
	if(currProcess != NULL)	// Remember to call the destructor when finished using newProc
	{
		PCBSetID(currProcess, currPID);
		PCBSetPriority(currProcess, rand() % PRIORITY_LEVELS);
		PCBSetState(currProcess, running);
		//fifoQueueEnqueue(newProcesses, currProcess);
		printf("Process created: PID: %d at %lu\r\n", PCBGetID(currProcess), PCBGetCreation(currProcess));
//		printf("Process created: %s\r\n", PCBToString(currProcess));
		cpu();
	}

	//free all the things!
	fifoQueueDestructor(&newProcesses);
	fifoQueueDestructor(&readyProcesses);
	fifoQueueDestructor(&terminatedProcesses);

	IODeviceDestructor(device1);
	IODeviceDestructor(device2);

	printf("End of simulation\r\n");
	return 0;
}
