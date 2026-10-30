#include "mv.h"

int inicializarMV(Maquina_Virtual *mv, uint32_t tam_bytes) {
    mv->tiene_vmi = 0;
    mv->ruta_vmi[0] = '\0';
    mv->paso_a_paso = 0;
    mv->bp_hecho = 0;

    inicializar_registros(&mv->regs);
    inicializar_tabla(&mv->ts);
    return inicializar_memoria(&mv->mem, tam_bytes);
}

void liberarMV(Maquina_Virtual *mv) {
    liberar_memoria(&mv->mem);
}