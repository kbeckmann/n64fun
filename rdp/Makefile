PROG_NAME = rdp

ROOTDIR = $(N64_INST)
GCCN64PREFIX = $(ROOTDIR)/bin/mips64-elf-

CC = $(GCCN64PREFIX)gcc
AS = $(GCCN64PREFIX)as
LD = $(GCCN64PREFIX)ld
OBJCOPY = $(GCCN64PREFIX)objcopy
CHKSUM64 = $(ROOTDIR)/bin/chksum64
N64TOOL = $(ROOTDIR)/bin/n64tool
MKDFS = $(ROOTDIR)/bin/mkdfs

WERROR = 

ASFLAGS = -mtune=vr4300 -march=vr4300
CFLAGS = -std=gnu99 -march=vr4300 -mtune=vr4300 -Wall $(WERROR) -I$(ROOTDIR)/mips64-elf/include
LDFLAGS = -L$(ROOTDIR)/mips64-elf/lib -ldragon -lc -lm -ldragonsys -Tn64.ld --gc-sections
N64TOOLFLAGS = -l 2M -h $(ROOTDIR)/mips64-elf/lib/header -t "Video Res Test"


C_SOURCES = $(wildcard *.c)
OBJECTS = $(C_SOURCES:.c=.o)

ifeq ($(N64_BYTE_SWAP),true)
$(PROG_NAME).v64: $(PROG_NAME).z64
	dd conv=swab if=$^ of=$@
endif

$(PROG_NAME).z64: $(PROG_NAME).bin $(PROG_NAME).dfs
	$(N64TOOL) $(N64TOOLFLAGS) -o $@ $(PROG_NAME).bin -s 1M $(PROG_NAME).dfs
	$(CHKSUM64) $@

$(PROG_NAME).bin: $(PROG_NAME).elf
	$(OBJCOPY) $< $@ -O binary

$(PROG_NAME).elf: $(OBJECTS)
	$(LD) -o $@ $^ $(LDFLAGS)

$(PROG_NAME).dfs: $(wildcard ./filesystem/*)
	$(MKDFS) $@ ./filesystem/

.PHONY: clean
clean:
	rm -f *.v64 *.z64 *.bin *.dfs *.elf *.o

