# Find all sources, relative to SRC_DIR
C_SOURCES   := $(shell find $(SRC_DIR) -name '*.c'   -printf "%P\n")
ASM_SOURCES := $(shell find $(SRC_DIR) -name '*.asm' -printf "%P\n")

# Map them to object paths under BUILD_DIR
C_OBJS   := $(patsubst %.c,   $(BUILD_DIR)/%.o,     $(C_SOURCES))
ASM_OBJS := $(patsubst %.asm, $(BUILD_DIR)/%.asm.o, $(ASM_SOURCES))

# Copy grub.cfg (order-only mkdir via | create)
$(ISO_DIR)/boot/grub/grub.cfg: $(SRC_DIR)/boot/grub.cfg | create
	cp $< $@

# Link the kernel from our objects
$(ISO_DIR)/boot/kernel: $(C_OBJS) $(ASM_OBJS) | create
	$(LD) $(LDFLAGS) -o $@ $^ -lgcc

# Compile C sources (ensure output directory exists)
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(SRC_DIR)/arch/i686/isr_gen.c | create
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Assemble ASM sources (ensure output directory exists)
$(BUILD_DIR)/%.asm.o: $(SRC_DIR)/%.asm $(SRC_DIR)/arch/i686/isr.inc | create
	@mkdir -p $(dir $@)
	$(ASM)  $(ASMFLAGS) -o $@ $<

$(SRC_DIR)/arch/i686/isr.inc: $(TOOLS_DIR)/gen_isr.py
	python3 $< asm > $@

$(SRC_DIR)/arch/i686/isr_gen.c: $(TOOLS_DIR)/gen_isr.py
	python3 $< c > $@