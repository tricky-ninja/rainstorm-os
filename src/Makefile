# Find all sources, relative to SRC_DIR
C_SOURCES   := $(shell find $(SRC_DIR) -name '*.c'   -printf "%P\n")
ASM_SOURCES := $(shell find $(SRC_DIR) -name '*.asm' -printf "%P\n")

# Map them to object paths under BUILD_DIR
C_OBJS   := $(patsubst %.c,   $(BUILD_DIR)/%.o,     $(C_SOURCES))
ASM_OBJS := $(patsubst %.asm, $(BUILD_DIR)/%.asm.o, $(ASM_SOURCES))

# Copy grub.cfg (order-only mkdir via | create)
$(ISO_DIR)/boot/grub/grub.cfg: $(SRC_DIR)/grub.cfg | create
	cp $< $@

# Link the kernel from our objects
$(ISO_DIR)/boot/kernel: $(C_OBJS) $(ASM_OBJS) | create
	$(LD) $(LDFLAGS) -o $@ $^ -lgcc

# Compile C sources (ensure output directory exists)
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | create
	@mkdir -p $(dir $@)
	$(CC)   $(CFLAGS)   -c $< -o $@

# Assemble ASM sources (ensure output directory exists)
$(BUILD_DIR)/%.asm.o: $(SRC_DIR)/%.asm | create
	@mkdir -p $(dir $@)
	$(ASM)  $(ASMFLAGS) -o $@ $<
