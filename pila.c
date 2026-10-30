#include <stdio.h>
#include "pila.h"
#include "mmu.h"
#include "mv_const.h"

int reservar_pila(Maquina_Virtual *mv) {
    int ok=1;
    // 1. Deecrementar SP en 4
    mv->regs.vec[SP] -= 4;

    // 2. Revisar si hubo stack overflow
    if((uint32_t)mv->regs.vec[SP] < (uint32_t)mv->regs.vec[SS]) {
        printf("Error: Stack Overflow");
        mv->regs.vec[IP] = STOP;
        ok = 0;
    }
    return ok;
}

int escribir_tope_pila(Maquina_Virtual *mv, uint32_t valor) {
    int ok = 0;
    uint32_t dir_fisica;

    // la MMU valida que los 4 bytes caigan dentro del segmento
    if(Traducir_Dl_a_DF(mv, (uint32_t)mv->regs.vec[SP], 4, 0, &dir_fisica)) {
        // Big endian
        for (int i=3; i>=0; i++) {
            mv->mem.ram[dir_fisica + i] = valor & 0xFF;
            valor >>= 8;
        }
        ok = 1;
    }
    else
        mv->regs.vec[IP] = STOP;
    return ok;
}

// Reservar + escribir
int apilar(Maquina_Virtual *mv, uint32_t valor) {
    return reservar_pila(mv) && escribir_tope_pila(mv, valor);
}

int leer_tope_pila(Maquina_Virtual *mv, unit32_t *valor) {
    int ok = 0;
    uint32_t dir_fisica;
    uint32_t sp = (uint32_t)mv->regs.vec[SP];
    uint32_t ss = (uint32_t)mv->regs.vec[SS];
    uint32_t tam_pila = 0;

    if(SS != STOP)
        tam_pila = mv->ts.vec[(ss >> 16) & MASCARA_16_BITS].tamanio;
    // 2. STACK UNDERFLOW: pila vacia o sin 4 bytes para sacar
    if ((sp & MASCARA_16_BITS) + 4 > tam_pila) {
        printf("Error: Stack Underflow.\n");
        mv->regs.vec[IP] = STOP;
    }
    else if (Traducir_DL_a_DF(mv, sp, 4, 0, &dir_fisica)) {
        // 1 y 3. Extraer 4 bytes; el del tope (SP) es el mas significativo
        uint32_t v = 0;
        for (int i = 0; i < 4; i++)
            v = (v << 8) | mv->mem.ram[dir_fisica + i];
        *valor = v;
        ok = 1;
    }
    else
        mv->regs.vec[IP] = STOP;
    return ok;
}