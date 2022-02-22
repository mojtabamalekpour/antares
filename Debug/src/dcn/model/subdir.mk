################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/dcn/model/dcn.cc \
../src/dcn/model/fivetuple.cc \
../src/dcn/model/hash-function-impl.cc \
../src/dcn/model/hash-routing.cc \
../src/dcn/model/md5sum.cc \
../src/dcn/model/mqq-net-device.cc \
../src/dcn/model/multi-priority-queue.cc \
../src/dcn/model/order-tag.cc \
../src/dcn/model/priority-queue.cc \
../src/dcn/model/priority-tag.cc \
../src/dcn/model/qbb-net-device.cc \
../src/dcn/model/shared-net-device.cc \
../src/dcn/model/simple-priority-ecn-queue.cc 

C_SRCS += \
../src/dcn/model/hsieh.c \
../src/dcn/model/md5.c 

CC_DEPS += \
./src/dcn/model/dcn.d \
./src/dcn/model/fivetuple.d \
./src/dcn/model/hash-function-impl.d \
./src/dcn/model/hash-routing.d \
./src/dcn/model/md5sum.d \
./src/dcn/model/mqq-net-device.d \
./src/dcn/model/multi-priority-queue.d \
./src/dcn/model/order-tag.d \
./src/dcn/model/priority-queue.d \
./src/dcn/model/priority-tag.d \
./src/dcn/model/qbb-net-device.d \
./src/dcn/model/shared-net-device.d \
./src/dcn/model/simple-priority-ecn-queue.d 

OBJS += \
./src/dcn/model/dcn.o \
./src/dcn/model/fivetuple.o \
./src/dcn/model/hash-function-impl.o \
./src/dcn/model/hash-routing.o \
./src/dcn/model/hsieh.o \
./src/dcn/model/md5.o \
./src/dcn/model/md5sum.o \
./src/dcn/model/mqq-net-device.o \
./src/dcn/model/multi-priority-queue.o \
./src/dcn/model/order-tag.o \
./src/dcn/model/priority-queue.o \
./src/dcn/model/priority-tag.o \
./src/dcn/model/qbb-net-device.o \
./src/dcn/model/shared-net-device.o \
./src/dcn/model/simple-priority-ecn-queue.o 

C_DEPS += \
./src/dcn/model/hsieh.d \
./src/dcn/model/md5.d 


# Each subdirectory must supply rules for building sources it contributes
src/dcn/model/%.o: ../src/dcn/model/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/dcn/model/%.o: ../src/dcn/model/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


