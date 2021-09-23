BUILD_DIR=build
include n64.mk

CFLAGS += -I../3rdparty

all: ugfxtest.z64

$(BUILD_DIR)/3rdparty/%.o: $(SOURCE_DIR)/../3rdparty/%.c
	@mkdir -p $(dir $@)
	@echo "    [CC] $<"
	$(CC) -c $(CFLAGS) -o $@ $<


$(BUILD_DIR)/spritemap.dfs: \
	filesystem/test.sprite \
	$(wildcard filesystem/*.obj)

$(BUILD_DIR)/ugfxtest.elf: \
	$(BUILD_DIR)/ugfxtest.o \
	$(BUILD_DIR)/3rdparty/fast_obj_dfs.o \
	$(BUILD_DIR)/3rdparty/fast_obj.o

ugfxtest.z64: N64_ROM_TITLE="UGFX Test"
ugfxtest.z64: N64_FLAGS=-h $(N64_HEADERPATH)/$(N64_HEADERNAME) -l 2M
ugfxtest.z64: $(BUILD_DIR)/spritemap.dfs

clean:
	rm -rf $(BUILD_DIR) ugfxtest.z64

-include $(wildcard $(BUILD_DIR)/*.d)

.PHONY: all clean
