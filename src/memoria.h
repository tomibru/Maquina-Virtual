#include <stdint.h>
#include <stdlib.h>

#define TAM_MEM 16384 // 16 KiB

typedef struct {
    // Cada celda de memroia guarda 1 byte (8 bits)
    uint8_t ram[TAM_MEM];
} Memoria;

void inicializar_memoria(Memoria *m);