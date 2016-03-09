/***********************************************************************************************
* Mutex.c
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
* This C file implements the class and methods for mutex.
*
************************************************************************************************/

#include <stdlib.h>
#include <stdio.h>

#include "Mutex.h"
#include "Pcb.h"
#include "Fifo.h"


/*********************************************************************************/
/*                               PRIVATE FUNCTIONS                               */
/*********************************************************************************/

int hasOwner(MutexPtr mutex)
{
	return mutex->owner != NULL;
}

void printSuccessLock(int pidRequester, int mutexId)
{
	printf("PID %d: requested lock on mutex %d - succeeded\n", pidRequester, mutexId);
}

void printFailLock(int pidRequester, int mutexId, int pidBlocker)
{
	printf("PID %d: requested lock on mutex %d - blocked by PID %d\n", pidRequester, mutexId, pidBlocker);
}

void printSuccessUnlock(int pidReleaser, int mutexId)
{
	printf("PID %d: unlocked mutex %d - succeeded.\n", pidReleaser, mutexId);
}

void printSuccessUnlockNextInLine(int pidReleaser, int mutexId, int nextInLine)
{
	printf("PID %d: unlocked mutex %d - succeeded. New owner PID: %d\n", pidReleaser, mutexId, nextInLine);
}

void printFailUnlockWrongOwner(int pidReleaser, int mutexId, int trueOwner)
{
	printf("PID %d: tried to unlock mutex %d, which is owned by PID %d - failed\n", pidReleaser, mutexId, trueOwner);
}

void printFailUnlockNoOwner(int pidReleaser, int mutexId)
{
	printf("PID %d: tried to unlock mutex %d, which was never locked - failed\n", pidReleaser, mutexId);
}

/*********************************************************************************/
/*                           CONSTRUCTOR, DESTRUCTOR                             */
/*********************************************************************************/

MutexPtr MutexConstructor(int id)
{
	MutexPtr mutex = (MutexPtr) malloc(sizeof(MutexStr));
	if(mutex == NULL)
	{
		return NULL;
	}

	mutex->id = id;
	mutex->owner = NULL;
	mutex->waitQ = fifoQueueConstructor();

	return mutex;
}

void MutexDestructor(MutexPtr* mutexPtrPtr)
{
	fifoQueueDestructor(&((*mutexPtrPtr)->waitQ));
	PCBDestructor((*mutexPtrPtr)->owner);

	free(*mutexPtrPtr);
	*mutexPtrPtr = NULL;
}

/*********************************************************************************/
/*                               MUTEX FUNCTIONALITY                             */
/*********************************************************************************/

int MutexLock(MutexPtr mutex, PcbPtr pcb)
{
	if(!hasOwner(mutex))
	{
		printSuccessLock(PCBGetID(pcb), mutex->id);
		mutex->owner = pcb;
		return 1;
	}
	else
	{
		printFailLock(PCBGetID(pcb), mutex->id, PCBGetID(mutex->owner));
		fifoQueueEnqueue(mutex->waitQ, pcb);
		return 0;
	}
}

void MutexUnlock(MutexPtr mutex, PcbPtr pcb)
{
	if(hasOwner(mutex) && mutex->owner == pcb)
	{


		if(MutexHasWaiting(mutex))
		{
			printSuccessUnlockNextInLine(PCBGetID(pcb), mutex->id, PCBGetID(fifoQueuePeek(mutex->waitQ)));
			mutex->owner = fifoQueueDequeue(mutex->waitQ);
		}
		else
		{
			printSuccessUnlock(PCBGetID(pcb), mutex->id);
			mutex->owner = NULL;
		}
	}
	else if(hasOwner(mutex) && mutex->owner != pcb)
	{
		printFailUnlockWrongOwner(PCBGetID(pcb), mutex->id, PCBGetID(mutex->owner));
	}
	else
	{
		printFailUnlockNoOwner(PCBGetID(pcb), mutex->id);
	}
}

int MutexHasWaiting(MutexPtr mutex)
{
	return !fifoQueueIsEmpty(mutex->waitQ);
}
