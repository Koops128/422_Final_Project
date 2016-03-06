/***********************************************************************************************
* IoPcb.h
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
* This header file defines the class and methods for the process control block implementation
* with I/O services.
*
************************************************************************************************/

#ifndef IO_PCB_H
#define IO_PCB_H

#include "Pcb.h"

#define NUM_IO_TRAPS 4

////////////////////////////////////////////////////////////////////////////////////
//                               STRUCT DEFINITIONS                               //
////////////////////////////////////////////////////////////////////////////////////

typedef struct IoPcb* IoPcbPtr;

//declaration of pointers to functions
typedef void 			(*fptrGetIO1Trap)	(IoPcbPtr, int);
typedef void 			(*fptrGetIO2Trap)	(IoPcbPtr, int);

// IoPcb struct
typedef struct IoPcb
{
	// data
	 PcbPtr baseObjPtr;
	 unsigned int IO_1_Traps[NUM_IO_TRAPS];
	 unsigned int IO_2_Traps[NUM_IO_TRAPS];
	
	// functions
	fptrGetIO1Trap 		getIO1Trap;
	fptrGetIO2Trap		getIO2Trap;
} IoPcbStr;
typedef IoPcbStr* IoPcbPtr;

////////////////////////////////////////////////////////////////////////////////////
//                            CONSTUCTOR, DESTRUCTOR                              //
////////////////////////////////////////////////////////////////////////////////////

PcbPtr IoPCBConstructor();
void IoPCBDestructor(IoPcbPtr pcb);

////////////////////////////////////////////////////////////////////////////////////
//                                 GETTERS                                        //
////////////////////////////////////////////////////////////////////////////////////

unsigned int IoPCBGetIO1Trap(IoPcbPtr pcb, int index);
unsigned int IoPCBGetIO2Trap(IoPcbPtr pcb, int index);

////////////////////////////////////////////////////////////////////////////////////
//                                 TO STRING                                      //
////////////////////////////////////////////////////////////////////////////////////

char* IoPCBToString(IoPcbPtr pcb);

#endif
