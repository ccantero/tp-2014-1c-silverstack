################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/protocol.c \
../src/umv.c 

OBJS += \
./src/protocol.o \
./src/umv.o 

C_DEPS += \
./src/protocol.d \
./src/umv.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
<<<<<<< HEAD
	gcc -I"/home/utnso/tp-2014-1c-silverstack/libs/commons/commons" -O0 -g3 -Wextra -Werror -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
=======
	gcc -I"/home/utnso/workspace/tp-2014-1c-silverstack/libs/silverstack" -I"/home/utnso/workspace/tp-2014-1c-silverstack/libs/parser" -I"/home/utnso/workspace/tp-2014-1c-silverstack/libs/commons" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
>>>>>>> ad109ffbe9ea70c1762863caf49c0853a30c7ac0
	@echo 'Finished building: $<'
	@echo ' '


