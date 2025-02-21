################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Middlewares2/ST/STM32_USB_Device_Library/Core/Src/usbd_core.c \
../Middlewares2/ST/STM32_USB_Device_Library/Core/Src/usbd_ctlreq.c \
../Middlewares2/ST/STM32_USB_Device_Library/Core/Src/usbd_ioreq.c 

OBJS += \
./Middlewares2/ST/STM32_USB_Device_Library/Core/Src/usbd_core.o \
./Middlewares2/ST/STM32_USB_Device_Library/Core/Src/usbd_ctlreq.o \
./Middlewares2/ST/STM32_USB_Device_Library/Core/Src/usbd_ioreq.o 

C_DEPS += \
./Middlewares2/ST/STM32_USB_Device_Library/Core/Src/usbd_core.d \
./Middlewares2/ST/STM32_USB_Device_Library/Core/Src/usbd_ctlreq.d \
./Middlewares2/ST/STM32_USB_Device_Library/Core/Src/usbd_ioreq.d 


# Each subdirectory must supply rules for building sources it contributes
Middlewares2/ST/STM32_USB_Device_Library/Core/Src/%.o Middlewares2/ST/STM32_USB_Device_Library/Core/Src/%.su Middlewares2/ST/STM32_USB_Device_Library/Core/Src/%.cyclo: ../Middlewares2/ST/STM32_USB_Device_Library/Core/Src/%.c Middlewares2/ST/STM32_USB_Device_Library/Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CustomHID/Inc -I"D:/3. stm32_workspace_1.16/Usb_test/App" -I"D:/3. stm32_workspace_1.16/Usb_test/Middlewares2/ST/STM32_USB_Device_Library/Core/Inc" -I"D:/3. stm32_workspace_1.16/Usb_test/Middlewares2/ST/STM32_USB_Device_Library/Class/CustomHID/Inc" -I"D:/3. stm32_workspace_1.16/Usb_test/USB_DEVICE2/App" -I"D:/3. stm32_workspace_1.16/Usb_test/USB_DEVICE2/Target" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Middlewares2-2f-ST-2f-STM32_USB_Device_Library-2f-Core-2f-Src

clean-Middlewares2-2f-ST-2f-STM32_USB_Device_Library-2f-Core-2f-Src:
	-$(RM) ./Middlewares2/ST/STM32_USB_Device_Library/Core/Src/usbd_core.cyclo ./Middlewares2/ST/STM32_USB_Device_Library/Core/Src/usbd_core.d ./Middlewares2/ST/STM32_USB_Device_Library/Core/Src/usbd_core.o ./Middlewares2/ST/STM32_USB_Device_Library/Core/Src/usbd_core.su ./Middlewares2/ST/STM32_USB_Device_Library/Core/Src/usbd_ctlreq.cyclo ./Middlewares2/ST/STM32_USB_Device_Library/Core/Src/usbd_ctlreq.d ./Middlewares2/ST/STM32_USB_Device_Library/Core/Src/usbd_ctlreq.o ./Middlewares2/ST/STM32_USB_Device_Library/Core/Src/usbd_ctlreq.su ./Middlewares2/ST/STM32_USB_Device_Library/Core/Src/usbd_ioreq.cyclo ./Middlewares2/ST/STM32_USB_Device_Library/Core/Src/usbd_ioreq.d ./Middlewares2/ST/STM32_USB_Device_Library/Core/Src/usbd_ioreq.o ./Middlewares2/ST/STM32_USB_Device_Library/Core/Src/usbd_ioreq.su

.PHONY: clean-Middlewares2-2f-ST-2f-STM32_USB_Device_Library-2f-Core-2f-Src

