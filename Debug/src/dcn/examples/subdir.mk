################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/dcn/examples/dcn-example.cc 

CC_DEPS += \
./src/dcn/examples/dcn-example.d 

OBJS += \
./src/dcn/examples/dcn-example.o 


# Each subdirectory must supply rules for building sources it contributes
src/dcn/examples/%.o: ../src/dcn/examples/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


