################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/spectrum/test/spectrum-ideal-phy-test.cc \
../src/spectrum/test/spectrum-interference-test.cc \
../src/spectrum/test/spectrum-value-test.cc 

CC_DEPS += \
./src/spectrum/test/spectrum-ideal-phy-test.d \
./src/spectrum/test/spectrum-interference-test.d \
./src/spectrum/test/spectrum-value-test.d 

OBJS += \
./src/spectrum/test/spectrum-ideal-phy-test.o \
./src/spectrum/test/spectrum-interference-test.o \
./src/spectrum/test/spectrum-value-test.o 


# Each subdirectory must supply rules for building sources it contributes
src/spectrum/test/%.o: ../src/spectrum/test/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


