################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/dcn/helper/fat-tree-helper.cc \
../src/dcn/helper/ipv4-hash-routing-helper.cc 

CC_DEPS += \
./src/dcn/helper/fat-tree-helper.d \
./src/dcn/helper/ipv4-hash-routing-helper.d 

OBJS += \
./src/dcn/helper/fat-tree-helper.o \
./src/dcn/helper/ipv4-hash-routing-helper.o 


# Each subdirectory must supply rules for building sources it contributes
src/dcn/helper/%.o: ../src/dcn/helper/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


