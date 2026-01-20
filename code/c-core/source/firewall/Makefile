TARGET = firewall_service
SRCS = main.c rules.c

# Configuração de bibliotecas
ifeq ($(ARCH),host)
    # No host, pedimos para o pkg-config dizer quais flags de linkagem usar
    LIBS = $(shell pkg-config --libs libmnl libnftnl)
else
    # No ARM, usamos o hardcoded que temos no vendor
    LIBS = -lmnl -lnftnl
endif

all:
	@echo "   [CC] $(TARGET)"
	$(CC) $(GLOBAL_CFLAGS) -o $(BUILD_DIR)/$(TARGET) $(SRCS) $(GLOBAL_LDFLAGS) $(LIBS)

clean:
	rm -f $(BUILD_DIR)/$(TARGET)
