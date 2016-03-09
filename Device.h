/*
 * Device.h
 *
 *  Created on: Feb 19, 2016
 *      Author: Abigail
 */

#ifndef DEVICE_H_
#define DEVICE_H_
#include "Fifo.h"

typedef struct {
	FifoQueue* waitQ;
	int counter;
} Device;

Device* DeviceConstructor();

void DeviceDestructor(Device* device);

#endif /* DEVICE_H_ */
