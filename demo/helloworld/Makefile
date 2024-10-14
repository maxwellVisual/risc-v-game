TARGET_NAME = hello
C_SRC = $(TARGET_NAME).c
ASM_SRC = $(TARGET_NAME).s
TARGET_ELF = $(TARGET_NAME).elf
TARGET = $(TARGET_NAME).bin
CFLAGS = -march=rv32i -mabi=ilp32 -Os -T hello.ld
CC = gcc
OBJCOPY = objcopy
CROSS_COMPILE = riscv32-unknown-elf-

.PHONY: all clean
all: $(TARGET)
$(TARGET): $(TARGET_ELF)
	@echo OBJCOPY $@
	@$(CROSS_COMPILE)$(OBJCOPY) -O binary $< $@

$(TARGET_ELF): $(ASM_SRC) $(C_SRC) hello.ld
	@echo CC $@
	@$(CROSS_COMPILE)$(CC) $(CFLAGS) $(ASM_SRC) $(C_SRC) -o $@

clean:
	@rm -rf $(TARGET) $(TARGET_ELF)
