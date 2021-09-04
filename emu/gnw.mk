PROG_NAME = emu
BUILD_DIR = build_gnw

ROOTDIR = $(N64_INST)
GCCN64PREFIX = $(ROOTDIR)/bin/mips64-elf-

CC = $(GCCN64PREFIX)gcc
AS = $(GCCN64PREFIX)as
LD = $(GCCN64PREFIX)ld
OBJCOPY = $(GCCN64PREFIX)objcopy
N64TOOL = $(ROOTDIR)/bin/n64tool
CHKSUM64 = $(ROOTDIR)/bin/chksum64

C_DEFS = -DIS_BIG_ENDIAN

C_INCLUDES =  \
-I$(ROOTDIR)/mips64-elf/include \
-I. \
-I./gnw \
-I./retro-go-stm32/gnuboy-go/components \
-I./retro-go-stm32/components/odroid \
-I./retro-go-stm32/components/lupng \
-ILCD-Game-Emulator/src \
-ILCD-Game-Emulator/src/cpus \
-ILCD-Game-Emulator/src/gw_sys

OPT = -Ofast
# OPT = -O0
WERROR = 
#WERROR = -Werror

ASFLAGS = -mtune=vr4300 -march=vr4300
CFLAGS = -ggdb -std=gnu99 -march=vr4300 -mtune=vr4300 -Wall $(WERROR) $(OPT) $(C_DEFS) $(C_INCLUDES)
LDFLAGS = -g -L$(ROOTDIR)/mips64-elf/lib -ldragon -lc -lm -ldragonsys -Tn64.ld --gc-sections
N64TOOLFLAGS = -l 1M -h $(ROOTDIR)/mips64-elf/lib/header -t "Emulator Test"

ifeq ($(N64_BYTE_SWAP),true)
$(PROG_NAME).v64: $(PROG_NAME).z64
	dd conv=swab if=$^ of=$@
endif


C_SOURCES =  \
gnw/main_n64.c \
lz4_depack.c \
loaded_gnw_rom.c \
odroid_input.c \
LCD-Game-Emulator/src/cpus/sm500op.c \
LCD-Game-Emulator/src/cpus/sm510op.c \
LCD-Game-Emulator/src/cpus/sm500core.c \
LCD-Game-Emulator/src/cpus/sm5acore.c \
LCD-Game-Emulator/src/cpus/sm510core.c \
LCD-Game-Emulator/src/cpus/sm511core.c \
LCD-Game-Emulator/src/cpus/sm510base.c \
LCD-Game-Emulator/src/gw_sys/gw_romloader.c \
LCD-Game-Emulator/src/gw_sys/gw_graphic.c \
LCD-Game-Emulator/src/gw_sys/gw_system.c \


OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/$(PROG_NAME).z64: $(BUILD_DIR)/$(PROG_NAME).bin
	$(N64TOOL) $(N64TOOLFLAGS) -o $@ $^
	$(CHKSUM64) $@

$(BUILD_DIR)/$(PROG_NAME).bin: $(BUILD_DIR)/$(PROG_NAME).elf
	$(OBJCOPY) $< $@ -O binary

$(BUILD_DIR)/$(PROG_NAME).elf: $(OBJECTS)
	$(LD) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR):
	mkdir $@


clean:
	rm -rf $(BUILD_DIR)
.PHONY: clean
