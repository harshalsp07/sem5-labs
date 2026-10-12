# Cross-compiler toolchain prefix (updated to match your Arch installation)
CROSS ?= riscv64-linux-gnu-

CC      = $(CROSS)gcc
LD      = $(CROSS)ld
OBJCOPY = $(CROSS)objcopy

CFLAGS  = -march=rv64gc -mabi=lp64 -mcmodel=medany -ffreestanding -fno-pie -nostdlib -O2 -Wall
LDFLAGS = -T linker.ld -m elf64lriscv

OBJS    = boot.o shell.o uart.o

.PHONY: all run clean

all: shell.elf

# Pattern rule for assembly files (.S)
%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

# Pattern rule for C files (.c)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

shell.elf: $(OBJS) linker.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

run: shell.elf
	qemu-system-riscv64 -M virt -bios none -kernel shell.elf -nographic

clean:
	rm -f *.o shell.elf