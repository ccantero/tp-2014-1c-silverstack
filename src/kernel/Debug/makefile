################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: kernel

dependents:
	-cd /home/utnso/Escritorio/TP_1C_2014/tp-2014-1c-silverstack/libs/parser/Debug && $(MAKE) clean all
	-cd /home/utnso/Escritorio/TP_1C_2014/tp-2014-1c-silverstack/libs/silverstack/Debug && $(MAKE) clean all
	-cd /home/utnso/Escritorio/TP_1C_2014/tp-2014-1c-silverstack/libs/commons/Debug && $(MAKE) clean all

# Tool invocations
kernel: $(OBJS) $(USER_OBJS) /home/utnso/Escritorio/TP_1C_2014/tp-2014-1c-silverstack/libs/parser/Debug/libparser.so /home/utnso/Escritorio/TP_1C_2014/tp-2014-1c-silverstack/libs/silverstack/Debug/libsilverstack.so /home/utnso/Escritorio/TP_1C_2014/tp-2014-1c-silverstack/libs/commons/Debug/libcommons.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -lpthread -pthread -L"/home/utnso/Escritorio/TP_1C_2014/tp-2014-1c-silverstack/libs/commons/Debug" -L"/home/utnso/Escritorio/TP_1C_2014/tp-2014-1c-silverstack/libs/silverstack/Debug" -L"/home/utnso/Escritorio/TP_1C_2014/tp-2014-1c-silverstack/libs/parser/Debug" -o "kernel" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) kernel
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
/home/utnso/Escritorio/TP_1C_2014/tp-2014-1c-silverstack/libs/parser/Debug/libparser.so:
/home/utnso/Escritorio/TP_1C_2014/tp-2014-1c-silverstack/libs/silverstack/Debug/libsilverstack.so:
/home/utnso/Escritorio/TP_1C_2014/tp-2014-1c-silverstack/libs/commons/Debug/libcommons.so:

-include ../makefile.targets
