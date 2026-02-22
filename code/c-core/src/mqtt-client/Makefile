CC := clang

INCLUDE_DIR := ../../include
BUILD_DIR := ../../build

x86_CFLAGS := -std=gnu99 -Wall -Werror -g -I $(INCLUDE_DIR) 
arm_CFLAGS := -std=gnu99 -Wall -Werror -g -I $(INCLUDE_DIR) --target=aarch64-linux-gnu 

ARCH ?= x86

ifeq ($(ARCH), arm)
    CFLAGS = $(arm_CFLAGS) 
    LIBS_DIR = ../../libs/arm
    OUT_DIR := ../../bin/arm
    LDFLAGS := --target=aarch64-linux-gnu -lpthread -lrt
else
    CFLAGS = $(x86_CFLAGS)
    LIBS_DIR = ../../libs/x86
    OUT_DIR := ../../bin/x86
    LDFLAGS := -lpthread -lrt
endif

TARGET_BIN := $(OUT_DIR)/mqtt-client

STATIC_LIBS := $(LIBS_DIR)/libmqtt.a $(LIBS_DIR)/libpaho-mqtt3c.a 
MAIN_OBJ := $(BUILD_DIR)/mqtt-client.o

all: directories $(TARGET_BIN)
.PHONY: all clean directories

directories: 
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(OUT_DIR)
	@mkdir -p $(LIBS_DIR)

# --- Compiling Source -----
$(BUILD_DIR)/mqtt-client.o: main.c $(INCLUDE_DIR)/sockclient.h | directories
	$(CC) $< $(CFLAGS) -c -o $@

# --- Compiling Dependencies -----
$(BUILD_DIR)/libmqtt.o: mqtt.c | directories
	$(CC) $< $(CFLAGS) -c -o $@

$(LIBS_DIR)/libmqtt.a: $(BUILD_DIR)/libmqtt.o 
	ar rcs $@ $<

# ----- Linking -------
$(TARGET_BIN): $(MAIN_OBJ) $(STATIC_LIBS) 
	$(CC) $^ $(LDFLAGS) -o $@ -v

clean:
	rm -rf $(BUILD_DIR) $(OUT_DIR) $(LIBS_DIR)/libmqtt.a




