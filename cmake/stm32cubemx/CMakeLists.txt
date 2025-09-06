cmake_minimum_required(VERSION 3.22)

project(stm32cubemx)
add_library(stm32cubemx INTERFACE)

# Enable CMake support for ASM and C languages
enable_language(C ASM)

add_definitions(-DSTM32L4)


target_compile_definitions(stm32cubemx INTERFACE 
	USE_HAL_DRIVER 
	STM32L476xx
    $<$<CONFIG:Debug>:DEBUG>
)

target_include_directories(stm32cubemx INTERFACE
    ../../Core/App/Inc
    ../../Core/Peripherals/Gpio/Inc
    ../../Core/Peripherals/I2c/Inc
    ../../Core/Peripherals/SystemClock/Inc
    ../../Core/Peripherals/Timer/Inc
    ../../Core/Peripherals/Uart/Inc
    ../../Core/Buzzer/Inc
    ../../Core/Hcsr04/Inc
    ../../Core/Ssd1306/Inc
    ../../Drivers/STM32L4xx_HAL_Driver/Inc
    ../../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy
    ../../Drivers/CMSIS/Device/ST/STM32L4xx/Include
    ../../Drivers/CMSIS/Include
)

target_sources(stm32cubemx INTERFACE
    ../../Core/App/Src/main.c
    ../../Core/Peripherals/Gpio/Src/gpio.c
    ../../Core/Peripherals/I2c/Src/i2c.c
    ../../Core/Peripherals/SystemClock/Src/systemclock.c
    ../../Core/Peripherals/Timer/Src/timer.c
    ../../Core/Peripherals/Uart/Src/uart.c
    ../../Core/Ssd1306/Src/ssd1306.c
    ../../Core/Ssd1306/Src/ssd1306_fonts.c
    ../../Core/Ssd1306/Src/ssd1306_tests.c
    ../../Core/Hcsr04/Src/hcsr04.c
    ../../Core/Buzzer/Src/buzzer.c
    ../../Core/App/Src/stm32l4xx_it.c
    ../../Core/App/Src/stm32l4xx_hal_msp.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_tim.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_tim_ex.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_uart.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_uart_ex.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rcc.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rcc_ex.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash_ex.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash_ramfunc.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_gpio.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_i2c.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_i2c_ex.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dma.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dma_ex.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pwr.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pwr_ex.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_cortex.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_exti.c
    ../../Core/App/Src/system_stm32l4xx.c
    ../../Core/App/Src/sysmem.c
    ../../Core/App/Src/syscalls.c
    ../../Core/Startup/startup_stm32l476xx.s
)

target_link_directories(stm32cubemx INTERFACE
)

target_link_libraries(stm32cubemx INTERFACE
)

# Validate that STM32CubeMX code is compatible with C standard
if(CMAKE_C_STANDARD LESS 11)
    message(ERROR "Generated code requires C11 or higher")
endif()


