################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../FreeRTOS_Source/portable/heap_2.c \
../FreeRTOS_Source/portable/port.c 

OBJS += \
./FreeRTOS_Source/portable/heap_2.o \
./FreeRTOS_Source/portable/port.o 

C_DEPS += \
./FreeRTOS_Source/portable/heap_2.d \
./FreeRTOS_Source/portable/port.d 


# Each subdirectory must supply rules for building sources it contributes
FreeRTOS_Source/portable/%.o: ../FreeRTOS_Source/portable/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DDEBUG -D__USE_CMSIS=CMSISv1p30_LPC17xx -D__CODE_RED -D__REDLIB__ -I"C:\Users\patricia\Documents\LPCXpresso_6.1.4_194\workspace2\Example18" -I"C:\Users\patricia\Documents\LPCXpresso_6.1.4_194\workspace2\Example18\FreeRTOS_Source\include" -I"C:\Users\patricia\Documents\LPCXpresso_6.1.4_194\workspace2\Example18\FreeRTOS_Source\portable" -I"C:\Users\patricia\Documents\LPCXpresso_6.1.4_194\workspace2\FreeRTOS_Library\demo_code" -I"C:\Users\patricia\Documents\LPCXpresso_6.1.4_194\workspace2\CMSISv1p30_LPC17xx\inc" -O1 -g3 -fsigned-char -c -fmessage-length=0 -fno-builtin -ffunction-sections -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


