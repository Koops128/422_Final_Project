/***********************************************************************************************
* Mutex.h
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
* This header file defines the class and methods for the mutex implementation.
*
************************************************************************************************/

#ifndef MUTEX_H
#define MUTEX_H

#include "Pcb.h"
#include "Fifo.h"


/*********************************************************************************/
/*                                 MUTEX STRUCT                                  */
/*********************************************************************************/

typedef struct Mutex
{
	int id;
	PcbPtr owner;
	FifoQueue* waitQ;
} MutexStr;
typedef MutexStr* MutexPtr;

/*********************************************************************************/
/*                           CONSTRUCTOR, DESTRUCTOR                             */
/*********************************************************************************/

MutexPtr MutexConstructor(int id);
void MutexDestructor(MutexPtr* mutex);

/*********************************************************************************/
/*                               MUTEX FUNCTIONALITY                             */
/*********************************************************************************/

/** Returns 1 if successful, 0 if put into the waiting queue.*/
int MutexLock(MutexPtr mutex, PcbPtr pcb);
void MutexUnlock(MutexPtr mutex, PcbPtr pcb);
int MutexHasWaiting(MutexPtr mutex);

#endif
