#include "registros.h"
#include <stdio.h>

void inicializar_regitros(Registros *r) {
    for (int i=0; i<NUM_REGS; i++) 
        r->vec[i] = 0;
}