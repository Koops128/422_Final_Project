/***********************************************************************************************
* IoPcb.c
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
* This C file implements the class and methods for the process control block with I/O services.
*
************************************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include "Pcb.h"
#include "IoPcb.h"

////////////////////////////////////////////////////////////////////////////////////
//                            CONSTUCTOR, DESTRUCTOR                              //
////////////////////////////////////////////////////////////////////////////////////

/*
 * Partitions (maxVal - minVal) into n non-overlapping partitions.
 * Sets storage[i] to a random number from the corresponding partition.
 * 
 * Ex.: n = 8, minVal = 0, maxVal = 2000
 * Partition Size = 250
 * partition[0] = 0 to 249
 * partition[1] = 250 to 499
 * ...
 * partition[7] = 1750 to 1999
 */
void genTraps(int n, unsigned int* storage, int minVal, int maxVal) {
	int partitionSize = (maxVal - minVal) / n;	// truncate if the division results in a double
	int i;
	for(i = 0; i < n; i++) {
		storage[i] = (rand() % (partitionSize)) + (i * partitionSize);
	}
}

PcbPtr IoPCBConstructor()
{
	IoPcbPtr ioPcbObj;
	PcbPtr pcbObj = PCBConstructor();
	
	// Allocate memory and set super
	ioPcbObj = (IoPcbPtr) malloc(sizeof(IoPcbStr));
	if(ioPcbObj == NULL)
	{
		pcbObj->destructor(pcbObj);
		return NULL;
	}
	pcbObj->derivedObjectPtr = ioPcbObj;
	
	// Set IoPcb-specific data
	unsigned int* allTraps = malloc(sizeof(unsigned int) * NUM_IO_TRAPS * 2);
	genTraps(NUM_IO_TRAPS * 2, allTraps, 0, pcbObj->maxPC);

	int i;
	for (i = 0; i < NUM_IO_TRAPS; i++) {
		ioPcbObj->IO_1_Traps[i] = allTraps[i*2];		//grab even indices
		ioPcbObj->IO_2_Traps[i] = allTraps[(i*2) + 1]; //grab odd indices
	}

	free(allTraps);
	
	// Set IoPcb-specific functions
	pcbObj->destructor = IoPCBDestructor;
	pcbObj->toString = IoPCBToString;
	ioPcbObj->getIO1Trap = IoPCBGetIO1Trap;
	ioPcbObj->getIO2Trap = IoPCBGetIO2Trap;
	
	return pcbObj;
}

void IoPCBDestructor(IoPcbPtr pcb) {
	free(pcb->baseObjPtr);
	free(pcb);
	pcb = NULL;	//Only locally sets the pointer to null
}

////////////////////////////////////////////////////////////////////////////////////
//                                 GETTERS                                        //
////////////////////////////////////////////////////////////////////////////////////

unsigned int IoPCBGetIO1Trap(IoPcbPtr pcb, int index) {
	if (index < NUM_IO_TRAPS) {
		return pcb->IO_1_Traps[index];
	} else {
		return -1;
	}
}

unsigned int IoPCBGetIO2Trap(IoPcbPtr pcb, int index) {
	if (index < NUM_IO_TRAPS) {
		return pcb->IO_2_Traps[index];
	} else {
		return -1;
	}
}

////////////////////////////////////////////////////////////////////////////////////
//                                 TO STRING                                      //
////////////////////////////////////////////////////////////////////////////////////

/**Need at most 5 chars for each 8 traps, plus 8 spaces before each, or 48*/
char* TrapsToString(IoPcbPtr pcb) {
	char* arrStr = (char*) malloc(sizeof(char) * (48 + 10 + 1));
	arrStr[0] = '\0';

	char dev1[sizeof(char) * (20 + 4 + 1)];
	dev1[0] = '\0';
	char dev2[sizeof(char) * (20 + 4 + 1)];
	dev2[0] = '\0';

	int i;
	for (i = 0; i < NUM_IO_TRAPS; i++) {
		char buffer[sizeof(char) * (5+2)]; //5 is max number of digits in unsigned int, + null + space
		sprintf(buffer, " %d", pcb->IO_1_Traps[i]);
		strncat(dev1,buffer,7);
		char buffer2[sizeof(char) * (5+2)];
		sprintf(buffer2, " %d", pcb->IO_2_Traps[i]);
		strncat(dev2,buffer2,7);
	}
	sprintf(arrStr, "d1[%s ] d2[%s ]", dev1, dev2);
	return arrStr;
}

char* IoPCBToString(IoPcbPtr pcb) {
	if (pcb == NULL)
		return NULL;

	PcbPtr base = pcb->baseObjPtr;

	char * emptyStr = (char*) malloc(sizeof(char) * 1000);
	emptyStr[199] = '\0';
	char* stateString = StateToString(base->state);
	char* trapString = TrapsToString(pcb);
	int lenNeeded = sprintf(emptyStr, "ID: %d, Priority: %d, State: %s, PC: %d"
			", MAX_PC %d"
			", CREATION %lu"
			", TERMINATE %d"
			", TERM_COUNT %d"
			", \n	Traps: (%s)"
							,base->PID, base->priority, stateString, base->PC
							,base->maxPC
							,base->creation
							,base->terminate
							, base->term_count
							, trapString
							);
							
	base = NULL;
	free(stateString);
	free(trapString);
	char * retString = (char *) malloc(sizeof(char) * lenNeeded);
	sprintf(retString, "%s", emptyStr);
	free(emptyStr);
	return retString;
}
