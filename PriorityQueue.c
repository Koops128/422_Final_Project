/*
 * Priority_Queue.c
 *
 *
 *  Created on: Jan 9, 2016
 *      Author: Wing-Sea Poon
 */

#include <stdbool.h>
#include <string.h>

#include "PriorityQueue.h"
#include "Fifo.h"
#include "Pcb.h"

#include <stdlib.h>
#include <stdio.h>

PQPtr pqConstructor()
{
	PQPtr pq = (PQPtr) malloc(sizeof(PQStr));
	if(pq == NULL)
	{
		return NULL;
	}
	int i;
	for(i = 0; i < PRIORITY_LEVELS; i++)
	{
		fQ * fq = createfQ();   //MODIFIED
		pq->priorityArray[i] = fq;
		//pq->priorityArray[i] = NULL;
	}

	return pq;
}

void pqDestructor(PQPtr this)
{
	int i;
	for(i = 0; i < PRIORITY_LEVELS; i++)
	{
		if(this->priorityArray[i] != NULL)
		{
			fQDestructor(this->priorityArray[i]);
		}
	}
	free(this);
	this = NULL;
}

void pqEnqueue(PQPtr this, PcbPtr pcb)
{
	fifoEnqueue(this->priorityArray[pcb->priority], pcb);
}

PcbPtr pqDequeue(PQPtr this)
{
	int i;
	PcbPtr retval = NULL;
	for(i = 0; i < PRIORITY_LEVELS; i++)
	{
		//++++++++++++MODIFIED START++++++++++++
	    	PcbStr *fifoFirst = fifoPeek(this->priorityArray[i]);
		//if(this->priorityArray[i] != NULL)
		if (fifoFirst != NULL)
            	//++++++++++++MODIFIED END++++++++++++
		{
			retval = fifoDequeue(this->priorityArray[i]);
			break;
		}
	}

	return retval;
}

PcbPtr pqPeek(PQPtr this)
{
	int i;
	for(i = 0; i < PRIORITY_LEVELS; i++)
	{
		//if(this->priorityArray[i] != NULL)
		//++++++++++++MODIFIED START++++++++++++
	    	PcbStr *fifoFirst = fifoPeek(this->priorityArray[i]);
		//if(this->priorityArray[i] != NULL)
		if (fifoFirst != NULL)
            	//++++++++++++MODIFIED END++++++++++++
		{
			//return this->priorityArray[i];
			return fifoPeek(this->priorityArray[i]);
		}
	}
	return NULL;
}

bool pqIsEmpty(PQPtr this)
{
	int i;
	for(i = 0; i < PRIORITY_LEVELS; i++) {
        PcbStr *fifoFirst = fifoPeek(this->priorityArray[i]);
		if (fifoFirst != NULL)
		{
			if(this->priorityArray[i] != NULL)
			{
				return false;
			}
		}
	}
	return true;
}
char* pqToString(PQPtr this)
{
	char* result = (char*) calloc(1000, sizeof(char));
	int bufferSize = 0;

	int qLabelLength = 10;
	char * qLabel = (char*) calloc (10, sizeof(char));

	int i;
	for(i = 0; i < PRIORITY_LEVELS; i++)
	{
		sprintf(qLabel, "Q%d: ", i);
		int labelLen = strlen(qLabel);
		strncat(result, qLabel, labelLen);
		//free(qLabel);

		PcbStr *fifoFirst = fifoPeek(this->priorityArray[i]);
		if (fifoFirst != NULL)
		{
			char* fifoString = fifoToString(this->priorityArray[i]);
			int fifoLength = strlen(fifoString);
			int length = qLabelLength + fifoLength + 1; // +1 for \n at the end

			strncat(result, fifoString, fifoLength);
		}
		//strncat(result, "\n", 1);
		strncat(result, "*\n", 2);
	}
	free(qLabel);

	return result;
}



/*
char* pqToString(PQPtr this)
{
	char* result = NULL;
	int bufferSize = 0;
	
	int qLabelLength = 4;
	char qLabel [qLabelLength];
	qLabel[0] = 'Q';
	// qLabel[1] will be the index i in the following loop
	qLabel[2] = ':';
	qLabel[3] = ' ';
	
	int i;
	for(i = 0; i < PRIORITY_LEVELS; i++)
	{
		qLabel[1] = i;
		
		if(this->priorityArray[i] != NULL)
		{
			char* fifoString = fifoToString(this->priorityArray[i]);
			int fifoLength = strlen(fifoString);
			int length = qLabelLength + fifoLength + 1; // +1 for \n at the end
			
			if(result == NULL)
			{
				bufferSize = length + 1;	// +1 for \0 at the end
				result = (char*) malloc(bufferSize);
				strncpy(result, qLabel, qLabelLength);
			}
			else
			{
				bufferSize += length;
				result = (char*) realloc(result, bufferSize);
				strncat(result, qLabel, qLabelLength);
			}
			
			strncat(result, fifoString, fifoLength);
			strncat(result, "\n", 1);
		}
	}
	
	return result;
}
*/
