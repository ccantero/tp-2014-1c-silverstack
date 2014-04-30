################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../primero/cliente.c 

OBJS += \
./primero/cliente.o 

C_DEPS += \
./primero/cliente.d 


# Each subdirectory must supply rules for building sources it contributes
primero/%.o: ../primero/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/tp-2014-1c-silverstack/libs/parser" -I"/home/utnso/tp-2014-1c-silverstack/libs/commons" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


