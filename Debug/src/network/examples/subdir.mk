################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/network/examples/droptail_vs_red.cc \
../src/network/examples/main-packet-header.cc \
../src/network/examples/main-packet-tag.cc \
../src/network/examples/red-tests.cc 

CC_DEPS += \
./src/network/examples/droptail_vs_red.d \
./src/network/examples/main-packet-header.d \
./src/network/examples/main-packet-tag.d \
./src/network/examples/red-tests.d 

OBJS += \
./src/network/examples/droptail_vs_red.o \
./src/network/examples/main-packet-header.o \
./src/network/examples/main-packet-tag.o \
./src/network/examples/red-tests.o 


# Each subdirectory must supply rules for building sources it contributes
src/network/examples/%.o: ../src/network/examples/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


