#!/bin/bash

# ==============================================================================
# SCRIPT DE BUILD DE DEPENDÊNCIAS (Cross-Platform)
# ==============================================================================
# Este script compila as bibliotecas externas para x86 e
# ARM, movendo os binários estáticos (.a) para a pasta libs/ organizada.
#
# Uso: ./scripts/build_dependencies.sh
# ==============================================================================

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
EXTERNAL_DIR="$PROJECT_ROOT/external"
LIBS_DIR="$PROJECT_ROOT/libs"
PAHO_SRC="$EXTERNAL_DIR/paho.mqtt.c"

GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}[INFO] Iniciando build de dependências...${NC}"
echo "       Raiz do Projeto: $PROJECT_ROOT"

if [ ! -f "$PAHO_SRC/CMakeLists.txt" ]; then
  echo "ERRO: O código fonte do Paho MQTT não foi encontrado em $PAHO_SRC."
  echo "      Você rodou 'git submodule update --init --recursive'?"
  exit 1
fi

if ! command -v arm-linux-gnueabihf-gcc &>/dev/null; then
  echo "ERRO: O compilador cross 'arm-linux-gnueabihf-gcc' não foi encontrado."
  echo "      Instale com: sudo apt install gcc-arm-linux-gnueabihf"
  exit 1
fi

build_paho() {
  local ARCH=$1
  local CMAKE_FLAGS=$2
  local BUILD_DIR="$PAHO_SRC/build_$ARCH"
  local TARGET_LIB_DIR="$LIBS_DIR/$ARCH"

  echo -e "${GREEN}[BUILD] Compilando Paho MQTT para arquitetura: $ARCH${NC}"

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

  echo -e "${GREEN}[OK] Lib gerada em: $TARGET_LIB_DIR/libpaho-mqtt3c.a${NC}"
}

build_paho "x86" ""

build_paho "arm" "-DCMAKE_C_COMPILER=arm-linux-gnueabihf-gcc"

echo -e "${BLUE}[INFO] Processo concluído com sucesso!${NC}"
echo -e "       Não esqueça de verificar se as libs estão linkando no Makefile."
