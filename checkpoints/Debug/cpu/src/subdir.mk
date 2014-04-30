################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../cpu/src/cpu.c \
../cpu/src/protocol.c 

OBJS += \
./cpu/src/cpu.o \
./cpu/src/protocol.o 

C_DEPS += \
./cpu/src/cpu.d \
./cpu/src/protocol.d 


# Each subdirectory must supply rules for building sources it contributes
cpu/src/%.o: ../cpu/src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/tp-2014-1c-silverstack/libs/parser" -I"/home/utnso/tp-2014-1c-silverstack/libs/commons" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


