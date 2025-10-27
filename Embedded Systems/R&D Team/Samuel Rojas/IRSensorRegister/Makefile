# Makefile for STM32F103C8T6 Blue Pill LED Blink

# Toolchain
CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size

# Project name
TARGET = led_blink

# Source files
SRCS = main.c startup.c

# Object files
OBJS = $(SRCS:.c=.o)

# Compiler flags
CFLAGS = -mcpu=cortex-m3 -mthumb
CFLAGS += -Os -Wall -Wextra
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -std=c99

# Linker flags
LDFLAGS = -mcpu=cortex-m3 -mthumb
LDFLAGS += -T linker.ld
LDFLAGS += -nostartfiles
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,-Map=$(TARGET).map

# Default target
all: $(TARGET).bin $(TARGET).hex
	@echo "Build complete!"
	@$(SIZE) $(TARGET).elf

# Build ELF file
$(TARGET).elf: $(OBJS) linker.ld
	$(CC) $(LDFLAGS) $(OBJS) -o $@

# Build binary file
$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

# Build hex file
$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

# Compile C files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -f $(OBJS) $(TARGET).elf $(TARGET).bin $(TARGET).hex $(TARGET).map

# Flash using st-flash (from stlink package)
flash: $(TARGET).bin
	st-flash write $(TARGET).bin 0x8000000

# Flash using OpenOCD
flash-openocd: $(TARGET).bin
	openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
		-c "program $(TARGET).bin 0x08000000 verify reset exit"

.PHONY: all clean flash flash-openocd