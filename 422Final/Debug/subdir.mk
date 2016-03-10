################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../CPU.c \
../CQ.c \
../CondVar.c \
../Device.c \
../Fifo.c \
../Mutex.c \
../Pcb.c \
../PriorityQueue.c 

OBJS += \
./CPU.o \
./CQ.o \
./CondVar.o \
./Device.o \
./Fifo.o \
./Mutex.o \
./Pcb.o \
./PriorityQueue.o 

C_DEPS += \
./CPU.d \
./CQ.d \
./CondVar.d \
./Device.d \
./Fifo.d \
./Mutex.d \
./Pcb.d \
./PriorityQueue.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cygwin C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


