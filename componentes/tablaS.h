#ifndef TABLAS_H
#define TABLAS_H

#include <stdlib.h>
#include <stdio.h>
#include "mv_const.h"
#include <stdint.h>

typedef struct {
    // La base y el tamaño ocupan 2 bytes cada uno
    uint16_t base;
    uint16_t tamanio;
} Segmento;

typedef struct {
    Segmento vec[CANT_SEGMENTOS];
}Tabla_Segmentos;

void inicializar_tabla(Tabla_Segmentos *ts);

#endif