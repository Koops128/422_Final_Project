/***********************************************************************************************
* Pcb.c
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
* This C file implements the class and methods for the process control block.
*
************************************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include "Pcb.h"


/***********************************************************************************/
/*                                      STATE                                      */
/***********************************************************************************/

const char* stateNames[] = {"Created","Running","Ready","Interrupted","Blocked","Terminated"};

char* StateToString(State state) {
	int len = strlen(stateNames[state]);
	char* string = malloc(sizeof(char) * len + 1);
	sprintf(string, "%s", stateNames[state]); //auto appends null at end
	return string;
}

/***********************************************************************************/
/*                                      PCB                                        */
/***********************************************************************************/

////////////////////////////////////////////////////////////////////////////////////
//                            CONSTUCTOR, DESTRUCTOR                              //
////////////////////////////////////////////////////////////////////////////////////

PcbPtr PCBConstructor(){
	PcbStr* pcb = (PcbStr*) malloc(sizeof(PcbStr));
	if(pcb == NULL)
	{
		return NULL;
	}
	
	// Set data
	pcb->derivedObjectPtr = pcb;	// PCB is base class; therefore, make derivedPtr point to self
	pcb->PID = 1;
	pcb->priority = 0;
	pcb->state = created;
	pcb->PC = 0;
	pcb->maxPC = 2000;
	pcb->creation = time(NULL);
	pcb->terminate = rand()%10;	//ranges from 0-9
	pcb->term_count = 0;
	
	// Set functions
	pcb->destructor = PCBDestructor;
	pcb->setID = PCBSetID;
	pcb->setPriority = PCBSetPriority;
	pcb->setState = PCBSetState;
	pcb->setPC = PCBSetPC;
	pcb->setTermination = PCBSetTermination;
	pcb->setTerminate = PCBSetTerminate;
	pcb->setTermCount = PCBSetTermCount;

	pcb->getID = PCBGetID;
	pcb->getPriority = PCBGetPriority;
	pcb->getState = PCBGetState;
	pcb->getPC = PCBGetPC;
	pcb->getMaxPC = PCBGetMaxPC;
	pcb->getCreation = PCBGetCreation;
	pcb->getTermination = PCBGetTermination;
	pcb->getTerminate = PCBGetTerminate;
	pcb->getTermCount = PCBGetTermCount;
	
	pcb->toString = PCBToString;

	return pcb;
}

void PCBDestructor(PcbPtr pcb) {
	free (pcb);
	pcb = NULL;	//Only locally sets the pointer to null
}

////////////////////////////////////////////////////////////////////////////////////
//                                 SETTERS                                        //
////////////////////////////////////////////////////////////////////////////////////

void PCBSetID(PcbStr* pcb, int id) {
	pcb->PID = id;
}

void PCBSetPriority(PcbStr* pcb, int priority) {
	pcb->priority = priority;

}

void PCBSetState(PcbStr* pcb, State newState) {
	pcb->state = newState;
}

void PCBSetPC(PcbStr* pcb, unsigned int newPC) {
	pcb->PC = newPC;
}

void PCBSetTermination(PcbStr* pcb, unsigned long newTermination) {
	pcb->termination = newTermination;
}

void PCBSetTerminate(PcbStr* pcb, int newTerminate) {
	pcb->terminate = newTerminate;
}

void PCBSetTermCount(PcbStr* pcb, unsigned int newTermCount) {
	pcb->term_count = newTermCount;
}

////////////////////////////////////////////////////////////////////////////////////
//                                 GETTERS                                        //
////////////////////////////////////////////////////////////////////////////////////

int PCBGetID(PcbStr* pcb) {
	return pcb->PID;
}

int PCBGetPriority(PcbStr* pcb) {
	return pcb->priority;
}

State PCBGetState(PcbStr* pcb) {
	return pcb->state;
}

unsigned int PCBGetPC(PcbStr* pcb) {
	return pcb->PC;
}

unsigned int PCBGetMaxPC(PcbStr* pcb) {
	return pcb->maxPC;
}

unsigned long PCBGetCreation(PcbStr* pcb) {
	return pcb->creation;
}

unsigned long PCBGetTermination(PcbStr* pcb) {
	return pcb->termination;
}

int PCBGetTerminate(PcbStr* pcb) {
	return pcb->terminate;
}

unsigned int PCBGetTermCount(PcbStr* pcb) {
	return pcb->term_count;
}

////////////////////////////////////////////////////////////////////////////////////
//                                 TO STRING                                      //
////////////////////////////////////////////////////////////////////////////////////

char *PCBToString(PcbStr* pcb) {
	if (pcb == NULL)
		return NULL;

	char * emptyStr = (char*) malloc(sizeof(char) * 1000);
	emptyStr[199] = '\0';
	char* stateString = StateToString(pcb->state);
	int lenNeeded = sprintf(emptyStr, "ID: %d, Priority: %d, State: %s, PC: %d"
			", MAX_PC %d"
			", CREATION %lu"
			", TERMINATE %d"
			", TERM_COUNT %d"
			", \n	Traps: (%s)"
							,pcb->PID, pcb->priority, stateString, pcb->PC
							,pcb->maxPC
							,pcb->creation
							,pcb->terminate
							, pcb->term_count
							);
	free(stateString);
	char * retString = (char *) malloc(sizeof(char) * lenNeeded);
	sprintf(retString, "%s", emptyStr);
	free(emptyStr);
	return retString;
}

////test pcb
//int main(void) {
//	srand(time(NULL));
//	PcbStr* pcb = PCBConstructor(0);
//	printf("%s", PCBToString(pcb));
//	return 0;
//}
