#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "componentes/mv.h"
#include "validacionArchivo.h"
#include "pila.h"
#include "mv_const.h"

// Posiciones de los segmentos
#define S_PS 0
#define S_KS 1
#define S_CS 2
#define S_DS 3
#define S_ES 4
#define S_SS 5
#define S_CANT 6

// Devuelve el registro correspondiente a cada segmento
int obtener_registro_segmento(int segmento) {
    switch (segmento) {
        case S_PS:
            return PS;
        case S_KS:
            return KS;
        case S_CS:
            return CS;
        case S_DS:
            return DS;
        case S_ES:
            return ES;
        case S_SS:
            return SS;
        default:
            return -1;
    }
}

// Leer 2 bytes BIG-ENDIAN
int leer2bytes(FILE *a, uint32_t *destino) {
    uint8_t b[2];
    if(fread(b,1,2,a) != 2) 
        return 0;
    else {
        *destino = (uint32_t)b[0] << 8 | b[1];
        return 1;
    }
}

// Escribe un entero de 4 bytes en big endian dentro de la RAM
void escribir4bytes(Maquina_Virtual *mv, uint32_t direccion, uint32_t valor) {
    mv->mem.ram[direccion] = (uint8_t)(valor >> 24);
    mv->mem.ram[direccion + 1] = (uint8_t)(valor >> 16);
    mv->mem.ram[direccion + 2] = (uint8_t)(valor >> 8);
    mv->mem.ram[direccion + 3] = (uint8_t)valor;
}

// Devuelve el string numero i del PS
char *obtenerString(Configuracion *cfg, int i) {
    if (i == 0)
        return cfg->ruta_vmx;
    else
        return cfg->params[i-1];
}

// Calcula el tamaño del PS
uint32_t calcularTamPS(Configuracion *cfg) {
    if(cfg->cant_params == 0)
        return 0;
    else {
        uint32_t tam = (uint32_t)strlen(cfg->ruta_vmx) + 1;
        int cantStrings = cfg->cant_params + 1;

        for (int i = 0; i < cfg->cant_params; i++)
            tam += (uint32_t)strlen(cfg->params[i]) + 1;

        tam += 4 * (uint32_t)cantStrings;
        return tam;
    }
}

// 1er Paso: leer la cabecera del archivo
int leerCabecera(FILE *arch, uint8_t *version, uint32_t tam[], uint32_t *entry) {
    char id[5];

    // Identificador (de 0 a 4) tiene que ser VMX26
    if(fread(id, 1, 5, arch) != 5 || strncmp(id, "VMX26", 5) != 0) {
        printf("Error: Archivo invalido. Identxificador incorrecto");
        return 0;
    }
    else {
        fread(version, 1, 1, arch);
        // Solo se aceptan las versiones 1 y 2
        if (*version != 1 || *version != 2) {
            printf("Error: Archvio invalido. Version incorrecta");
            return 0;
        }
        else {
            // Cabecera P1: solo trae el tamaño del codigo
            if(*version == 1) {
                if (!leer2bytes(arch, &tam[S_CS])) {
                    printf("Error: Archivo invaliodo. No se puedo leer el tamaño del codigo");
                    return 0;
                }
            }
            // Cabecera P2: code, data, extra, stack, const y entry point
            else
                if (!leer2bytes(arch, &tam[S_CS]) || !leer2bytes(arch, &tam[S_DS]) || !leer2bytes(arch, &tam[S_ES]) || !leer2bytes(arch, &tam[S_SS]) || !leer2bytes(arch, &tam[S_KS]) || !leer2bytes(arch, entry)) {
                    printf("Error: Archivo invalido. Cabecera incompleta.");
                    return 0;
                }
        }
    }
    return 1;
}

// 2do Paso: controlar que el proceso entre la memoria y el archivo tenga sentido
int verificarEspacio(uint32_t tam[], uint32_t memBytes, uint32_t entry) {
    uint32_t total = 0;
    for (int s=0; s<S_CANT; s++)
        total += tam[s];
    
    if (tam[S_PS] > 0xFFFF) {
        printf("Error: los parametros exceden el Param Segment");
        return 0;
    }

    if (total > memBytes) {
        printf("Error: Memoria insuficiente. El proceso necesita %u bytes y la memoria tiene %u.\n", total, memBytes);
        return 0;
    }

    if (tam[S_CS] == 0) {
        printf("Error: Archivo invalido. El Code Segment esta vacio");
        return 0;
    }

    if(entry >= tam[S_CS]) {
        printf("Error Archivo invalido. El entry point (%u) cae fuera del Code Segment", entry);
    }
}

// 3er Paso: ubicar los segmentos uno a continuacion del otro, arma la tabla y carga los registros de segmento
// Deja en base la posicion FISICA donde empieza cada segmento
void ubicarSegmentos(Maquina_Virtual *mv, uint32_t tam[], uint32_t base[]) {
    uint32_t posicion = 0; // Indica donde empieza el proximo segmento
    uint32_t entradaTabla = 0; // Proxima entrada libre de la tarde

    for (int s = 0; s < S_CANT; s++) {
        int registro = obtener_registro_segmento(s);
        base[s] = posicion;

        if (tam[s] > 0) {
            mv->ts.vec[entradaTabla].base = (uint16_t) base[s];
            mv->ts.vec[entradaTabla].tamanio = (uint16_t) tam[s];
            // Puntero logico: numero de entrada en los 16 bits alto y offset 0 en los bajos
            mv->regs.vec[registro] = (uint32_t)(entradaTabla << 16);
            entradaTabla++;
        }
        else
            mv->regs.vec[registro = 0xFFFFFFFF]; 
            // El segmento no existe = -1
        
        posicion += tam[s];
    }
}

// 4to Paso: copiar el codigo y las constantes en la memoria
int cargarContenido(FILE *arch, Maquina_Virtual *mv, uint32_t tam[], uint32_t base[]) {
    if (fread(&mv->mem.ram[base[S_CS]], 1, tam[S_CS], arch) != tam[S_CS])
        return 0;
    if (tam[S_KS] > 0 && fread(&mv->mem.ram[base[S_KS]], 1, tam[S_KS], arch))
        return 0;
    return 1;
}

// 5to Paso: IP apunta al entry point
// SP apunta justo despues del final de la pila
void inicializarIPySP(Maquina_Virtual *mv, uint32_t tam[], uint32_t entry) {
    int32_t cs = mv->regs.vec[obtener_registro_segmento(S_CS)];
    int32_t ss = mv->regs.vec[obtener_registro_segmento(S_SS)];

    mv->regs.vec[IP] = cs | entry;
    if(tam[S_SS] > 0)
        mv->regs.vec[SS] = ss + tam[S_SS];
    else
        mv->regs.vec[SS] + 0xFFFFFFFF;
}

// 6to Paso: escribir el Param Segment
// Devuelveel offset donde empeiza argv
uint32_t cargarPS(Configuracion *cfg, Maquina_Virtual *mv, uint32_t base) {
    int cantStrings = cfg->cant_params + 1;
    uint32_t offset = 0;

    for (int i=0; i < cantStrings; i++) {
        char *texto = obtenerString(cfg, i);
        strcpy((char *)&mv->mem.ram[base + offset], texto);
        offset += (uint32_t)strlen(texto) + 1;
    }
    uint32_t offsetArgv = offset;

    // argv: puntero de 4 bytes por cada string
    uint32_t offsetString = 0;
    for (int i = 0; i < cantStrings; i++) {
        escribir4bytes(mv, base + offset, offsetString);
        offset += 4;
        offsetString += (uint32_t)strlen(obtenerString(cfg, i)) + 1;
    }
    return offsetArgv;
}

// 7to Paso: Dejar la pila lista para la subrutina principal
int cargarPilaInicial(Maquina_Virtual *mv, uint32_t argc, uint32_t argv) {
    if (!apilar(mv, argv))
        return 0;
    if(!apilar(mv, argc))
        return 0;
    if(!apilar(mv, 0xFFFFFFFF))
        return 0;
    return 1;
}

int cargarProceso(FILE *arch, Configuracion *cfg, Maquina_Virtual *mv) {
    uint8_t version;
    uint32_t tam[S_CANT] = {0};
    uint32_t base[S_CANT];
    uint32_t entry = 0;
    uint32_t memBytes = cfg->mem_kib * 1024;

    // 1. Cabecera
    if (!leerCabecera(arch, &version, tam, &entry))
        return 0;

    // En la version 1 no habia Data fijo: ocupaba toda la memoria que sobraba despues del codigo
    if (version == 1 && memBytes > tam[S_CS]) {
        tam[S_DS] = memBytes - tam[S_CS];
        if (tam[S_DS] > 0xFFFF)
            tam[S_DS] = 0xFFFF;
    }
    tam[S_PS] = calcularTamPS(cfg);

    // 2. ¿Entra todo?
    if (!verificarEspacio(tam, memBytes, entry))
        return 0;

    if (!inicializarMV(mv, memBytes)) {
        printf("Error: No se pudo reservar la memoria principal.\n");
        return 0;
    }

    // 3. Segmentos, tabla y registros de segmento
    ubicarSegmentos(mv, tam, base);

    // 4. Codigo y constantes
    if (!cargarContenido(arch, mv, tam, base)) {
        printf("Error: Archivo truncado. No se puede leer todo el contenido.\n");
        liberarMV(mv);
        return 0;
    }

    // 5. IP y SP
    inicializarIPySP(mv, tam, entry);

    // 6 y 7. Solo la version 2 tiene Param Segment y pila inicial
    if (version == 2) {
        uint32_t argc = 0;
        uint32_t argv = 0xFFFFFFFF;

        if (tam[S_PS] > 0) {
            argc = cfg->cant_params + 1;
            argv = cargarParamSegment(cfg, mv, base[S_PS]); // el PS es la entrada 0: el puntero es solo el offset
        }
        if (!cargarPilaInicial(mv, argc, argv)) {
            liberarMV(mv);
            return 0;
        }
    }
    return 1;
}

int validacion_arch (const Configuracion *cfg, Maquina_Virtual *mv) {
    FILE *arch = fopen(cfg->ruta_vmx, "rb");
    int ok;

    if (arch == NULL) {
        printf("Error: No se pudo abrir el archivo %s\n", cfg->ruta_vmx);
        return 0;
    }
    ok = cargarProceso(arch, cfg, mv);
    fclose(arch);
    return ok;
}
