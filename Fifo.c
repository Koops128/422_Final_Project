/***********************************************************************************************
* Fifo.c
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
* This C file implements the class and methods for the FIFO Queue.
*
************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "Fifo.h"
#include "Pcb.h"
#include <string.h>

Node* nodeConstructor() {
	Node * newNode = (Node*) malloc(sizeof(Node));
	newNode->content = NULL;
	newNode->next = NULL;
	return newNode;
}

/*Accepts a double pointer to a Node, so the caller's Node pointer can be set to NULL.
 *Assumes that the caller will handle deallocating the contents of the Node, in case
 *there is still work to do with the contents.*/
PcbPtr nodeDestructor(Node ** node) {
	PcbPtr contents = (*node)->content;
	free (*node);
	*node = NULL; //Sets dangling pointer in calling function to NULL

	return contents;
}

void nodeSetNext(Node * node, Node * nextNode) {
	node->next = nextNode;
}

FifoQueue * fifoQueueConstructor() {
	FifoQueue* newQueue = (FifoQueue*) malloc(sizeof(FifoQueue));
	newQueue->head = NULL;
	newQueue->back = NULL;
	newQueue->size = 0;
	return newQueue;
}

void fifoQueueDestructor(FifoQueue ** queue_p) {

	//printf("fifoQueueDestructor queue size: %d\n", (queue_p)->size);


	if ((*queue_p)->head != NULL) {
	//if ((*queue_p)->size != 0) {
		if ((*queue_p)->head->content == (*queue_p)->back->content) {
			//Pass a pointer to the head Node in the queue that queue_p points to:
			PcbPtr toDestroy = nodeDestructor(&((*queue_p)->head));
			PCBDestructor(toDestroy);

		}else {
			Node * curr = (*queue_p)->head;
			Node * curr2 = (*queue_p)->head->next;
			while (curr2 != (*queue_p)->back) {
				PcbPtr toDestroy = nodeDestructor(&curr);
				PCBDestructor(toDestroy);
				curr = curr2;
				curr2 = curr2->next;
			}
			PcbPtr toDestroy = nodeDestructor(&curr);
			PCBDestructor(toDestroy);
			toDestroy = nodeDestructor(&curr2);
			PCBDestructor(toDestroy);
		}
		(*queue_p)->size = 0;
		free(*queue_p);
		*queue_p = NULL; //Only local
	}

}

void fifoQueueEnqueue(FifoQueue *queue, PcbPtr pcb) {
	if (pcb == NULL) {
		printf("pcb is null");
		return;
	}

	Node * newNode = nodeConstructor();
	newNode->content = pcb;

	if (queue->head == NULL) {
		queue->head = newNode;
		queue->back = newNode;
	}
	else if (queue->head == queue->back) {
		nodeSetNext(queue->head, newNode);
		queue->back = newNode;
	}
	else {
		nodeSetNext(queue->back, newNode);
		queue->back = queue->back->next;
	}

	(queue->size)++;
}


PcbPtr fifoQueueDequeue(FifoQueue *queue) {
	if (queue->head == NULL) {
		printf("Queue is empty");
		return NULL;
	}
	//PcbStr * ret = queue->head->content; //This ends up getting destroyed when destructing the Node holding it, so we instead make a copy.

	Node * toDestroy = queue->head;
	queue->head = queue->head->next; //Sets head to null when head points to the last Node
	PcbPtr ret = nodeDestructor(&toDestroy);
	(queue->size)--;
	return ret;
}


PcbPtr fifoQueuePeek(FifoQueue * queue) {
	if (queue->head == NULL) {
		return NULL;
	}
	return queue->head->content;
}

int fifoQueueIsEmpty(FifoQueue * queue) {
	return (queue->head == NULL);
}

/*Returns the number of bytes needed for a fifoQueue.*/
int fifoQueueSize() {
	return sizeof(FifoQueue);
}

char * fifoQueueToString(FifoQueue * queue) {
	char * string = malloc(sizeof(char) * (queue->size * 4 + 500));
	string[0]='\0';
	strncat(string, "Q: ", 5);

	if (queue->size == 0) {
		strncat(string, "Empty", 6);
		return string;
	}

	int i;
	Node *front = queue->head;
	for (i = 1; i <= queue->size; i++) {
		char s[5] = "P";
		int j = PCBGetID(front->content);
		int numWritten = sprintf(s + 1, "%d", j);
		s[numWritten + 1] = '-';
		strncat(string, s, 5);
		front = front->next;
	}

	/*
	strncat(string, "* : contents: ", 15);
	PcbStr *curr = queue->back;
	strncat(string, toString(curr), 100);
	*/
	return string;
}


