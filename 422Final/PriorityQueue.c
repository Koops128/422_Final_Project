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
		FifoQueue * fq = fifoQueueConstructor();   //MODIFIED
		pq->priorityArray[i] = fq;
		//pq->priorityArray[i] = NULL;
	}

	return pq;
}

//TODO accept double pointer
void pqDestructor(PQPtr this)
{
	int i;
	for(i = 0; i < PRIORITY_LEVELS; i++)
	{
		if(this->priorityArray[i] != NULL)
		{
			fifoQueueDestructor(&((this->priorityArray)[i]));
		}
	}
	free(this);
	this = NULL;
}

void pqEnqueue(PQPtr this, PcbPtr pcb)
{
	fifoQueueEnqueue(this->priorityArray[PCBGetPriority(pcb)], pcb);
}

PcbPtr pqDequeue(PQPtr this)
{
	int i;
	PcbPtr retval = NULL;
	for(i = 0; i < PRIORITY_LEVELS; i++)
	{
		//++++++++++++MODIFIED START++++++++++++
	    	PcbPtr fifoFirst = fifoQueuePeek(this->priorityArray[i]);
		//if(this->priorityArray[i] != NULL)
		if (fifoFirst != NULL)
            	//++++++++++++MODIFIED END++++++++++++
		{
			retval = fifoQueueDequeue(this->priorityArray[i]);
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
	    	PcbPtr fifoFirst = fifoQueuePeek(this->priorityArray[i]);
		//if(this->priorityArray[i] != NULL)
		if (fifoFirst != NULL)
            	//++++++++++++MODIFIED END++++++++++++
		{
			//return this->priorityArray[i];
			return fifoQueuePeek(this->priorityArray[i]);
		}
	}
	return NULL;
}

bool pqIsEmpty(PQPtr this)
{
	int i;
	for(i = 0; i < PRIORITY_LEVELS; i++) {
        PcbPtr fifoFirst = fifoQueuePeek(this->priorityArray[i]);
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
	//int bufferSize = 0;

	//int qLabelLength = 10;
	char * qLabel = (char*) calloc (10, sizeof(char));

	int i;
	for(i = 0; i < PRIORITY_LEVELS; i++)
	{
		sprintf(qLabel, "Q%d: ", i);
		int labelLen = strlen(qLabel);
		strncat(result, qLabel, labelLen);
		//free(qLabel);

		PcbPtr fifoFirst = fifoQueuePeek(this->priorityArray[i]);
		if (fifoFirst != NULL)
		{
			char* fifoString = fifoQueueToString(this->priorityArray[i]);
			int fifoLength = strlen(fifoString);
			//int length = qLabelLength + fifoLength + 1; // +1 for \n at the end

			strncat(result, fifoString, fifoLength);
		}
		//strncat(result, "\n", 1);
		strncat(result, "*\n", 2);
	}
	free(qLabel);

	return result;
}
