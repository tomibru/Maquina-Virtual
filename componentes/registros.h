#ifndef REGISTROS_H
#define REGISTROS_H


#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define NUM_REGS 32

typedef struct {
    // Cada registro es de 4 bytes
    int32_t vec[NUM_REGS];
} Registros;

void inicializar_registros(Registros *r);

#endif