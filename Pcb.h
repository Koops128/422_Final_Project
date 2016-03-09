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

//IO DEFINES
#define NUM_IO_TRAPS 			4


//PRODUCER CONSUMER DEFINES
#define PC_LOCK_UNLOCK 			2
#define PC_WAIT 				1
#define PC_SIGNAL 				1
#define MAX_SHARED_SIZE 		5

//SHARED RESOURCE DEFINES
#define SR_LOCK_UNLOCK 			2
#define NUM_MUTEX_STEPS 		4

typedef enum {
	created=0,
	running=1,
	ready=2,
	interrupted=3,
	blocked=4,
	terminated=5
} State;

//TODO combine following
//enum pairType {
//	NONE, MUTREC
//};

typedef enum {
	none,	producer,	consumer,	mutrecA,	mutrecB
} RelationshipType;

typedef struct PCB* PcbPtr;

/*********************************************************************************/
/*                           CONSTRUCTOR, DESTRUCTOR                             */
/*********************************************************************************/
PcbPtr PCBAllocateSpace();
PcbPtr PCBConstructor(PcbPtr pcb, RelationshipType theType, PcbPtr partner);
void PCBDestructor(PcbPtr pcb);

/*********************************************************************************/
/*                          	 Mutex Related			                         */
/*********************************************************************************/
void PCBSetMutexLockSteps(PcbPtr pcb, int mutexNum, unsigned int theSteps[NUM_MUTEX_STEPS]);
void PCBSetMutexUnlockSteps(PcbPtr pcb, int mutexNum, unsigned int theSteps[NUM_MUTEX_STEPS]);
int isMutexLockStep(PcbPtr pcb, int mutexNum, unsigned int theStep);
int isMutexUnlockStep(PcbPtr pcb, int mutexNum, unsigned int theStep);
void PCBSetMutexIndex(PcbPtr pcb, int mutexNum, int index);
int PCBGetMutexIndex(PcbPtr pcb, int mutexNum);



/*********************************************************************************/
/*                          	 I/O Related			                         */
/*********************************************************************************/
void initializeTrapArray(PcbPtr pcb);
unsigned int PCBGetIO1Trap(PcbPtr pcb, int index);
unsigned int PCBGetIO2Trap(PcbPtr pcb, int index);

/*Returns a string representation of this PCB.*/
char *PCBToString(PcbPtr pcb);
/*Returns a string value for the given state.*/
char* StateToString(State state);


void PCBSetPriority(PcbPtr pcb, int priority);
void PCBSetID(PcbPtr pcb, int id);
void PCBSetState(PcbPtr pcb, State newState);
void PCBSetPC(PcbPtr pcb, unsigned int newPC);

unsigned int PCBGetPC(PcbPtr pcb);
int PCBGetPriority(PcbPtr pcb);
int PCBGetID(PcbPtr pcb);
State PCBGetState(PcbPtr pcb);
unsigned int PCBGetMaxPC(PcbPtr pcb);
unsigned long PCBGetCreation(PcbPtr pcb);

/*********************************************************************************/
/*                          Termination Related			                         */
/*********************************************************************************/
void PCBSetTermination(PcbPtr pcb, unsigned long newTermination);
void PCBSetTerminate(PcbPtr pcb, int newTerminate);
void PCBSetTermCount(PcbPtr pcb, unsigned int newTermCount);

unsigned long PCBGetTermination(PcbPtr pcb);
int PCBGetTerminate(PcbPtr pcb);
unsigned int PCBGetTermCount(PcbPtr pcb);


/*********************************************************************************/
/*                         Relationship Related			                         */
/*********************************************************************************/
typedef struct mutRecPairSteps* MRStepsPtr;
typedef struct prodConsPairSteps* PCStepsPtr;
typedef struct MRData* MRDataPtr;
typedef struct PCData* PCDataPtr;

RelationshipType PCBgetPairType(PcbPtr pcb);

/*Contains the steps arrays specific to a mutual resource user
 *for locking/unlocking each of its two mutexes.*/
typedef struct mutRecPairSteps {
	unsigned int lock1[NUM_MUTEX_STEPS];
	unsigned int unlock1[NUM_MUTEX_STEPS];
	unsigned int lock2[NUM_MUTEX_STEPS];
	unsigned int unlock2[NUM_MUTEX_STEPS];
} MutRecStepsStr;

typedef struct prodConsPairSteps {
	unsigned int lock[PC_LOCK_UNLOCK];
	unsigned int unlock[PC_LOCK_UNLOCK];
	unsigned int signal[PC_SIGNAL];
	unsigned int wait[PC_WAIT];

	//TODO -- I think we need these as well, for cond var 2?
	//	unsigned int signal2[PC_SIGNAL];
	//	unsigned int wait2[PC_WAIT];
} ProdConsStepsStr;

typedef struct Relationship {
	RelationshipType mType;
	PcbPtr mPartner;
	union {
		MRStepsPtr mrSteps;
		PCStepsPtr pcSteps;
	};
}RelationshipStr;

/*Special data for a Mutual Resource PCB*/
typedef struct MRData{
	/*mutex1: Index into CPU's mutex array for the
	 *first mutex this PCB tries to get a lock on.*/
	int mutex1;
	int mutex2;
	//MutexPtr mutex1;
	//MutexPtr mutex2;
} MRDataStr;

typedef struct PCData{
	int mutex;
	//MutexPtr mutex;
	//condition var 1
	//condition var 2
} PCDataStr;




#endif /* PCB_H_ */
