################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../USB_DEVICE2/Target/usbd_conf.c 

OBJS += \
./USB_DEVICE2/Target/usbd_conf.o 

C_DEPS += \
./USB_DEVICE2/Target/usbd_conf.d 


# Each subdirectory must supply rules for building sources it contributes
USB_DEVICE2/Target/%.o USB_DEVICE2/Target/%.su USB_DEVICE2/Target/%.cyclo: ../USB_DEVICE2/Target/%.c USB_DEVICE2/Target/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CustomHID/Inc -I"D:/3. stm32_workspace_1.16/Usb_test/App" -I"D:/3. stm32_workspace_1.16/Usb_test/Middlewares2/ST/STM32_USB_Device_Library/Core/Inc" -I"D:/3. stm32_workspace_1.16/Usb_test/Middlewares2/ST/STM32_USB_Device_Library/Class/CustomHID/Inc" -I"D:/3. stm32_workspace_1.16/Usb_test/USB_DEVICE2/App" -I"D:/3. stm32_workspace_1.16/Usb_test/USB_DEVICE2/Target" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-USB_DEVICE2-2f-Target

clean-USB_DEVICE2-2f-Target:
	-$(RM) ./USB_DEVICE2/Target/usbd_conf.cyclo ./USB_DEVICE2/Target/usbd_conf.d ./USB_DEVICE2/Target/usbd_conf.o ./USB_DEVICE2/Target/usbd_conf.su

.PHONY: clean-USB_DEVICE2-2f-Target

