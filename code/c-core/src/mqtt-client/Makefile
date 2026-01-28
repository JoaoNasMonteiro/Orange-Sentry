
CC := clang

INCLUDE_DIR := ../../include
BUILD_DIR := ../../build

x86_CFLAGS := -std=c99 -Wall -Werror -g -I $(INCLUDE_DIR)
arm_CFLAGS := -std=c99 -Wall -Werror -g -I $(INCLUDE_DIR) --target=aarch64-linux-gnu

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
$(BUILD_DIR)/mqtt-client.o: main.c | directories
	$(CC) $< $(CFLAGS) -c -o $@


#linkagem
$(TARGET_BIN): $(BUILD_DIR)/mqtt-client.o $(LIBS_DIR)/libfifo-ipc.a $(LIBS_DIR)/libpaho-mqtt3a.a
	$(CC) $^ -o $@ 

$(LIBS_DIR)/libfifo-ipc.a: $(BUILD_DIR)/libfifo-ipc.o
	ar rcs libfifo-ipc.a fifo-ipc.o

$(BUILD_DIR)/libfifo-ipc.o: $(INCLUDE_DIR)/fifo-ipc.c
	$(CC) $< $(CFLAGS) -c -o $@

clean:
	rm -rf $(BUILD_DIR) $(OUT_DIR)






