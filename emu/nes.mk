PROG_NAME = emu
BUILD_DIR = build_nes

ROOTDIR = $(N64_INST)
GCCN64PREFIX = $(ROOTDIR)/bin/mips64-elf-

CC = $(GCCN64PREFIX)gcc
AS = $(GCCN64PREFIX)as
LD = $(GCCN64PREFIX)ld
OBJCOPY = $(GCCN64PREFIX)objcopy
N64TOOL = $(ROOTDIR)/bin/n64tool
CHKSUM64 = $(ROOTDIR)/bin/chksum64

C_DEFS = 

C_INCLUDES =  \
-I$(ROOTDIR)/mips64-elf/include \
-I. \
-I./retro-go-stm32/nofrendo-go/components/nofrendo/cpu \
-I./retro-go-stm32/nofrendo-go/components/nofrendo/mappers \
-I./retro-go-stm32/nofrendo-go/components/nofrendo/nes \
-I./retro-go-stm32/nofrendo-go/components/nofrendo \
-I./retro-go-stm32/components/odroid

OPT = -O2
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
nes/nofrendo_linux.c \
nes/main_n64.c \
loaded_nes_rom.c \
crc32.c \
porting.c \
gw_lcd.c \
odroid_audio.c \
odroid_display.c \
odroid_input.c \
odroid_netplay.c \
odroid_overlay.c \
odroid_sdcard.c \
odroid_system.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/bitmap.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/cpu/dis6502.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/cpu/nes6502.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map000.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map001.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map002.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map003.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map004.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map005.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map007.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map008.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map009.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map010.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map011.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map015.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map016.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map018.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map019.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map024.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map032.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map033.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map034.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map040.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map041.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map042.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map046.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map050.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map064.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map065.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map066.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map070.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map073.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map075.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map078.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map079.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map085.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map087.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map093.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map094.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map160.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map162.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map193.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map228.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map229.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map231.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/mappers/mapvrc.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/nes/nes.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/nes/nes_apu.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/nes/nes_input.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/nes/nes_mem.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/nes/nes_mmc.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/nes/nes_ppu.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/nes/nes_rom.c \
./retro-go-stm32/nofrendo-go/components/nofrendo/nes/nes_state.c \


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
