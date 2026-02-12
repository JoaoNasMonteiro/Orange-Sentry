
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
  LDFLAGS := --target=aarch64-linux-gnu -L $(LIBS_DIR) -lfifo-ipc -lpaho-mqtt3c -lmqtt 
else
	CFLAGS = $(x86_CFLAGS)
	LIBS_DIR = ../../libs/x86
	OUT_DIR := ../../bin/x86
	LDFLAGS := -L $(LIBS_DIR) -lfifo-ipc -lpaho-mqtt3c -lmqtt 
endif

TARGET_BIN := $(OUT_DIR)/mqtt-client
LIBS := $(BUILD_DIR)/mqtt-client.o $(LIBS_DIR)/libfifo-ipc.a $(LIBS_DIR)/libpaho-mqtt3c.a $(LIBS_DIR)/libmqtt.a 

all: directories $(TARGET_BIN)
.PHONY: all clean directories

directories: 
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(OUT_DIR)
	@mkdir -p $(LIBS_DIR)


#--- Compiling Source -----
$(BUILD_DIR)/mqtt-client.o: main.c | directories
	$(CC) $< $(CFLAGS) -c -o $@


# --- Compiling Dependencies -----
$(LIBS_DIR)/libfifo-ipc.a: $(BUILD_DIR)/libfifo-ipc.o
	ar rcs $(LIBS_DIR)/libfifo-ipc.a $(BUILD_DIR)/libfifo-ipc.o

$(BUILD_DIR)/libfifo-ipc.o: $(INCLUDE_DIR)/fifo-ipc.c
	$(CC) $< $(CFLAGS) -c -o $@

$(BUILD_DIR)/libmqtt.o: mqtt.c
	$(CC) $< $(CFLAGS) -c -o $@

$(LIBS_DIR)/libmqtt.a: $(BUILD_DIR)/libmqtt.o 
	ar rcs $(LIBS_DIR)/libmqtt.a $(BUILD_DIR)/libmqtt.o


# ----- Linking -------
$(TARGET_BIN): $(LIBS) 
	$(CC) $^ $(LDFLAGS) -o $@ -v


clean:
	rm -rf $(BUILD_DIR) $(OUT_DIR) $(LIBS_DIR)






