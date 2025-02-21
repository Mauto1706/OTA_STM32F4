################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../USB_DEVICE/App/usb_device.c \
../USB_DEVICE/App/usbd_cdc_if.c \
../USB_DEVICE/App/usbd_desc.c 

OBJS += \
./USB_DEVICE/App/usb_device.o \
./USB_DEVICE/App/usbd_cdc_if.o \
./USB_DEVICE/App/usbd_desc.o 

C_DEPS += \
./USB_DEVICE/App/usb_device.d \
./USB_DEVICE/App/usbd_cdc_if.d \
./USB_DEVICE/App/usbd_desc.d 


# Each subdirectory must supply rules for building sources it contributes
USB_DEVICE/App/%.o USB_DEVICE/App/%.su USB_DEVICE/App/%.cyclo: ../USB_DEVICE/App/%.c USB_DEVICE/App/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I"D:/Gitwork/test/memflash_test/App" -I"D:/Gitwork/test/memflash_test/Middlewares2/ST/STM32_USB_Device_Library/Core/Inc" -I"D:/Gitwork/test/memflash_test/Middlewares2/ST/STM32_USB_Device_Library/Class/CustomHID/Inc" -I"D:/Gitwork/test/memflash_test/USB_DEVICE2/App" -I"D:/Gitwork/test/memflash_test/USB_DEVICE2/Target" -I"D:/Gitwork/test/memflash_test/Drivers/My_Driver/Devices/memflash" -I"D:/Gitwork/test/memflash_test/Drivers/My_Driver/Devices/memflash/hw/w25qxx" -I"D:/Gitwork/test/memflash_test/Drivers/My_Driver/Devices/memflash/cfg" -I"D:/Gitwork/test/memflash_test/Middle/ufs" -I"D:/Gitwork/test/memflash_test/Middle/ufs/cfg" -I"D:/Gitwork/test/memflash_test/Middle/base" -I"D:/Gitwork/test/memflash_test/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc" -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I"D:/Gitwork/test/memflash_test/Middle/llnet" -I"D:/Gitwork/test/memflash_test/Middle/buffer" -I"D:/Gitwork/test/memflash_test/Middle/File_sequence" -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I"D:/Gitwork/test/memflash_test/Middle/flash" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-USB_DEVICE-2f-App

clean-USB_DEVICE-2f-App:
	-$(RM) ./USB_DEVICE/App/usb_device.cyclo ./USB_DEVICE/App/usb_device.d ./USB_DEVICE/App/usb_device.o ./USB_DEVICE/App/usb_device.su ./USB_DEVICE/App/usbd_cdc_if.cyclo ./USB_DEVICE/App/usbd_cdc_if.d ./USB_DEVICE/App/usbd_cdc_if.o ./USB_DEVICE/App/usbd_cdc_if.su ./USB_DEVICE/App/usbd_desc.cyclo ./USB_DEVICE/App/usbd_desc.d ./USB_DEVICE/App/usbd_desc.o ./USB_DEVICE/App/usbd_desc.su

.PHONY: clean-USB_DEVICE-2f-App

