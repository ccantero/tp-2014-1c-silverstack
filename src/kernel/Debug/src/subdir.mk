################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/kernel.c \
../src/protocol.c 

OBJS += \
./src/kernel.o \
./src/protocol.o 

C_DEPS += \
./src/kernel.d \
./src/protocol.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/Escritorio/TP_1C_2014/tp-2014-1c-silverstack/libs/commons" -I"/home/utnso/Escritorio/TP_1C_2014/tp-2014-1c-silverstack/libs/silverstack" -I"/home/utnso/Escritorio/TP_1C_2014/tp-2014-1c-silverstack/libs/parser" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


