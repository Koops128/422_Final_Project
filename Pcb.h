/***********************************************************************************************
* Pcb.h
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
* This header file defines the class and methods for the process control block implementation.
*
************************************************************************************************/

#ifndef PCB_H_
#define PCB_H_

/***********************************************************************************/
/*                                      STATE                                      */
/***********************************************************************************/

typedef enum {
	created=0,
	running=1,
	ready=2,
	interrupted=3,
	blocked=4,
	terminated=5
} State;

char* StateToString(State state);

/***********************************************************************************/
/*                                      PCB                                        */
/***********************************************************************************/

typedef struct PCB* PcbPtr;

//declaration of pointers to functions
typedef void 			(*fptrDestructor)	(PcbPtr);

typedef	void			(*fptrSetID) 		(PcbPtr, int);
typedef	void 			(*fptrSetPriority)	(PcbPtr, int);
typedef	void 			(*fptrSetState)		(PcbPtr, State);
typedef	void 			(*fptrSetPC)		(PcbPtr, unsigned int);
typedef	void 			(*fptrSetTermination)(PcbPtr, unsigned long);
typedef	void 			(*fptrSetTerminate)	(PcbPtr, int);
typedef	void 			(*fptrSetTermCount)	(PcbPtr, unsigned int);
	
typedef	int 			(*fptrGetID)		(PcbPtr);
typedef	int 			(*fptrGetPriority)	(PcbPtr);
typedef	State 			(*fptrGetState)		(PcbPtr);
typedef	unsigned int 	(*fptrGetPC)		(PcbPtr);
typedef	unsigned int 	(*fptrGetMaxPC)		(PcbPtr);
typedef	unsigned long 	(*fptrGetCreation)	(PcbPtr);
typedef	unsigned long 	(*fptrGetTermination)(PcbPtr);
typedef	int 			(*fptrGetTerminate)	(PcbPtr);
typedef	unsigned int 	(*fptrGetTermCount)	(PcbPtr);

typedef char* 			(*fptrToString)		(PcbPtr);

// PCB struct
typedef struct PCB {
	// data
	 void* derivedObjectPtr;
	 int PID;
	 int priority;
	 State state;
	 unsigned int PC;
	 unsigned int maxPC;
	 unsigned long int creation;
	 unsigned long int termination;
	 unsigned int terminate;
	 unsigned int term_count;
	
	//functions
	fptrDestructor 		destructor;
	fptrSetID			setID;
	fptrSetPriority		setPriority;
	fptrSetState		setState;
	fptrSetPC			setPC;
	fptrSetTermination 	setTermination;
	fptrSetTerminate	setTerminate;
	fptrSetTermCount	setTermCount;

	fptrGetID			getID;
	fptrGetPriority		getPriority;
	fptrGetState		getState;
	fptrGetPC			getPC;
	fptrGetMaxPC		getMaxPC;
	fptrGetCreation		getCreation;
	fptrGetTermination	getTermination;
	fptrGetTerminate	getTerminate;
	fptrGetTermCount	getTermCount;
	
	fptrToString		toString;
} PcbStr;
typedef struct PCB* PcbPtr;

////////////////////////////////////////////////////////////////////////////////////
//                            CONSTUCTOR, DESTRUCTOR                              //
////////////////////////////////////////////////////////////////////////////////////

PcbPtr PCBConstructor();
void PCBDestructor(PcbPtr pcb);

////////////////////////////////////////////////////////////////////////////////////
//                                 SETTERS                                        //
////////////////////////////////////////////////////////////////////////////////////

void PCBSetID(PcbPtr pcb, int id);
void PCBSetPriority(PcbPtr pcb, int priority);
void PCBSetState(PcbPtr pcb, State newState);
void PCBSetPC(PcbPtr pcb, unsigned int newPC);
//void PCBSetMaxPC(PcbPtr pcb, unsigned int newMaxPC);
//void PCBSetCreation(PcbPtr pcb, unsigned int newCreation);
void PCBSetTermination(PcbPtr pcb, unsigned long newTermination);
void PCBSetTerminate(PcbPtr pcb, int newTerminate);
void PCBSetTermCount(PcbPtr pcb, unsigned int newTermCount);

////////////////////////////////////////////////////////////////////////////////////
//                                 GETTERS                                        //
////////////////////////////////////////////////////////////////////////////////////

int PCBGetID(PcbPtr pcb);
int PCBGetPriority(PcbPtr pcb);
State PCBGetState(PcbPtr pcb);
unsigned int PCBGetPC(PcbPtr pcb);
unsigned int PCBGetMaxPC(PcbPtr pcb);
unsigned long PCBGetCreation(PcbPtr pcb);
unsigned long PCBGetTermination(PcbPtr pcb);
int PCBGetTerminate(PcbPtr pcb);
unsigned int PCBGetTermCount(PcbPtr pcb);

////////////////////////////////////////////////////////////////////////////////////
//                                 TO STRING                                      //
////////////////////////////////////////////////////////////////////////////////////

char *PCBToString(PcbPtr pcb);

#endif /* PCB_H_ */
