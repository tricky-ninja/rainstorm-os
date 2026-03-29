PROJECT_NAME = rainstorm
OUTPUT = $(PROJECT_NAME).iso
SRC_DIR = src
BUILD_DIR = build
TOOLS_DIR = tools
ISO_DIR = $(BUILD_DIR)/$(PROJECT_NAME)

ASM = nasm
CC  = x86_64-elf-gcc
LD  = x86_64-elf-gcc

ASMFLAGS = -f elf64

CFLAGS  = -ffreestanding \
		  -std=gnu11	\
          -Wall -Wextra -Werror \
          -m64 \
          -mno-red-zone \
          -mno-mmx \
          -mno-sse \
          -mno-sse2 \
          -mcmodel=kernel \
          -I$(SRC_DIR) \
          -g

LDFLAGS = -T $(SRC_DIR)/link.ld \
          -ffreestanding \
          -nostdlib \
          -z max-page-size=0x1000

include src/Makefile

.PHONY: all clean create run run_efi debug

all: $(BUILD_DIR)/$(OUTPUT)

$(BUILD_DIR)/$(OUTPUT): $(ISO_DIR)/boot/$(PROJECT_NAME) | create
	cp -v $(SRC_DIR)/limine.conf limine/limine-bios.sys limine/limine-bios-cd.bin limine/limine-uefi-cd.bin $(ISO_DIR)/boot/limine/
	cp -v limine/BOOTX64.EFI $(ISO_DIR)/EFI/BOOT/
	cp -v limine/BOOTIA32.EFI $(ISO_DIR)/EFI/BOOT/
	xorriso -as mkisofs -R -r -J -b boot/limine/limine-bios-cd.bin \
        -no-emul-boot -boot-load-size 4 -boot-info-table -hfsplus \
        -apm-block-size 2048 --efi-boot boot/limine/limine-uefi-cd.bin \
        -efi-boot-part --efi-boot-image --protective-msdos-label \
        $(ISO_DIR) -o $@
	./limine/limine bios-install $@


run: $(BUILD_DIR)/$(OUTPUT)
	qemu-system-x86_64 $< -serial file:log.txt -m 512M

run_efi: $(BUILD_DIR)/$(OUTPUT)
	qemu-system-x86_64 -M q35 -bios /usr/share/ovmf/OVMF.fd $< -serial file:log.txt -m 512M

debug: $(BUILD_DIR)/$(OUTPUT)
	qemu-system-x86_64 $< -S -s \
		-serial stdio \
		-m 512M \
		-d int,cpu_reset,guest_errors \
		-D qemu.log \
		-no-reboot \
		-no-shutdown

create:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(ISO_DIR)
	mkdir -p $(ISO_DIR)/boot
	mkdir -p $(ISO_DIR)/boot/limine
	mkdir -p $(ISO_DIR)/EFI/BOOT

clean:
	rm -rf $(BUILD_DIR)/*