################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/buildings/model/building-list.cc \
../src/buildings/model/building.cc \
../src/buildings/model/buildings-mobility-model.cc \
../src/buildings/model/buildings-propagation-loss-model.cc \
../src/buildings/model/hybrid-buildings-propagation-loss-model.cc \
../src/buildings/model/itu-r-1238-propagation-loss-model.cc \
../src/buildings/model/oh-buildings-propagation-loss-model.cc 

CC_DEPS += \
./src/buildings/model/building-list.d \
./src/buildings/model/building.d \
./src/buildings/model/buildings-mobility-model.d \
./src/buildings/model/buildings-propagation-loss-model.d \
./src/buildings/model/hybrid-buildings-propagation-loss-model.d \
./src/buildings/model/itu-r-1238-propagation-loss-model.d \
./src/buildings/model/oh-buildings-propagation-loss-model.d 

OBJS += \
./src/buildings/model/building-list.o \
./src/buildings/model/building.o \
./src/buildings/model/buildings-mobility-model.o \
./src/buildings/model/buildings-propagation-loss-model.o \
./src/buildings/model/hybrid-buildings-propagation-loss-model.o \
./src/buildings/model/itu-r-1238-propagation-loss-model.o \
./src/buildings/model/oh-buildings-propagation-loss-model.o 


# Each subdirectory must supply rules for building sources it contributes
src/buildings/model/%.o: ../src/buildings/model/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


