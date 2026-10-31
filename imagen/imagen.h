#ifndef IMAGEN_H
#define IMAGEN_H

#include "componentes/mv.h"

// Guarda el estado completo de la MV en mv->ruta_vmi. Devuelve 1 si anduvo.
int guardar_imagen(Maquina_Virtual *mv);

// Carga una imagen .vmi en 'mv' (reserva la memoria segun la cabecera). Devuelve 1 si anduvo.
int cargar_imagen(char *ruta, Maquina_Virtual *mv);

// Breakpoint: genera la imagen y espera la accion del usuario (g / q / Enter).
void ejecutar_breakpoint(Maquina_Virtual *mv);

#endif