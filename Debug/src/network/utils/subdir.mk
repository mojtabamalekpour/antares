################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/network/utils/address-utils.cc \
../src/network/utils/ascii-file.cc \
../src/network/utils/data-rate.cc \
../src/network/utils/drop-tail-queue.cc \
../src/network/utils/error-model.cc \
../src/network/utils/ethernet-header.cc \
../src/network/utils/ethernet-trailer.cc \
../src/network/utils/flow-id-tag.cc \
../src/network/utils/inet-socket-address.cc \
../src/network/utils/inet6-socket-address.cc \
../src/network/utils/infinite-queue.cc \
../src/network/utils/infinite-simple-red-ecn-queue.cc \
../src/network/utils/ipv4-address.cc \
../src/network/utils/ipv6-address.cc \
../src/network/utils/llc-snap-header.cc \
../src/network/utils/mac48-address.cc \
../src/network/utils/mac64-address.cc \
../src/network/utils/output-stream-wrapper.cc \
../src/network/utils/packet-burst.cc \
../src/network/utils/packet-socket-address.cc \
../src/network/utils/packet-socket-factory.cc \
../src/network/utils/packet-socket.cc \
../src/network/utils/packetbb.cc \
../src/network/utils/pause-header.cc \
../src/network/utils/pcap-file-wrapper.cc \
../src/network/utils/pcap-file.cc \
../src/network/utils/queue.cc \
../src/network/utils/radiotap-header.cc \
../src/network/utils/red-queue.cc \
../src/network/utils/shared-ecn-queue.cc \
../src/network/utils/simple-channel.cc \
../src/network/utils/simple-net-device.cc \
../src/network/utils/simple-priority-ecn-queue.cc \
../src/network/utils/simple-red-ecn-queue.cc \
../src/network/utils/time-tag.cc 

C_SRCS += \
../src/network/utils/hsieh.c 

CC_DEPS += \
./src/network/utils/address-utils.d \
./src/network/utils/ascii-file.d \
./src/network/utils/data-rate.d \
./src/network/utils/drop-tail-queue.d \
./src/network/utils/error-model.d \
./src/network/utils/ethernet-header.d \
./src/network/utils/ethernet-trailer.d \
./src/network/utils/flow-id-tag.d \
./src/network/utils/inet-socket-address.d \
./src/network/utils/inet6-socket-address.d \
./src/network/utils/infinite-queue.d \
./src/network/utils/infinite-simple-red-ecn-queue.d \
./src/network/utils/ipv4-address.d \
./src/network/utils/ipv6-address.d \
./src/network/utils/llc-snap-header.d \
./src/network/utils/mac48-address.d \
./src/network/utils/mac64-address.d \
./src/network/utils/output-stream-wrapper.d \
./src/network/utils/packet-burst.d \
./src/network/utils/packet-socket-address.d \
./src/network/utils/packet-socket-factory.d \
./src/network/utils/packet-socket.d \
./src/network/utils/packetbb.d \
./src/network/utils/pause-header.d \
./src/network/utils/pcap-file-wrapper.d \
./src/network/utils/pcap-file.d \
./src/network/utils/queue.d \
./src/network/utils/radiotap-header.d \
./src/network/utils/red-queue.d \
./src/network/utils/shared-ecn-queue.d \
./src/network/utils/simple-channel.d \
./src/network/utils/simple-net-device.d \
./src/network/utils/simple-priority-ecn-queue.d \
./src/network/utils/simple-red-ecn-queue.d \
./src/network/utils/time-tag.d 

OBJS += \
./src/network/utils/address-utils.o \
./src/network/utils/ascii-file.o \
./src/network/utils/data-rate.o \
./src/network/utils/drop-tail-queue.o \
./src/network/utils/error-model.o \
./src/network/utils/ethernet-header.o \
./src/network/utils/ethernet-trailer.o \
./src/network/utils/flow-id-tag.o \
./src/network/utils/hsieh.o \
./src/network/utils/inet-socket-address.o \
./src/network/utils/inet6-socket-address.o \
./src/network/utils/infinite-queue.o \
./src/network/utils/infinite-simple-red-ecn-queue.o \
./src/network/utils/ipv4-address.o \
./src/network/utils/ipv6-address.o \
./src/network/utils/llc-snap-header.o \
./src/network/utils/mac48-address.o \
./src/network/utils/mac64-address.o \
./src/network/utils/output-stream-wrapper.o \
./src/network/utils/packet-burst.o \
./src/network/utils/packet-socket-address.o \
./src/network/utils/packet-socket-factory.o \
./src/network/utils/packet-socket.o \
./src/network/utils/packetbb.o \
./src/network/utils/pause-header.o \
./src/network/utils/pcap-file-wrapper.o \
./src/network/utils/pcap-file.o \
./src/network/utils/queue.o \
./src/network/utils/radiotap-header.o \
./src/network/utils/red-queue.o \
./src/network/utils/shared-ecn-queue.o \
./src/network/utils/simple-channel.o \
./src/network/utils/simple-net-device.o \
./src/network/utils/simple-priority-ecn-queue.o \
./src/network/utils/simple-red-ecn-queue.o \
./src/network/utils/time-tag.o 

C_DEPS += \
./src/network/utils/hsieh.d 


# Each subdirectory must supply rules for building sources it contributes
src/network/utils/%.o: ../src/network/utils/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/network/utils/%.o: ../src/network/utils/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


