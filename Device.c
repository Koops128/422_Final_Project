/*
 * Device.c
 *
 *  Created on: Feb 19, 2016
 *      Author: Abigail
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Fifo.h"
#include "Device.h"

Device* DeviceConstructor() {
	Device* device = (Device*) malloc(sizeof(Device));
	device->waitQ = fifoQueueConstructor();
	device->counter = -1;
	return device;
}

void DeviceDestructor(Device* device) {
	fifoQueueDestructor(&device->waitQ);
	free(device);
}
