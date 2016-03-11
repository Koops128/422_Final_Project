/***********************************************************************************************
* CondVar.c
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
* This C file implements the class and methods for condition variable.
*
************************************************************************************************/

#include <stdlib.h>
#include <stdio.h>

#include "CondVar.h"
#include "Pcb.h"
#include "Mutex.h"


/*********************************************************************************/
/*                               PRIVATE FUNCTIONS                               */
/*********************************************************************************/

void printWait(int pidRequester, int condId, int mutexId)
{
	printf("PID %d requested condition wait on cond %d with mutex %d\n", pidRequester, condId, mutexId);
}

void printSignal(int pidSignaller, int condId)
{
	printf("PID %d sent signal on cond %d\n", pidSignaller, condId);
}

/*********************************************************************************/
/*                           CONSTRUCTOR, DESTRUCTOR                             */
/*********************************************************************************/

CondVarPtr CondVarConstructor(int id)
{
	CondVarPtr condVar = (CondVarPtr) malloc(sizeof(CondVarStr));
	if(condVar == NULL)
	{
		return NULL;
	}

	condVar->id = id;
	condVar->pcb = NULL;
	condVar->mutex = NULL;

	return condVar;
}

void CondVarDestructor(CondVarPtr* condVarPtrPtr)
{
	PCBDestructor((*condVarPtrPtr)->pcb);
	MutexDestructor(&((*condVarPtrPtr)->mutex));

	free(*condVarPtrPtr);
	*condVarPtrPtr = NULL;
}

/*********************************************************************************/
/*                            COND_VAR FUNCTIONALITY                             */
/*********************************************************************************/

void CondVarWait(CondVarPtr var, MutexPtr mutex, PcbPtr pcb)
{
	printWait(PCBGetID(pcb), var->id, mutex->id);
	MutexUnlock(mutex, pcb);
	var->pcb = pcb;
	var->mutex = mutex;
}

//returns the other process in the pair that was waiting, but isn't now.
//If the other process wasn't waiting, then return NULL
PcbPtr CondVarSignal(CondVarPtr var, PcbPtr signaller)
{
	printSignal(PCBGetID(signaller), var->id);
	PcbPtr toReturn = var->pcb;
	//because the other process in the pair might not be waiting for the condVar when this pcb signals
	if (var->pcb) {
		MutexLock(var->mutex, var->pcb);
		var->pcb = NULL;
		var->mutex = NULL;
	}
	return toReturn;
}
