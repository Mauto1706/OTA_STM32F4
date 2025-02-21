################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Middle/File_sequence/file_SQ.c 

OBJS += \
./Middle/File_sequence/file_SQ.o 

C_DEPS += \
./Middle/File_sequence/file_SQ.d 


# Each subdirectory must supply rules for building sources it contributes
Middle/File_sequence/%.o Middle/File_sequence/%.su Middle/File_sequence/%.cyclo: ../Middle/File_sequence/%.c Middle/File_sequence/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I"D:/Gitwork/test/memflash_test/App" -I"D:/Gitwork/test/memflash_test/Middlewares2/ST/STM32_USB_Device_Library/Core/Inc" -I"D:/Gitwork/test/memflash_test/Middlewares2/ST/STM32_USB_Device_Library/Class/CustomHID/Inc" -I"D:/Gitwork/test/memflash_test/USB_DEVICE2/App" -I"D:/Gitwork/test/memflash_test/USB_DEVICE2/Target" -I"D:/Gitwork/test/memflash_test/Drivers/My_Driver/Devices/memflash" -I"D:/Gitwork/test/memflash_test/Drivers/My_Driver/Devices/memflash/hw/w25qxx" -I"D:/Gitwork/test/memflash_test/Drivers/My_Driver/Devices/memflash/cfg" -I"D:/Gitwork/test/memflash_test/Middle/ufs" -I"D:/Gitwork/test/memflash_test/Middle/ufs/cfg" -I"D:/Gitwork/test/memflash_test/Middle/base" -I"D:/Gitwork/test/memflash_test/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc" -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I"D:/Gitwork/test/memflash_test/Middle/llnet" -I"D:/Gitwork/test/memflash_test/Middle/buffer" -I"D:/Gitwork/test/memflash_test/Middle/File_sequence" -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I"D:/Gitwork/test/memflash_test/Middle/flash" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Middle-2f-File_sequence

clean-Middle-2f-File_sequence:
	-$(RM) ./Middle/File_sequence/file_SQ.cyclo ./Middle/File_sequence/file_SQ.d ./Middle/File_sequence/file_SQ.o ./Middle/File_sequence/file_SQ.su

.PHONY: clean-Middle-2f-File_sequence

