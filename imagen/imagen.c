#include <stdio.h>
#include <string.h>
#include "imagen.h"
#include "mv_const.h"

// ---- Funciones auxiliares para el archivo .vmi (todo en BIG-ENDIAN) ----

// Escribe 2 bytes: primero el mas significativo
void escribirArchivo2(FILE *f, uint32_t valor) {
    uint8_t b[2];
    b[0] = (uint8_t)(valor >> 8);
    b[1] = (uint8_t)valor;
    fwrite(b, 1, 2, f);
}

// Escribe 4 bytes: primero el mas significativo
void escribirArchivo4(FILE *f, uint32_t valor) {
    uint8_t b[4];
    b[0] = (uint8_t)(valor >> 24);
    b[1] = (uint8_t)(valor >> 16);
    b[2] = (uint8_t)(valor >> 8);
    b[3] = (uint8_t)valor;
    fwrite(b, 1, 4, f);
}

// Lee 2 bytes. Devuelve 0 si el archivo se termino antes
int leerArchivo2(FILE *f, uint32_t *destino) {
    uint8_t b[2];
    if (fread(b, 1, 2, f) != 2)
        return 0;
    *destino = (uint32_t)b[0] << 8 | b[1];
    return 1;
}

// Lee 4 bytes. Devuelve 0 si el archivo se termino antes
int leerArchivo4(FILE *f, uint32_t *destino) {
    uint8_t b[4];
    if (fread(b, 1, 4, f) != 4)
        return 0;
    *destino = (uint32_t)b[0] << 24 | (uint32_t)b[1] << 16 | (uint32_t)b[2] << 8 | b[3];
    return 1;
}

// Guardar la imagen
int guardarImagen(Maquina_Virtual *mv) {
    FILE *arch = fopen(mv->ruta_vmi, "wb");
    uint8_t version = 1;
    int ok;

    if(arch == NULL) {
        printf("Error: no se pudo cerar la imagen %s", mv->ruta_vmi);
        return 0;
    }

    // 1. Cabecera
    fwrite("VMI26", 1, 5, arch);
    fwrite(&version, 1, 1, arch);
    escribirArchivo2(arch, mv->mem.tam / 1024);

    // 2. Registros
    for (int i=0; i < NUM_REGS; i++)
        escribirArchivo4(arch, (uint32_t)mv->regs.vec[i]);
    
    // 3. Tabla de segmentos
    for (int i=0; i < CANT_SEGMENTOS; i++) {
        escribirArchivo2(arch, mv->ts.vec[i].base);
        escribirArchivo2(arch,mv->ts.vec[i].tamanio);
    }

    // 4. Memoria Principal
    ok = fwrite(mv->mem.ram, 1, mv->mem.tam, arch) == mv->mem.tam;
    fclose(arch);
    return ok;
}

// Cargar una imagen

// Se usa cuando el archivo termina antes de lo esperado: libera lo reservado y avisa
int imagenTruncada(FILE *arch, Maquina_Virtual *mv) {
    printf("Error: Imagen truncada.\n");
    liberarMV(mv);
    fclose(arch);
    return 0;
}

int cargarImagen (char *ruta, Maquina_Virtual *mv) {
    FILE *arch = fopen(ruta, "rb");
    char id[5];
    uint8_t version;
    uint32_t kib;
    uint32_t valor;

    if (arch == NULL) {
        printf("Error: No se pudo abrir el archivo");
        return 0;
    }

    // 1. Cabecera
    if (fread(id, 1, 5, arch) != 5 || strncmp(id, "VMI26", 5) != 0) {
        printf("Error: Imagen inválida. Identificador incorrecto.\n");
        fclose(arch);
        return 0;
    }
    if (fread(&version, 1, 1, arch) != 1 || version != 1) {
        printf("Error: Imagen inválida. Versión incorrecta.\n");
        fclose(arch);
        return 0;
    }
    if (!leerArchivo2(arch, &kib) || kib == 0) {
        printf("Error: Imagen inválida. Tamaño de memoria incorrecto.\n");
        fclose(arch);
        return 0;
    }

    // La memoria tiene el tamaño que dice la imagen (el parametro m=M se ignora)
    if (!inicializarMV(mv, kib * 1024)) {
        printf("Error: No se pudo reservar la memoria principal.\n");
        fclose(arch);
        return 0;
    }

    // 2. Registros
    for (int i = 0; i < NUM_REGS; i++) {
        if (!leerArchivo4(arch, &valor))
            return imagenTruncada(arch, mv);
        mv->regs.vec[i] = (int32_t)valor;
    }

    // 3. Tabla de segmentos
    for (int i = 0; i < CANT_SEGMENTOS; i++) {
        if (!leerArchivo2(arch, &valor))
            return imagenTruncada(arch, mv);
        mv->ts.vec[i].base = (uint16_t)valor;

        if (!leerArchivo2(arch, &valor))
            return imagenTruncada(arch, mv);
        mv->ts.vec[i].tamanio = (uint16_t)valor;
    }

    // 4/ Memoria principal
    if (fread(mv->mem.ram, 1, mv->mem.tam, arch) != mv->mem.tam)
        return imagenTruncada(arch, mv);

    fclose(arch);
    return 1;
}

// ---- Breakpoint (SYS F) ----
void ejecutar_breakpoint(Maquina_Virtual *mv) {
    int tecla;
    int resto;

    // Sin archivo .vmi, los breakpoints se ignoran
    if (!mv->tiene_vmi)
        return;

    mv->bp_hecho = 1; // avisa al ciclo que esta instruccion ya pauso

    // 1. guardar la imagen con el estado actual
    if (!guardar_imagen(mv)) {
        mv->regs.vec[IP] = STOP;
        return;
    }

    // 2. Esperar la orden del usuario (si tipea otra cosa, se vuelve a preguntar)
    while (1) {
        printf("[breakpoint] g = continuar | q = salir | Enter = paso a paso > ");

        // Se lee la primera letra y despues se descarta lo que sobre de la linea, hasta el Enter
        tecla = getchar();
        resto = tecla;
        while (resto != '\n' && resto != EOF)
            resto = getchar();

        if (tecla == EOF) {                       // se acabo la entrada
            mv->regs.vec[IP] = STOP;
            return;
        }
        if (tecla == 'g' || tecla == 'G') {       // seguir hasta el proximo breakpoint
            mv->paso_a_paso = 0;
            return;
        }
        if (tecla == 'q' || tecla == 'Q') {       // abortar (la imagen ya quedo guardada)
            mv->regs.vec[IP] = STOP;
            return;
        }
        if (tecla == '\n') {                      // solo Enter: una instruccion y vuelve a pausar
            mv->paso_a_paso = 1;
            return;
        }
    }
}