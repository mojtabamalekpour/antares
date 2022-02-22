################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../scratch/datacenter.cc \
../scratch/dctcp_test.cc \
../scratch/dropper.cc \
../scratch/fat-Agg-link.cc \
../scratch/fattree_jitter_incast.cc \
../scratch/incast.cc \
../scratch/jitter_incast.cc \
../scratch/mix.cc \
../scratch/mqq-datacenter.cc \
../scratch/mqq-incast-jitter-app.cc \
../scratch/mqq-incast-jitter-dropper.cc \
../scratch/mqq-incast-jitter.cc \
../scratch/mqq-incast.cc \
../scratch/mqq-priority-incast.cc \
../scratch/mqq-priority-jitter-incast.cc \
../scratch/mqq-rate-incast.cc \
../scratch/mqq-sharedbuffer-incast.cc \
../scratch/multi-nic.cc \
../scratch/multi-queue.cc \
../scratch/old_incast.cc \
../scratch/oldi_sim.cc \
../scratch/pFabric_sim.cc \
../scratch/priority-datacenter.cc \
../scratch/priority-incast.cc \
../scratch/priority-mix.cc \
../scratch/qbb_test.cc \
../scratch/queuelength.cc \
../scratch/rate-limiter.cc \
../scratch/scratch-simulator.cc \
../scratch/shared-queue.cc \
../scratch/sharedbuffer-incast.cc \
../scratch/test.cc \
../scratch/testfattree.cc \
../scratch/utilization.cc 

CC_DEPS += \
./scratch/datacenter.d \
./scratch/dctcp_test.d \
./scratch/dropper.d \
./scratch/fat-Agg-link.d \
./scratch/fattree_jitter_incast.d \
./scratch/incast.d \
./scratch/jitter_incast.d \
./scratch/mix.d \
./scratch/mqq-datacenter.d \
./scratch/mqq-incast-jitter-app.d \
./scratch/mqq-incast-jitter-dropper.d \
./scratch/mqq-incast-jitter.d \
./scratch/mqq-incast.d \
./scratch/mqq-priority-incast.d \
./scratch/mqq-priority-jitter-incast.d \
./scratch/mqq-rate-incast.d \
./scratch/mqq-sharedbuffer-incast.d \
./scratch/multi-nic.d \
./scratch/multi-queue.d \
./scratch/old_incast.d \
./scratch/oldi_sim.d \
./scratch/pFabric_sim.d \
./scratch/priority-datacenter.d \
./scratch/priority-incast.d \
./scratch/priority-mix.d \
./scratch/qbb_test.d \
./scratch/queuelength.d \
./scratch/rate-limiter.d \
./scratch/scratch-simulator.d \
./scratch/shared-queue.d \
./scratch/sharedbuffer-incast.d \
./scratch/test.d \
./scratch/testfattree.d \
./scratch/utilization.d 

OBJS += \
./scratch/datacenter.o \
./scratch/dctcp_test.o \
./scratch/dropper.o \
./scratch/fat-Agg-link.o \
./scratch/fattree_jitter_incast.o \
./scratch/incast.o \
./scratch/jitter_incast.o \
./scratch/mix.o \
./scratch/mqq-datacenter.o \
./scratch/mqq-incast-jitter-app.o \
./scratch/mqq-incast-jitter-dropper.o \
./scratch/mqq-incast-jitter.o \
./scratch/mqq-incast.o \
./scratch/mqq-priority-incast.o \
./scratch/mqq-priority-jitter-incast.o \
./scratch/mqq-rate-incast.o \
./scratch/mqq-sharedbuffer-incast.o \
./scratch/multi-nic.o \
./scratch/multi-queue.o \
./scratch/old_incast.o \
./scratch/oldi_sim.o \
./scratch/pFabric_sim.o \
./scratch/priority-datacenter.o \
./scratch/priority-incast.o \
./scratch/priority-mix.o \
./scratch/qbb_test.o \
./scratch/queuelength.o \
./scratch/rate-limiter.o \
./scratch/scratch-simulator.o \
./scratch/shared-queue.o \
./scratch/sharedbuffer-incast.o \
./scratch/test.o \
./scratch/testfattree.o \
./scratch/utilization.o 


# Each subdirectory must supply rules for building sources it contributes
scratch/%.o: ../scratch/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


