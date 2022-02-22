################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/network/utils/backup/order-tag.cc \
../src/network/utils/backup/priority-queue.cc \
../src/network/utils/backup/priority-tag.cc 

CC_DEPS += \
./src/network/utils/backup/order-tag.d \
./src/network/utils/backup/priority-queue.d \
./src/network/utils/backup/priority-tag.d 

OBJS += \
./src/network/utils/backup/order-tag.o \
./src/network/utils/backup/priority-queue.o \
./src/network/utils/backup/priority-tag.o 


# Each subdirectory must supply rules for building sources it contributes
src/network/utils/backup/%.o: ../src/network/utils/backup/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


