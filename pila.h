#ifndef PILA.H
#define PILA_H

#include <stdint.h>
#include "componentes/mv.h"

// Todas las funciones devuleven 1 si andan y 0 en caso contrario
// Si hay error dejan IP en STOP

// PUSH. Paso 1 y 2: SP -= 4 y control de overflow
int reservar_pila(Maquina_Virtual *mv);

// PUSH. Paso 5: escribe 4 bytes en el tope de la pila
int escribir_tope_pila(Maquina_Virtual *mv, uint32_t valor);

// Reservar + escribir
int apilar(Maquina_Virtual *mv, uint32_t valor);

// POP. Pasos 1 a 3: lee 4 bytes del tope SIN MOVER SP y controla underflow
int leer_tope_pila(Maquina_Virtual *mv, unit32_t *valor);
 
#endif PILA.H