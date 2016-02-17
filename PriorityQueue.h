// Author: Wing-Sea Poon

#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#include <stdbool.h>
#include "Fifo.h"
#include "Pcb.h"

#define PRIORITY_LEVELS 16

// Priority Queue ADT
typedef struct 
{
	fQ * priorityArray[PRIORITY_LEVELS];
} PQStr;
typedef PQStr* PQPtr;

// Allocates memory for a Priority Queue object.
PQPtr pqConstructor();

// Deallocates memory for a Priority Queue object.
void pqDestructor(PQPtr this);

// Adds the PCB to the end of a FIFO queue based on the PCB's priority.
void pqEnqueue(PQPtr this, PcbPtr pcb);

// Removes and returns the PCB with the highest priority.
PcbPtr pqDequeue(PQPtr this);

// Returns, without removing, the PCB with the highest priority.
PcbPtr pqPeek(PQPtr this);

// Checks whether there are any PCBs in this priority queue.
bool pqIsEmpty(PQPtr this);

// Returns a string representation of this priority queue.
char* pqToString(PQPtr this);

#endif

