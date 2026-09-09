#include <stdlib.h>
#include <stdio.h>

#define NUM_SEG 8

typedef struct {
    // La base y el tamaño ocupan 2 bytes cada uno
    int16_t base;
    int16_t tamanio;
} Segmento;

typedef struct {
    Segmento vec[NUM_SEG];
}Tabla_Segmentos;

void inicializar_tabla(Tabla_Segmentos *ts);