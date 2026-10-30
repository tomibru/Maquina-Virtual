#ifndef ARGS_H
#define ARGS_H

#include <stdint.h>

typedef struct {
    const char *ruta_vmx;   // NULL si no se indico .vmx
    const char *ruta_vmi;   // NULL si no se indico .vmi
    uint32_t    mem_kib;    // valor de m=M (o el default)
    int         desensamblar;   // flag -d
    int         cant_params;    // cantidad de parametros luego de -p
    char      **params;         // param1 ... paramN (apuntan dentro de argv)
} Configuracion;

// Devuelve 1 si los argumentos son validos, 0 si hay error (ya lo imprimio).
int parsear_argumentos(int argc, char *argv[], Configuracion *cfg);

#endif