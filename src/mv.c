#include "mv.h"

void inicializarMV(Maquina_Virtual *mv) {
    inicializar_memoria(&mv->mem);
    inicializar_regitros(&mv->regs);
    inicializar_tabla(&mv->ts);
}