################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/mpi/examples/nms-p2p-nix-distributed.cc \
../src/mpi/examples/simple-distributed.cc \
../src/mpi/examples/third-distributed.cc 

CC_DEPS += \
./src/mpi/examples/nms-p2p-nix-distributed.d \
./src/mpi/examples/simple-distributed.d \
./src/mpi/examples/third-distributed.d 

OBJS += \
./src/mpi/examples/nms-p2p-nix-distributed.o \
./src/mpi/examples/simple-distributed.o \
./src/mpi/examples/third-distributed.o 


# Each subdirectory must supply rules for building sources it contributes
src/mpi/examples/%.o: ../src/mpi/examples/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


