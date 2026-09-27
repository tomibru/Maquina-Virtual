# ============================================================
# Makefile - Máquina Virtual (MV1 2026)
# Compila el ejecutor "vmx" a partir de main.c y los módulos
# ============================================================

CC       := gcc
CSTD     := -std=c11
WARN     := -Wall -Wextra
INCLUDES := -I. -Icomponentes
CFLAGS   := $(CSTD) $(WARN) $(INCLUDES)

# Nombre del ejecutable final
TARGET := vmx

# Archivos fuente (raíz + componentes/)
SRCS := main.c \
        cpu.c \
        mmu.c \
        disassembler.c \
        validacionArchivo.c \
        componentes/mv.c \
        componentes/memoria.c \
        componentes/registros.c \
        componentes/tablaS.c

OBJS := $(SRCS:.c=.o)

# Regla por defecto
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

# Compilación genérica de cada .c a .o (permite recompilar sólo lo modificado)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compilación con símbolos de debug (make debug)
debug: CFLAGS += -g -O0
debug: clean $(TARGET)

# Limpieza de objetos y binario
clean:
	rm -f $(OBJS) $(TARGET)

# Ejecutar rápido: make run ARGS="archivo.vmx -d"
run: $(TARGET)
	./$(TARGET) $(ARGS)

.PHONY: all debug clean run