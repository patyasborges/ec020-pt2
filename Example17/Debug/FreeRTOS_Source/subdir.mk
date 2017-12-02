################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../FreeRTOS_Source/list.c \
../FreeRTOS_Source/queue.c \
../FreeRTOS_Source/tasks.c 

OBJS += \
./FreeRTOS_Source/list.o \
./FreeRTOS_Source/queue.o \
./FreeRTOS_Source/tasks.o 

C_DEPS += \
./FreeRTOS_Source/list.d \
./FreeRTOS_Source/queue.d \
./FreeRTOS_Source/tasks.d 


# Each subdirectory must supply rules for building sources it contributes
FreeRTOS_Source/%.o: ../FreeRTOS_Source/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DDEBUG -D__USE_CMSIS=CMSISv1p30_LPC17xx -D__CODE_RED -D__REDLIB__ -I"C:\Users\patricia\Documents\LPCXpresso_6.1.4_194\workspace2\FreeRTOS_Library\demo_code" -I"C:\Users\patricia\Documents\LPCXpresso_6.1.4_194\workspace2\Example17" -I"C:\Users\patricia\Documents\LPCXpresso_6.1.4_194\workspace2\Example17\FreeRTOS_Source\include" -I"C:\Users\patricia\Documents\LPCXpresso_6.1.4_194\workspace2\Example17\FreeRTOS_Source\portable" -I"C:\Users\patricia\Documents\LPCXpresso_6.1.4_194\workspace2\CMSISv1p30_LPC17xx\inc" -O1 -g3 -fsigned-char -c -fmessage-length=0 -fno-builtin -ffunction-sections -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


