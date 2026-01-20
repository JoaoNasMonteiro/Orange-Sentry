# code/c-core/Makefile

# --- Configurações Padrão ---
# Se o usuário não digitar nada, assume 'arm64' (Cross-Compilação)
ARCH ?= arm64

# Diretórios Comuns
ROOT_DIR = $(CURDIR)
INCLUDE_SHR = $(ROOT_DIR)/include-shr
BUILD_DIR = $(ROOT_DIR)/build/$(ARCH)  # <--- Note que separamos as builds!
SRC_DIR = $(ROOT_DIR)/source

# ==========================================
# MODO 1: CROSS-COMPILAÇÃO (ARM64 / Orange Pi)
# ==========================================
ifeq ($(ARCH),arm64)
    CC = clang
    TARGET_FLAGS = --target=aarch64-linux-gnu
    GCC_TOOLCHAIN = /usr/aarch64-linux-gnu
    
    # Bibliotecas Customizadas (Vendor ARM)
    VENDOR_DIR = $(ROOT_DIR)/vendor/out
    
    GLOBAL_CFLAGS = -Wall -O2 $(TARGET_FLAGS) \
                    --sysroot=$(GCC_TOOLCHAIN) \
                    --gcc-toolchain=/usr \
                    -I$(INCLUDE_SHR) \
                    -I$(VENDOR_DIR)/include
    
    GLOBAL_LDFLAGS = $(TARGET_FLAGS) \
                     --sysroot=$(GCC_TOOLCHAIN) \
                     -L$(VENDOR_DIR)/lib

# ==========================================
# MODO 2: HOST NATIVO (PC / Testes Rápidos)
# ==========================================
else ifeq ($(ARCH),host)
    CC = clang
    # Não usamos target, sysroot ou vendor. Usamos o sistema do PC.
    
    # Dica Pro: pkg-config descobre onde estão as libs no seu PC automaticamente
    HOST_LIBS_CFLAGS := $(shell pkg-config --cflags libmnl libnftnl)
    
    GLOBAL_CFLAGS = -Wall -O2 -g \
                    -I$(INCLUDE_SHR) \
                    $(HOST_LIBS_CFLAGS) \
                    -DHOST_TESTING  # <--- Flag útil para usar #ifdef no código C
    
    GLOBAL_LDFLAGS = 
    # (As libs específicas (-lmnl) serão adicionadas nos Makefiles filhos)

else
    $(error Arquitetura nao suportada: $(ARCH). Use 'arm64' ou 'host')
endif

# Exporta variáveis
export CC GLOBAL_CFLAGS GLOBAL_LDFLAGS BUILD_DIR ARCH

# --- Lista de Módulos ---
# Nota: Talvez o modulo de kernel 'buttons' não compile no host facilmente.
# Podemos filtrar a lista baseada na arquitetura se necessário.
MODULES = firewall controller

all: setup $(MODULES)

setup:
	@mkdir -p $(BUILD_DIR)

$(MODULES):
	@echo ">>> Compilando [$@] para [$(ARCH)]..."
	@$(MAKE) -C $(SRC_DIR)/$@

clean:
	@rm -rf $(ROOT_DIR)/build/*
	@for mod in $(MODULES); do \
		$(MAKE) -C $(SRC_DIR)/$$mod clean; \
	done
