#ifndef MMU_H
#define MMU_H

#include "componentes/mv.h"

int Traducir_DL_a_DF(Maquina_Virtual *mv, uint32_t dir_logica, uint16_t cant_bytes, int Codesegment, uint32_t *dir_fisica_calculada);

#endif