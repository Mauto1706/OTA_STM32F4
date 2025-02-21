################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../USB_DEVICE2/App/usb_device.c \
../USB_DEVICE2/App/usbd_custom_hid_if.c \
../USB_DEVICE2/App/usbd_desc.c 

OBJS += \
./USB_DEVICE2/App/usb_device.o \
./USB_DEVICE2/App/usbd_custom_hid_if.o \
./USB_DEVICE2/App/usbd_desc.o 

C_DEPS += \
./USB_DEVICE2/App/usb_device.d \
./USB_DEVICE2/App/usbd_custom_hid_if.d \
./USB_DEVICE2/App/usbd_desc.d 


# Each subdirectory must supply rules for building sources it contributes
USB_DEVICE2/App/%.o USB_DEVICE2/App/%.su USB_DEVICE2/App/%.cyclo: ../USB_DEVICE2/App/%.c USB_DEVICE2/App/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CustomHID/Inc -I"D:/3. stm32_workspace_1.16/Usb_test/App" -I"D:/3. stm32_workspace_1.16/Usb_test/Middlewares2/ST/STM32_USB_Device_Library/Core/Inc" -I"D:/3. stm32_workspace_1.16/Usb_test/Middlewares2/ST/STM32_USB_Device_Library/Class/CustomHID/Inc" -I"D:/3. stm32_workspace_1.16/Usb_test/USB_DEVICE2/App" -I"D:/3. stm32_workspace_1.16/Usb_test/USB_DEVICE2/Target" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-USB_DEVICE2-2f-App

clean-USB_DEVICE2-2f-App:
	-$(RM) ./USB_DEVICE2/App/usb_device.cyclo ./USB_DEVICE2/App/usb_device.d ./USB_DEVICE2/App/usb_device.o ./USB_DEVICE2/App/usb_device.su ./USB_DEVICE2/App/usbd_custom_hid_if.cyclo ./USB_DEVICE2/App/usbd_custom_hid_if.d ./USB_DEVICE2/App/usbd_custom_hid_if.o ./USB_DEVICE2/App/usbd_custom_hid_if.su ./USB_DEVICE2/App/usbd_desc.cyclo ./USB_DEVICE2/App/usbd_desc.d ./USB_DEVICE2/App/usbd_desc.o ./USB_DEVICE2/App/usbd_desc.su

.PHONY: clean-USB_DEVICE2-2f-App

