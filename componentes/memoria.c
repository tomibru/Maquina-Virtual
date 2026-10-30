#include "memoria.h"
#include <stdio.h>

int inicializar_memoria(Memoria *m, uint32_t tam_bytes) {
    m->ram = (uint8_t *) calloc (tam_bytes, sizeof(uint8_t));
    // remplazo de for por calloc, que deja en 0 los espacios
    m->tam = (m->ram != NULL) ? tam_bytes : 0;
    return m->ram != NULL;
    // calloc puede fallar y devolver null
}

// calloc requiere su free
void liberar_memoria(Memoria *m) {
    free(m->ram);
    m->ram = NULL;
    m->tam = 0;
}