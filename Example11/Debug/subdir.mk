################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../cr_startup_lpc17.c \
../ethmac.c \
../ew_systick.c \
../main.c \
../tcpip.c 

OBJS += \
./cr_startup_lpc17.o \
./ethmac.o \
./ew_systick.o \
./main.o \
./tcpip.o 

C_DEPS += \
./cr_startup_lpc17.d \
./ethmac.d \
./ew_systick.d \
./main.d \
./tcpip.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DDEBUG -D__USE_CMSIS=CMSISv1p30_LPC17xx -D__CODE_RED -D__REDLIB__ -I"C:\Users\patricia\Documents\LPCXpresso_6.1.4_194\workspace2\CMSISv1p30_LPC17xx\inc" -I"C:\Users\patricia\Documents\LPCXpresso_6.1.4_194\workspace2\FreeRTOS_Library\demo_code" -I"C:\Users\patricia\Documents\LPCXpresso_6.1.4_194\workspace2\FreeRTOS_Library\include" -I"C:\Users\patricia\Documents\LPCXpresso_6.1.4_194\workspace2\FreeRTOS_Library\portable" -I"C:\Users\patricia\Documents\LPCXpresso_6.1.4_194\workspace2\Lib_EaBaseBoard\inc" -I"C:\Users\patricia\Documents\LPCXpresso_6.1.4_194\workspace2\Lib_MCU\inc" -O0 -g3 -fsigned-char -c -fmessage-length=0 -fno-builtin -ffunction-sections -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


