#include "tablaS.h"
#include <stdio.h>

void inicializar_tabla(Tabla_Segmentos *ts) {
    for (int i=0; i<CANT_SEGMENTOS; i++) {
        ts->vec[i].base = SEGMENTO_LIBRE;
        ts->vec[i].tamanio = SEGMENTO_LIBRE;
    }
}