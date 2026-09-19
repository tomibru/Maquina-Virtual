#include "memoria.h"
#include "registros.h"
#include "tablaS.h"

typedef struct{
    Memoria mem;
    Registros regs;
    Tabla_Segmentos ts;
} Maquina_Virtual;

void inicializarMV(Maquina_Virtual *mv);

