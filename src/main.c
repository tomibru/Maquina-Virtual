#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "mv.h"

// 1. Validación de archivo

void validacion_arch (char *ruta_archivo, Maquina_Virtual *mv) {
    FILE *arch = fopen(ruta_archivo, "rb");
    if (arch == NULL)
        printf("Error: No se pudo abrir el archivo %s\n", ruta_archivo);
    else {

        // Leemos los primeros 5 bytes
        char identificador[6];
        fread(identificador, sizeof(char), 5, arch);
        identificador[5] = '\0';

        // Validamos que diga "VMX26"
        if (strncmp(identificador, "VMX26", 5) != 0) 
            printf("Error: Archivo inválido. Identificador incorrecto.\n");
        else {

            //Leemos el sexto byte, que indicia la versión
            uint8_t version;
            fread(&version, sizeof(uint8_t), 1, arch);

            // Validamos que sea version 1
            if (strcmp(version, 1) != 0)
                printf("Error: Archivo inválido. Versión incorrecta.\n");
            else {

                //Leemos el tamaño del código
                uint16_t tamCodigo;
                fread(&tamCodigo, sizeof(uint16_t), 1, arch);

                /*
                El archivo pasó todas las validaciones, por lo que podemos 
                empezar a modificar los valores de la tabla de segmentos de 
                nuentra maquina virtual
                */

                inicializarMV(mv);
                mv->ts.vec[0].base = 0;
                mv->ts.vec[0].tamanio = tamCodigo;
                mv->ts.vec[1].base = tamCodigo;
                mv->ts.vec[1].tamanio = 16384 - tamCodigo;

                //Leemos todo el codigo y lo volcamos en la memoria
                fread(mv->mem.ram, sizeof(uint8_t), tamCodigo, arch);

                //Modificamos los regisros

                //
                mv->regs.vec[26]= 0x00000000;
                mv->regs.vec[27]= 0x00010000;
                mv->regs.vec[0] = mv->regs.vec[27];
            }

        }
    }
    fclose(arch);
}