#!/bin/bash
set -e

# === CONFIGURAÇÃO DE CAMINHOS ===
# Pega o diretório onde este script está e define a raiz do projeto
SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
PROJECT_ROOT=$(dirname "$SCRIPT_DIR") # Sobe um nível para c-core

VENDOR_DIR="$PROJECT_ROOT/vendor"
INSTALL_DIR="$VENDOR_DIR/out" # Onde os binários finais vão ficar

# === CONFIGURAÇÃO DO CROSS-COMPILER ===
HOST_ARCH="aarch64-linux-gnu"
CROSS_CC="aarch64-linux-gnu-gcc"

echo "=== INICIANDO BUILD DE DEPENDÊNCIAS ==="
echo "Destino: $INSTALL_DIR"

# Cria o diretório de saída limpo
mkdir -p "$INSTALL_DIR"

# --- 1. COMPILANDO LIBMNL (A dependência base) ---
echo ">>> [1/2] Compilando libmnl..."
cd "$VENDOR_DIR/libmnl"

# Limpa builds antigos
if [ -f Makefile ]; then make distclean; fi

# Gera o script 'configure'
./autogen.sh

# Configura para ARM64 e instala no nosso diretório local
./configure --host=$HOST_ARCH --prefix="$INSTALL_DIR" CC=$CROSS_CC

make -j$(nproc)
make install

# --- 2. COMPILANDO LIBNFTNL (A lib de firewall) ---
echo ">>> [2/2] Compilando libnftnl..."
cd "$VENDOR_DIR/libnftnl"

if [ -f Makefile ]; then make distclean; fi

./autogen.sh

# Precisamos dizer para a libnftnl onde encontrar a libmnl que ACABAMOS de compilar.
export PKG_CONFIG_PATH="$INSTALL_DIR/lib/pkgconfig"
export LD_LIBRARY_PATH="$INSTALL_DIR/lib"
# As flags abaixo forçam o compilador a olhar para o diretório vendor/out/include
export CFLAGS="-I$INSTALL_DIR/include"
export LDFLAGS="-L$INSTALL_DIR/lib"

./configure --host=$HOST_ARCH --prefix="$INSTALL_DIR" CC=$CROSS_CC

make -j$(nproc)
make install

echo ""
echo "=== SUCESSO! ==="
echo "As bibliotecas foram compiladas e instaladas em:"
echo "$INSTALL_DIR"
