################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Program/program.c \
../src/Program/protocol.c 

OBJS += \
./src/Program/program.o \
./src/Program/protocol.o 

C_DEPS += \
./src/Program/program.d \
./src/Program/protocol.d 


# Each subdirectory must supply rules for building sources it contributes
src/Program/%.o: ../src/Program/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


