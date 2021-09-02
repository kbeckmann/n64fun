PROG_NAME = emu
BUILD_DIR = build_gb

ROOTDIR = $(N64_INST)
GCCN64PREFIX = $(ROOTDIR)/bin/mips64-elf-

CC = $(GCCN64PREFIX)gcc
AS = $(GCCN64PREFIX)as
LD = $(GCCN64PREFIX)ld
OBJCOPY = $(GCCN64PREFIX)objcopy
N64TOOL = $(ROOTDIR)/bin/n64tool
CHKSUM64 = $(ROOTDIR)/bin/chksum64

C_DEFS = -DLINUX_EMU

C_INCLUDES =  \
-I$(ROOTDIR)/mips64-elf/include \
-I. \
-I./gb \
-I./retro-go-stm32/gnuboy-go/components \
-I./retro-go-stm32/components/odroid \
-I./retro-go-stm32/components/lupng

OPT = -Ofast
WERROR = 
#WERROR = -Werror

ASFLAGS = -mtune=vr4300 -march=vr4300
CFLAGS = -std=gnu99 -march=vr4300 -mtune=vr4300 -Wall $(WERROR) $(OPT) $(C_DEFS) $(C_INCLUDES)
LDFLAGS = -L$(ROOTDIR)/mips64-elf/lib -ldragon -lc -lm -ldragonsys -Tn64.ld --gc-sections
N64TOOLFLAGS = -l 1M -h $(ROOTDIR)/mips64-elf/lib/header -t "Emulator Test"

ifeq ($(N64_BYTE_SWAP),true)
$(PROG_NAME).v64: $(PROG_NAME).z64
	dd conv=swab if=$^ of=$@
endif

#

C_SOURCES =  \
gb/main_n64.c \
../Core/Src/porting/lib/lz4_depack.c \
odroid_input.c \
odroid_netplay.c \
odroid_overlay.c \
odroid_sdcard.c \
odroid_system.c \
odroid_display.c \
odroid_audio.c \
gw_lcd.c \
loaded_gb_rom.c \
crc32.c \
porting.c \
./retro-go-stm32/gnuboy-go/components/gnuboy/cpu.c \
./retro-go-stm32/gnuboy-go/components/gnuboy/debug.c \
./retro-go-stm32/gnuboy-go/components/gnuboy/emu.c \
./retro-go-stm32/gnuboy-go/components/gnuboy/hw.c \
./retro-go-stm32/gnuboy-go/components/gnuboy/lcd.c \
./retro-go-stm32/gnuboy-go/components/gnuboy/loader.c \
./retro-go-stm32/gnuboy-go/components/gnuboy/mem.c \
./retro-go-stm32/gnuboy-go/components/gnuboy/rtc.c \
./retro-go-stm32/gnuboy-go/components/gnuboy/sound.c \
./retro-go-stm32/components/lupng/miniz.c \


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
