#ifndef MEMORIA_H
#define MEMORIA_H

#include <stdint.h>
#include <stdlib.h>



typedef struct {
    // Cada celda de memroia guarda 1 byte (8 bits)
    // El arreglo se pide en tiempo de ejecución porque el tamaño de la memoria es variable
    uint8_t *ram;
    uint32_t tam;
} Memoria;

int inicializar_memoria(Memoria *m, uint32_t tam_bytes);

#endif