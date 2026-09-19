#include <stdlib.h>
#include <stdio.h>
#include "mv_const.h"

typedef struct {
    // La base y el tamaño ocupan 2 bytes cada uno
    int16_t base;
    int16_t tamanio;
} Segmento;

typedef struct {
    Segmento vec[CANT_SEGMENTOS];
}Tabla_Segmentos;

void inicializar_tabla(Tabla_Segmentos *ts);