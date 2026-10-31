#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "mv.h"               // Maquina_Virtual
#include "args.h"             // parsear_argumentos
#include "validacionArchivo.h" // validacion_arch (carga un .vmx)
#include "imagen.h"           // cargar_imagen
#include "cpu.h"              // ejecutarCiclo
#include "disassembler.h"     // desensamblarCiclo

int main(int argc, char *argv[]) {
    //Inicializar semilla para uso de RND
    srand(time(NULL));

    Configuracion cfg;
    if (!parsear_argumentos(argc, argv, &cfg))
        return 1; // el error ya se imprimio adentro

    Maquina_Virtual mv;
    int cargada;

    if (cfg.ruta_vmx != NULL)
        cargada = validacion_arch(&cfg, &mv);       // 1) hay .vmx: se arranca un proceso nuevo
    else
        cargada = cargar_imagen(cfg.ruta_vmi, &mv); // 2) solo .vmi: se retoma la ejecucion

    if (!cargada)
        return 1; // el error ya se imprimio adentro

    // Si hay .vmi, los breakpoints (SYS F) van a guardar la imagen ahi
    if (cfg.ruta_vmi != NULL) {
        mv.tiene_vmi = 1;
        strncpy(mv.ruta_vmi, cfg.ruta_vmi, RUTA_MAX - 1);
        mv.ruta_vmi[RUTA_MAX - 1] = '\0';
    }

    if (cfg.desensamblar)
        desensamblarCiclo(&mv);
    else
        ejecutarCiclo(&mv);

    liberarMV(&mv);
    return 0;
}