#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "componentes/mv.h"
#include "validacionArchivo.h"

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("Error: Debes ingresar el nombre del archivo. Uso: %s <archivo.vmx>\n", argv[0]);
        return 1;
    }
    
    Maquina_Virtual mv;
}