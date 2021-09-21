PROG_NAME = slow3d
BUILD_DIR = build

ROOTDIR = $(N64_INST)
GCCN64PREFIX = $(ROOTDIR)/bin/mips64-elf-

CC = $(GCCN64PREFIX)gcc
AS = $(GCCN64PREFIX)as
LD = $(GCCN64PREFIX)ld
OBJCOPY = $(GCCN64PREFIX)objcopy
CHKSUM64 = $(ROOTDIR)/bin/chksum64
N64TOOL = $(ROOTDIR)/bin/n64tool
MKDFS = $(ROOTDIR)/bin/mkdfs

C_SOURCES = \
	$(wildcard *.c) \
	$(wildcard ../3rdparty/*.c)

C_INCLUDES = \
	-I$(ROOTDIR)/mips64-elf/include \
	-I. \
	-I../3rdparty

OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))

# WERROR = -w
# OPT = -O0
OPT   = -O2 -ffast-math
LDOPT =


ASFLAGS = -mtune=vr4300 -march=vr4300
CFLAGS = -g3 $(OPT) -std=gnu99 -march=vr4300 -mtune=vr4300 -Wall $(WERROR) $(C_DEFS) $(C_INCLUDES)
LDFLAGS = -g $(OPT) -L$(ROOTDIR)/mips64-elf/lib -lc -lm -ldragon -ldragonsys -Tn64.ld
N64TOOLFLAGS = -l 2M -h $(ROOTDIR)/mips64-elf/lib/header -t "Video Res Test"

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/$(PROG_NAME).z64: $(BUILD_DIR)/$(PROG_NAME).bin $(BUILD_DIR)/$(PROG_NAME).dfs
	$(N64TOOL) $(N64TOOLFLAGS) -o $@ $(BUILD_DIR)/$(PROG_NAME).bin -s 1M $(BUILD_DIR)/$(PROG_NAME).dfs
	$(CHKSUM64) $@

$(BUILD_DIR)/$(PROG_NAME).dfs: $(wildcard ./filesystem/*) | $(BUILD_DIR)
	$(MKDFS) $@ ./filesystem/

$(BUILD_DIR)/$(PROG_NAME).bin: $(BUILD_DIR)/$(PROG_NAME).elf
	$(OBJCOPY) $< $@ -O binary

$(BUILD_DIR)/$(PROG_NAME).elf: $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR):
	mkdir $@

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

