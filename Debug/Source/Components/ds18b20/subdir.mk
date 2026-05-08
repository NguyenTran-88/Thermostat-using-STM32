################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Source/Components/ds18b20/ds18b20.c 

OBJS += \
./Source/Components/ds18b20/ds18b20.o 

C_DEPS += \
./Source/Components/ds18b20/ds18b20.d 


# Each subdirectory must supply rules for building sources it contributes
Source/Components/ds18b20/%.o Source/Components/ds18b20/%.su Source/Components/ds18b20/%.cyclo: ../Source/Components/ds18b20/%.c Source/Components/ds18b20/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_FULL_LL_DRIVER -DHSE_VALUE=25000000 -DHSE_STARTUP_TIMEOUT=100 -DLSE_STARTUP_TIMEOUT=5000 -DLSE_VALUE=32768 -DEXTERNAL_CLOCK_VALUE=12288000 -DHSI_VALUE=16000000 -DLSI_VALUE=32000 -DVDD_VALUE=3300 -DPREFETCH_ENABLE=1 -DINSTRUCTION_CACHE_ENABLE=1 -DDATA_CACHE_ENABLE=1 -DSTM32F401xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"E:/STM32/Workspace/thermostat_final/Source/App" -I"E:/STM32/Workspace/thermostat_final/Source/BSP" -I"E:/STM32/Workspace/thermostat_final/Source/Components/delay" -I"E:/STM32/Workspace/thermostat_final/Source/Components/ds18b20" -I"E:/STM32/Workspace/thermostat_final/Source/Components/lcd1602" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Source-2f-Components-2f-ds18b20

clean-Source-2f-Components-2f-ds18b20:
	-$(RM) ./Source/Components/ds18b20/ds18b20.cyclo ./Source/Components/ds18b20/ds18b20.d ./Source/Components/ds18b20/ds18b20.o ./Source/Components/ds18b20/ds18b20.su

.PHONY: clean-Source-2f-Components-2f-ds18b20

