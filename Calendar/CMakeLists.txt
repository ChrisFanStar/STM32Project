# THIS FILE IS AUTO GENERATED FROM THE TEMPLATE! DO NOT CHANGE!
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
cmake_minimum_required(VERSION 3.28)

# specify cross-compilers and tools
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER  arm-none-eabi-gcc)
set(CMAKE_AR arm-none-eabi-ar)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(CMAKE_OBJDUMP arm-none-eabi-objdump)
set(SIZE arm-none-eabi-size)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# project settings
project(Calendar C CXX ASM)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

# !!! 强制使用最小体积优化（覆盖原有配置）
add_compile_options(-Os)  # 优先级高于 -Ofast/-Og
add_compile_options(-ffunction-sections -fdata-sections -fno-common -fmessage-length=0)
add_compile_options(-fno-unwind-tables -fno-asynchronous-unwind-tables)  # !!! 减少异常处理开销

# !!! 启用链接时优化（LTO）
add_compile_options(-flto)
add_link_options(-flto)

# Hardware/Software FPU (disabled by default for F103C8T6)
#add_compile_options(-mfloat-abi=hard -mfpu=fpv4-sp-d16)
#add_link_options(-mfloat-abi=hard -mfpu=fpv4-sp-d16)

# MCU-specific options
add_compile_options(-mcpu=cortex-m3 -mthumb -mthumb-interwork)
add_link_options(-mcpu=cortex-m3 -mthumb -mthumb-interwork)

# Enable assembler files preprocessing
add_compile_options($<$<COMPILE_LANGUAGE:ASM>:-x$<SEMICOLON>assembler-with-cpp>)

# !!! 精简调试信息（保留必要符号）
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    add_compile_options(-g1)  # !!! 使用 -g1 替代 -g
else ()
    add_compile_options(-fno-builtin)  # !!! 禁用内置函数
endif ()

# Include directories
include_directories(
        Core/Inc
        Drivers/STM32F1xx_HAL_Driver/Inc/Legacy
        Drivers/STM32F1xx_HAL_Driver/Inc
        Drivers/CMSIS/Device/ST/STM32F1xx/Include
        Drivers/CMSIS/Include
        App/Inc
        Lib/Inc
)

# !!! 禁用未使用的 HAL 模块
add_definitions(
        -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB
        -DHAL_ADC_MODULE_DISABLED  # !!! 示例：禁用 ADC
        -DHAL_CAN_MODULE_DISABLED  # !!! 根据实际需求调整
)

# !!! 使用 newlib-nano 标准库
add_definitions(--specs=nano.specs)
link_libraries(c_nano nosys)  # !!! 替换原有的 `c nosys`

file(GLOB_RECURSE SOURCES "App/*.*" "Core/*.*" "Drivers/*.*" "Lib/*.*")
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/STM32F103C8TX_FLASH.ld)

# !!! 链接器优化：清除未使用段 + 最小内存占用
add_link_options(
        -Wl,--gc-sections
        -Wl,--print-memory-usage
        -Wl,-Map=${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map
        -T ${LINKER_SCRIPT}
)

add_executable(${PROJECT_NAME}.elf ${SOURCES} ${LINKER_SCRIPT})

set(HEX_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.hex)
set(BIN_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin)

add_custom_command(
        TARGET ${PROJECT_NAME}.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${HEX_FILE}
        COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${PROJECT_NAME}.elf> ${BIN_FILE}
        COMMENT "Generating ${HEX_FILE} and ${BIN_FILE}"
)