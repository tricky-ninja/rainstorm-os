PROJECT_NAME = Rainstorm
OUTPUT = $(PROJECT_NAME).iso
SRC_DIR = src
BUILD_DIR = build
ISO_DIR = $(BUILD_DIR)/$(PROJECT_NAME)

ASM = nasm
CC = i686-elf-gcc
LD = i686-elf-gcc

ASMFLAGS = -f elf32
CFLAGS = -ffreestanding -Wall -Wextra -I$(SRC_DIR) -g
LDFLAGS = -T $(SRC_DIR)/link.ld -ffreestanding -nostdlib

include src/Makefile

.PHONY: all clean create run debug

all: $(BUILD_DIR)/$(OUTPUT) create

$(BUILD_DIR)/$(OUTPUT): $(ISO_DIR)/boot/kernel $(ISO_DIR)/boot/grub/grub.cfg create
	grub-mkrescue -o $@ $(ISO_DIR)


run: $(BUILD_DIR)/$(OUTPUT) create
	qemu-system-i386 $<

debug: $(BUILD_DIR)/$(OUTPUT) create
	qemu-system-i386 $< -S -s

create:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(ISO_DIR)
	mkdir -p $(ISO_DIR)/boot
	mkdir -p $(ISO_DIR)/boot/grub

clean:
	rm -rf $(BUILD_DIR)/*