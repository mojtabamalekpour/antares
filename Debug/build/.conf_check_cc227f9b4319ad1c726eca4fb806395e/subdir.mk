################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../build/.conf_check_cc227f9b4319ad1c726eca4fb806395e/test.cpp 

OBJS += \
./build/.conf_check_cc227f9b4319ad1c726eca4fb806395e/test.o 

CPP_DEPS += \
./build/.conf_check_cc227f9b4319ad1c726eca4fb806395e/test.d 


# Each subdirectory must supply rules for building sources it contributes
build/.conf_check_cc227f9b4319ad1c726eca4fb806395e/%.o: ../build/.conf_check_cc227f9b4319ad1c726eca4fb806395e/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


