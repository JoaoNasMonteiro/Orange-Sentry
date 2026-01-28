#!/bin/bash

# ==============================================================================
# SCRIPT TO COMPILE THE BUILD DEPEDENCIES (CROSS PLATFORM) 
# ==============================================================================
# This script compiles external libraries for x86 and
# ARM, moving the static binaries (.a) to the libs folder.
#
# Usage: ./scripts/build_dependencies.sh
# ==============================================================================

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
EXTERNAL_DIR="$PROJECT_ROOT/vendor"
LIBS_DIR="$PROJECT_ROOT/libs"
PAHO_SRC="$EXTERNAL_DIR/paho.mqtt.c"

GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}[INFO] Starting build process ...${NC}"
echo "       Project Root: $PROJECT_ROOT"

if [ ! -f "$PAHO_SRC/CMakeLists.txt" ]; then
  echo "ERROR: The Paho MQTT source code was not found in $PAHO_SRC."
  echo "       Did you run 'git submodule update --init --recursive'"?
  exit 1
fi

if ! command -v arm-linux-gnueabihf-gcc &>/dev/null; then
  echo "ERROR: The cross compiler 'arm-linux-gnueabihf-gcc' was not found."
  echo "      Install with: sudo apt install gcc-arm-linux-gnueabihf"
  exit 1
fi

build_paho() {
  local ARCH=$1
  local CMAKE_FLAGS=$2
  local BUILD_DIR="$PAHO_SRC/build_$ARCH"
  local TARGET_LIB_DIR="$LIBS_DIR/$ARCH"

  echo -e "${GREEN}[BUILD] Compiling Paho MQTT for: $ARCH${NC}"

  rm -rf "$BUILD_DIR"
  mkdir -p "$BUILD_DIR"

  mkdir -p "$TARGET_LIB_DIR"

  pushd "$BUILD_DIR" >/dev/null

  cmake .. \
    -DPAHO_BUILD_STATIC=TRUE \
    -DPAHO_BUILD_SHARED=FALSE \
    -DPAHO_WITH_SSL=FALSE \
    -DPAHO_BUILD_TESTS=FALSE \
    -DPAHO_BUILD_SAMPLES=FALSE \
    $CMAKE_FLAGS >/dev/null

  make -j$(nproc) >/dev/null

  popd >/dev/null

  cp "$BUILD_DIR/src/libpaho-mqtt3c.a" "$TARGET_LIB_DIR/"

  echo -e "${GREEN}[OK] Lib generated in: $TARGET_LIB_DIR/libpaho-mqtt3c.a${NC}"
}

build_paho "x86" ""

build_paho "arm" "-DCMAKE_C_COMPILER=arm-linux-gnueabihf-gcc"

echo -e "${BLUE}[INFO] Process completed successfully!${NC}"
echo -e "       Don't forget to check if the libs are linking in the Makefile."