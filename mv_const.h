#ifndef MV_CONST_H
#define MV_CONST_H

#define MASCARA_16_BITS   0xFFFF
#define BITS_SEGMENTO     16
#define CANT_SEGMENTOS    8
#define SEGMENTO_LIBRE    0xFFFF   /* valor -1 según el enunciado */

// Memoria: tiene tamaño variable, se indica con m=M (en KiB)
#define MEM_KIB_DEFECTO   16 // Si se omite m=M
#define MEM_KIB_MAXIMO    64 // Tamaño maximo de memoria

// CPU.C
#define STOP 0xFFFFFFFFu

// Códigos de registro
#define REG_IP   0
#define REG_OPC  1
#define REG_OP1  2
#define REG_OP2  3
#define REG_LAR  4
#define REG_MAR  5
#define REG_MBR  6
#define REG_SP   7
#define REG_BP   8
#define REG_EAX  10
#define REG_EBX  11
#define REG_ECX  12
#define REG_EDX  13
#define REG_EEX  14
#define REG_EFX  15
#define REG_AC   16
#define REG_CC   17
#define REG_CS   26
#define REG_DS   27
#define REG_ES   28
#define REG_SS   29
#define REG_KS   30
#define REG_PS   31

#endif