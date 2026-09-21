#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include "mv.h"

// Definición del tipo de puntero a función para la Jump Table de instrucciones
typedef void (*InstruccionFunc)(Maquina_Virtual *mv);

// Tabla de salto para los códigos de operación (0x00 a 0x1F)
extern InstruccionFunc tabla_opc[32];

// Prototipo de la función principal que ejecuta el ciclo de la CPU
void ejecutarCiclo(Maquina_Virtual *mv);

// Funciones auxiliares de operandos y flags
uint32_t obtener_valor_operando(Maquina_Virtual *mv, uint32_t reg_op);
void guardar_valor_operando(Maquina_Virtual *mv, uint32_t reg_op, uint32_t resultado);
void modificar_flags_CC(Maquina_Virtual *mv, uint32_t resultado, uint8_t c, uint8_t v);

// Declaraciones de las funciones de ejecución de instrucciones (0x00 a 0x1F)
void ejecutar_SYS(Maquina_Virtual *mv);
void ejecutar_JMP(Maquina_Virtual *mv);
void ejecutar_JP(Maquina_Virtual *mv);
void ejecutar_JN(Maquina_Virtual *mv);
void ejecutar_JZ(Maquina_Virtual *mv);
void ejecutar_JC(Maquina_Virtual *mv);
void ejecutar_JV(Maquina_Virtual *mv);
void ejecutar_JNP(Maquina_Virtual *mv);
void ejecutar_JNN(Maquina_Virtual *mv);
void ejecutar_JNZ(Maquina_Virtual *mv);
void ejecutar_NOT(Maquina_Virtual *mv);
void ejecutar_STOP(Maquina_Virtual *mv);

void ejecutar_MOV(Maquina_Virtual *mv);
void ejecutar_ADD(Maquina_Virtual *mv);
void ejecutar_SUB(Maquina_Virtual *mv);
void ejecutar_MUL(Maquina_Virtual *mv);
void ejecutar_DIV(Maquina_Virtual *mv);
void ejecutar_CMP(Maquina_Virtual *mv);
void ejecutar_AND(Maquina_Virtual *mv);
void ejecutar_OR(Maquina_Virtual *mv);
void ejecutar_XOR(Maquina_Virtual *mv);
void ejecutar_SWAP(Maquina_Virtual *mv);
void ejecutar_SHL(Maquina_Virtual *mv);
void ejecutar_SHR(Maquina_Virtual *mv);
void ejecutar_SAR(Maquina_Virtual *mv);
void ejecutar_LDL(Maquina_Virtual *mv);
void ejecutar_LDH(Maquina_Virtual *mv);
void ejecutar_RND(Maquina_Virtual *mv);

#endif // CPU_H