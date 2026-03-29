# Find all sources, relative to SRC_DIR
C_SOURCES   := $(shell find $(SRC_DIR) -name '*.c'   -printf "%P\n")
ASM_SOURCES := $(shell find $(SRC_DIR) -name '*.asm' -printf "%P\n")

# Map them to object paths under BUILD_DIR
C_OBJS   := $(patsubst %.c,   $(BUILD_DIR)/%.o,     $(C_SOURCES))
ASM_OBJS := $(patsubst %.asm, $(BUILD_DIR)/%.asm.o, $(ASM_SOURCES))


$(ISO_DIR)/boot/$(PROJECT_NAME): $(C_OBJS) $(ASM_OBJS) | create
	$(LD) $(LDFLAGS) -o $@ $^ -lgcc


$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(SRC_DIR)/cpu/isr_gen.c | create
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.asm.o: $(SRC_DIR)/%.asm $(SRC_DIR)/cpu/isr_gen.inc | create
	@mkdir -p $(dir $@)
	$(ASM)  $(ASMFLAGS) -o $@ $<

$(SRC_DIR)/cpu/isr_gen.inc: $(TOOLS_DIR)/gen_isr.py
	python3 $< asm > $@

$(SRC_DIR)/cpu/isr_gen.c: $(TOOLS_DIR)/gen_isr.py
	python3 $< c > $@