cmake_minimum_required(VERSION 3.20)

project(MicroMouseFirmware LANGUAGES C ASM)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(TARGET firmware)

set(SRC_FILES
    src/main.c
    src/motor_control.c
    src/movement_controller.c
    src/MovementLoop.c
    src/FloodFill.c
    src/PathFollow.c
)

set(STARTUP_FILE startup/startup_stm32f103c8t6.s)
set(LINKER_SCRIPT ld/linker.ld)

add_executable(${TARGET}.elf
    ${SRC_FILES}
    ${STARTUP_FILE}
)

target_include_directories(${TARGET}.elf PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(${TARGET}.elf PRIVATE
    -mcpu=cortex-m3
    -mthumb
    -g
    -Wall
    -O2
    -ffunction-sections
    -fdata-sections
)

target_link_options(${TARGET}.elf PRIVATE
    -mcpu=cortex-m3
    -mthumb
    -nostdlib
    -Wl,--gc-sections
    -T${CMAKE_CURRENT_SOURCE_DIR}/${LINKER_SCRIPT}
)

find_program(CMAKE_OBJCOPY arm-none-eabi-objcopy REQUIRED)
find_program(CMAKE_SIZE arm-none-eabi-size REQUIRED)
find_program(CMAKE_OBJDUMP arm-none-eabi-objdump REQUIRED)
find_program(OPENOCD_EXECUTABLE openocd)

add_custom_command(
    TARGET ${TARGET}.elf
    POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary
            $<TARGET_FILE:${TARGET}.elf>
            ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}.bin
    COMMENT "Generating ${TARGET}.bin"
)

add_custom_target(size
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${TARGET}.elf>
    DEPENDS ${TARGET}.elf
    COMMENT "Displaying firmware size"
)

add_custom_target(disasm
    COMMAND ${CMAKE_OBJDUMP} -S $<TARGET_FILE:${TARGET}.elf> > ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}.asm
    DEPENDS ${TARGET}.elf
    COMMENT "Generating disassembly ${TARGET}.asm"
)

if(OPENOCD_EXECUTABLE)
    add_custom_target(flash
        COMMAND ${OPENOCD_EXECUTABLE}
                -f interface/stlink.cfg
                -f target/stm32f1x.cfg
                -c "program $<TARGET_FILE:${TARGET}.elf> verify reset exit"
        DEPENDS ${TARGET}.elf
        COMMENT "Flashing ${TARGET}.elf via ST-Link"
    )
else()
    message(WARNING "openocd not found in PATH. 'flash' target will be unavailable.")
endif()

message(STATUS "Configure with: cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-arm-none-eabi.cmake")
