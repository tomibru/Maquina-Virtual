#include "memoria.h"
#include <stdio.h>

void inicializar_memoria(Memoria *m) {
    for (int i=0; i<TAM_MEM; i++) {
        m->ram[i] = 0;
    }
}