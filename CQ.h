/*
 * CQ.h
 *
 *  Created on: Mar 9, 2016
 *      Author: Abigail
 */

#ifndef CQ_H_
#define CQ_H_


typedef struct circQ* cQPtr;

cQPtr makeCQ(int size);

int bufFull(cQPtr q);

int bufEmpty(cQPtr q);

void pushCQ(cQPtr q, int val);

int popCQ(cQPtr q, int* storage);

#endif /* CQ_H_ */
