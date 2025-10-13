TARGET = firmware

CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
CFLAGS = -mcpu=cortex-m3 -mthumb -O2 -Wall

all: $(TARGET).bin

$(TARGET).elf: main.o startup.o linker.ld
	$(CC) $(CFLAGS) -nostdlib -T linker.ld main.o startup.o -o $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

main.o: main.c
	$(CC) $(CFLAGS) -c $< -o $@

startup.o: startup_stm32f103c8t6.s
	$(CC) $(CFLAGS) -c $< -o $@

flash: $(TARGET).bin
	openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
	-c "program $(TARGET).elf verify reset exit"

clean:
	rm -f *.o *.elf *.bin
