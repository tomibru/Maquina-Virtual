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
#define IP   0
#define OPC  1
#define OP1  2
#define OP2  3
#define LAR  4
#define MAR  5
#define MBR  6
#define SP   7
#define BP   8
#define EAX  10
#define EBX  11
#define ECX  12
#define EDX  13
#define EEX  14
#define EFX  15
#define AC   16
#define CC   17
#define CS   26
#define DS   27
#define ES   28
#define SS   29
#define KS   30
#define PS   31

#endif