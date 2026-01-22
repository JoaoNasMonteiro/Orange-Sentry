CC = clang
CFLAGS = -std=c99 -Wall -Werror 

BUILD_DIR := ./build
OUT_DIR := ./bin
SRC_DIR := ./src

TARGET_BIN = $(OUT_DIR)/hello_world

all: directories $(TARGET_BIN)

.PHONY: all clean directories

$(BUILD_DIR)/hello.o: $(SRC_DIR)/hello.c 
	mkdir -p $(BUILD_DIR)
	mkdir -p $(OUT_DIR)
	$(CC) $< -c $(CFLAGS) -o $@

$(TARGET_BIN): $(BUILD_DIR)/hello.o
	$(CC) $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(OUT_DIR)