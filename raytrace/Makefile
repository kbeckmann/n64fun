PROG_NAME = raytrace

ROOTDIR = $(N64_INST)
GCCN64PREFIX = $(ROOTDIR)/bin/mips64-elf-

CC = $(GCCN64PREFIX)gcc
CXX = $(GCCN64PREFIX)g++
AS = $(GCCN64PREFIX)as
LD = $(GCCN64PREFIX)ld
OBJCOPY = $(GCCN64PREFIX)objcopy
N64TOOL = $(ROOTDIR)/bin/n64tool
CHKSUM64 = $(ROOTDIR)/bin/chksum64

ASFLAGS = -mtune=vr4300 -march=vr4300
CFLAGS = -std=gnu99 -march=vr4300 -mtune=vr4300 -Wall -Werror -I$(ROOTDIR)/mips64-elf/include
CXXFLAGS = -march=vr4300 -mtune=vr4300 -O2 -Wall -I$(ROOTDIR)/mips64-elf/include \
	-fno-rtti \
	-fno-exceptions \
	-fno-math-errno \
	-fomit-frame-pointer \
	-fno-stack-protector \
	-DRIGHTSHIFT_IS_SAR \
	-DHAVE_STDINT_H \
	-DIS_LITTLE_ENDIAN \
	-Wno-error=format-truncation

LDFLAGS = -L$(ROOTDIR)/mips64-elf/lib -ldragon -lc -lm -lstdc++ -ldragonsys -Tn64.ld -Wl,--gc-sections
N64TOOLFLAGS = -l 1M -h $(ROOTDIR)/mips64-elf/lib/header -t "Video Res Test"

ifeq ($(N64_BYTE_SWAP),true)
$(PROG_NAME).v64: $(PROG_NAME).z64
	dd conv=swab if=$^ of=$@
endif

$(PROG_NAME).z64: $(PROG_NAME).bin
	$(N64TOOL) $(N64TOOLFLAGS) -o $@ $^
	$(CHKSUM64) $@

$(PROG_NAME).bin: $(PROG_NAME).elf
	$(OBJCOPY) $< $@ -O binary

%.o : %.cpp
	$(CXX) -c $(CXXFLAGS) -fverbose-asm  -Wa,-a,-ad,-alms=$(notdir $(<:.cpp=.lst)) $< -o $@

$(PROG_NAME).elf: $(PROG_NAME).o tinyraytracer.o
	$(CXX) -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f *.v64 *.z64 *.bin *.elf *.o
