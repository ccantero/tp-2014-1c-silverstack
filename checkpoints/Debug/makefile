################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include umv/src/subdir.mk
-include segundo/servidor/subdir.mk
-include program/subdir.mk
-include primero/subdir.mk
-include kernel/src/subdir.mk
-include cpu/src/subdir.mk
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
all: checkpoints

dependents:
	-cd /home/utnso/tp-2014-1c-silverstack/libs/commons/Debug && $(MAKE) all
	-cd /home/utnso/tp-2014-1c-silverstack/libs/parser/Debug && $(MAKE) all

# Tool invocations
checkpoints: $(OBJS) $(USER_OBJS) /home/utnso/tp-2014-1c-silverstack/libs/commons/Debug/libcommons.so /home/utnso/tp-2014-1c-silverstack/libs/parser/Debug/libparser.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"/home/utnso/tp-2014-1c-silverstack/libs/parser/Debug" -L"/home/utnso/tp-2014-1c-silverstack/libs/commons/Debug" -o "checkpoints" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) checkpoints
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
/home/utnso/tp-2014-1c-silverstack/libs/commons/Debug/libcommons.so:
/home/utnso/tp-2014-1c-silverstack/libs/parser/Debug/libparser.so:

-include ../makefile.targets
