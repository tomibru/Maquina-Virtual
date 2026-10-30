#ifndef MV_H
#define MV_H

#include "memoria.h"
#include "registros.h"
#include "tablaS.h"

#define RUTA_MAX 260

typedef struct{
    Memoria mem;
    Registros regs;
    Tabla_Segmentos ts;

    int tiene_vmi; 
    char ruta_vmi[RUTA_MAX];
    int paso_a_paso;
    int bp_hecho;
} Maquina_Virtual;

int inicializarMV(Maquina_Virtual *mv, uint32_t tam_bytes);
void liberarMV(Maquina_Virtual *mv);

#endif