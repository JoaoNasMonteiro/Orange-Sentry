
CC := clang

INCLUDE_DIR := ../../include
BUILD_DIR := ../../build

x86_CFLAGS := -std=c99 -Wall -Werror -I $(INCLUDE_DIR)
arm_CFLAGS := -std=c99 -Wall -Werror -I $(INCLUDE_DIR) --target=aarch64-linux-gnu

ARCH ?= x86

ifeq ($(ARCH), arm)
	CFLAGS = $(arm_CFLAGS) 
	LIBS_DIR = ../../libs/arm
	OUT_DIR := ../../bin/arm
else
	CFLAGS = $(x86_CFLAGS)
	LIBS_DIR = ../../libs/x86
	OUT_DIR := ../../bin/x86
endif

TARGET_BIN := $(OUT_DIR)/controller

all: directories $(TARGET_BIN)
.PHONY: all clean directories

directories: 
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(OUT_DIR)


#todos os passos até o assembly
$(BUILD_DIR)/controller.o: controller.c | directories
	$(CC) $< $(CFLAGS) -c -o $@


#linkagem
$(TARGET_BIN): $(BUILD_DIR)/controller.o
	$(CC) $^ -o $@ 


clean:
	rm -rf $(BUILD_DIR) $(OUT_DIR)






