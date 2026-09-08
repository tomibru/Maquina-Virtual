#include <stdlib.h>
#include <stdio.h>

#define NUM_REGS 32

/*
REGISTROS DE INSTRUCCION
IP = 0
OPC = 1
OP1 = 2
OP2 = 3

REGISTROS DE ACCESO A MEMORIA
LAR = 4
MAR = 5
MBR = 6

REGISTROS DE PROPOSITO GENERAL
EAX = 10
EBX = 11
ECX = 12
EDX = 13
EEX = 14
EFX = 15

REGISTRO ACUMULADOR 
AC = 16

REGISTRO DE CODIGO DE CONDICION
CC = 17

REGGISTROS DE SEGMENTOS
CS = 26
DS = 27
*/

typedef struct {
    // Cada refistro es de 4 bytes
    int32_t vec[NUM_REGS];
} Registros;

void inicializar_regitros(Registros *r);