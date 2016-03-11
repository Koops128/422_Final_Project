/***********************************************************************************************
* CondVar.h
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
* This header file defines the class and methods for the condition variable implementation.
*
************************************************************************************************/

#ifndef COND_VAR_H
#define COND_VAR_H

#include "Pcb.h"
#include "Mutex.h"

/*********************************************************************************/
/*                               COND_VAR STRUCT                                 */
/*********************************************************************************/

typedef struct CondVar
{
	int id;
	PcbPtr pcb;
	MutexPtr mutex;
} CondVarStr;
typedef CondVarStr* CondVarPtr;

/*********************************************************************************/
/*                           CONSTRUCTOR, DESTRUCTOR                             */
/*********************************************************************************/

CondVarPtr CondVarConstructor(int id);
void CondVarDestructor(CondVarPtr* condVarPtrPtr);

/*********************************************************************************/
/*                            COND_VAR FUNCTIONALITY                             */
/*********************************************************************************/

void CondVarWait(CondVarPtr var, MutexPtr mutex, PcbPtr pcb);
PcbPtr CondVarSignal(CondVarPtr var, PcbPtr pcb);

#endif
