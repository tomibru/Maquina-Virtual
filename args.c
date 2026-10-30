#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "args.h"
#include "mv_const.h"

// 1 si 'texto' termina con 'ext' (ignora mayusculas/minusculas)
static int termina_con(const char *texto, const char *ext) {
    size_t lt = strlen(texto), le = strlen(ext);
    int ok = 0;
    if (lt >= le) {
        ok = 1;
        for (size_t i = 0; i < le && ok; i++)
            if (tolower((unsigned char)texto[lt - le + i]) != tolower((unsigned char)ext[i]))
                ok = 0;
    }
    return ok;
}

int parsear_argumentos(int argc, char *argv[], Configuracion *cfg) {
    int ok = 1;
    int i = 1;

    cfg->ruta_vmx     = NULL;
    cfg->ruta_vmi     = NULL;
    cfg->mem_kib      = MEM_KIB_DEFECTO;
    cfg->desensamblar = 0;
    cfg->cant_params  = 0;
    cfg->params       = NULL;

    // El enunciado dice que -p va SIEMPRE al final: todo lo que siga son parametros
    while (i < argc && ok) {
        if (strcmp(argv[i], "-p") == 0) {
            cfg->params      = &argv[i + 1];
            cfg->cant_params = argc - i - 1;
            i = argc; // termina el parseo
        }
        else if (strcmp(argv[i], "-d") == 0) {
            cfg->desensamblar = 1;
            i++;
        }
        else if (strncmp(argv[i], "m=", 2) == 0) {
            char *fin;
            long kib = strtol(argv[i] + 2, &fin, 10);
            if (*fin != '\0' || kib < 1 || kib > MEM_KIB_MAXIMO) {
                printf("Error: m=%s invalido. Debe ser un entero entre 1 y %d (KiB).\n",
                       argv[i] + 2, MEM_KIB_MAXIMO);
                ok = 0;
            }
            else
                cfg->mem_kib = (uint32_t)kib;
            i++;
        }
        else if (termina_con(argv[i], ".vmx")) {
            cfg->ruta_vmx = argv[i];
            i++;
        }
        else if (termina_con(argv[i], ".vmi")) {
            cfg->ruta_vmi = argv[i];
            i++;
        }
        else {
            printf("Error: argumento desconocido '%s'.\n", argv[i]);
            ok = 0;
        }
    }

    if (ok && cfg->ruta_vmx == NULL && cfg->ruta_vmi == NULL) {
        printf("Uso: %s [filename.vmx] [filename.vmi] [m=M] [-d] [-p param1 ... paramN]\n", argv[0]);
        printf("Error: hay que indicar al menos un archivo .vmx o .vmi.\n");
        ok = 0;
    }

    // Sin .vmx se ignoran los parametros -p (lo dice el enunciado)
    if (ok && cfg->ruta_vmx == NULL)
        cfg->cant_params = 0;

    return ok;
}