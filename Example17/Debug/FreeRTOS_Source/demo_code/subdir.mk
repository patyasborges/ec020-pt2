################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../FreeRTOS_Source/demo_code/basic_io.c \
../FreeRTOS_Source/demo_code/consoleprint.c 

OBJS += \
./FreeRTOS_Source/demo_code/basic_io.o \
./FreeRTOS_Source/demo_code/consoleprint.o 

C_DEPS += \
./FreeRTOS_Source/demo_code/basic_io.d \
./FreeRTOS_Source/demo_code/consoleprint.d 


# Each subdirectory must supply rules for building sources it contributes
FreeRTOS_Source/demo_code/%.o: ../FreeRTOS_Source/demo_code/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DDEBUG -D__USE_CMSIS=CMSISv1p30_LPC17xx -D__CODE_RED -D__REDLIB__ -I"C:\Users\patricia\Documents\LPCXpresso_6.1.4_194\workspace2\FreeRTOS_Library\demo_code" -I"C:\Users\patricia\Documents\LPCXpresso_6.1.4_194\workspace2\Example17" -I"C:\Users\patricia\Documents\LPCXpresso_6.1.4_194\workspace2\Example17\FreeRTOS_Source\include" -I"C:\Users\patricia\Documents\LPCXpresso_6.1.4_194\workspace2\Example17\FreeRTOS_Source\portable" -I"C:\Users\patricia\Documents\LPCXpresso_6.1.4_194\workspace2\CMSISv1p30_LPC17xx\inc" -O1 -g3 -fsigned-char -c -fmessage-length=0 -fno-builtin -ffunction-sections -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


