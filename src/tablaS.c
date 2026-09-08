#include "tablaS.h"
#include <stdio.h>

void inicializar_tabla(Tabla_Segmentos *ts) {
    for (int i=0; i<NUM_SEG; i++) {
        ts->vec[i].base = -1;
        ts->vec[i].tamanio = -1;
    }
}