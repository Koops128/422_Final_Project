/*
 * CQ.c
 *
 *  Created on: Mar 5, 2016
 *      Author: Tabi
 */

#include <stdlib.h>
#include <stdio.h>
#include "CQ.h"

struct circQ {
	int bufavail;
	int max;
	int hd;
	int tl;
	/*A pointer to a sequence length "max" of ints.*/
	int* buffer; //will do pcb though
};

cQPtr makeCQ(int size) {
	cQPtr q = malloc(sizeof(cQPtr));
	q->buffer = malloc(size * sizeof(int));
	q->max = size;
	q->bufavail = size;
	q->hd = q->tl = 0;
	return q;
}

int bufFull(cQPtr q) {
	return q->bufavail == 0;
}

int bufEmpty(cQPtr q) {
	return q->bufavail == q->max;
}

void pushCQ(cQPtr q, int val) {
	if (!bufFull(q)) {
		(q->buffer)[q->tl % q->max] = val;
		q->tl++;
		q->bufavail--;
	}
}

/*Puts popped value into storage upon success. Success is indicated by return value, which is
 * < 0 upon failure, 0 upon success.*/
int popCQ(cQPtr q, int* storage) {
	int success = !bufEmpty(q);
	if (success) {
		*storage = (q->buffer)[q->hd % q->max];
		printf("in pop: popped %d\n", *storage);
		q->hd++;
		q->bufavail++;
	}
	return success;
}

/*Tests*/
//int main(void) {
//	cQPtr q = makeCQ(4);
//	int i;
//	for(i=0; i < 6; i++) {
//		pushCQ(q, i);
//	}
//	for(i=0; i < 4; i++) {
//		printf(" %d ", (q->buffer)[i]);
//	}
//	printf("\n");
//	printf("Buf full? %d\n", bufFull(q));
//	printf("Buf empty? %d\n", bufEmpty(q));
//
//	int j = 0;
//	for(i=0; i < 2; i++) {
//		popCQ(q, &j);
//		printf("%d popped.\n", j);
//	}
//	printf("Buf avail: %d\n", q->bufavail);
//	return 0;
//}
