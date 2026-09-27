#include <stdio.h>
#include <string.h>
#include "mv.h"             // Maquina_Virtual
#include "validacionArchivo.h" // validacion_arch
#include "cpu.h"            // ejecutarCiclo
#include "disassembler.h"   // mostrar_disassembler

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s filename.vmx [-d]\n", argv[0]);
        return 1;
    }

    int mostrar_desensamblado = 0;
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0)
            mostrar_desensamblado = 1;
    }

    Maquina_Virtual mv;
    if (!validacion_arch(argv[1], &mv))
        return 1; // el error ya se imprimio adentro de validacion_arch

    if (mostrar_desensamblado)
        desensamblarCiclo(&mv);
    else
        ejecutarCiclo(&mv);

    return 0;
}