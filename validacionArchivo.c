#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "componentes/mv.h"

int validacion_arch (char *ruta_archivo, Maquina_Virtual *mv) {
    FILE *arch = fopen(ruta_archivo, "rb");
    int ok = 0;
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

            //Leemos el sexto byte, que indica la versión
            uint8_t version;
            fread(&version, sizeof(uint8_t), 1, arch);

            // Validamos que sea version 1
            if (version!=1)
                printf("Error: Archivo inválido. Versión incorrecta.\n");
            else {

                //Leemos el tamaño del código
                uint16_t tamCodigo;

                //Validamos que se pueda leer correctamente
                if (fread(&tamCodigo, sizeof(uint16_t), 1, arch != 1))
                    printf("Error: Archivo inválido. No se pudo leer el tamaño del código.\n");
                else {

                    //Validamos que el códifo no sobrepase el tamaño de la memoria de la máquian virtual.
                    if (tamCodigo > TAM_MEM)
                        printf("Error: Archivo inválido. Tamaño de código (%u) excede la memoria (%d).\n");
                    else {
                        
                        uint8_t bufferCodigo[TAM_MEM];
                        size_t leidos = 0;

                        //Leemos todo el codigo y lo volcamos el buffer
                        if (tamCodigo > 0) 
                            leidos = fread(bufferCodigo, sizeof(uint8_t), tamCodigo, arch);
                        
                        if (leidos != tamCodigo)
                            printf("Error: Archivo truncado. No se puede leer todo el código.\n");
                        else {

                        inicializarMV(mv);

                            /*
                            El archivo pasó todas las validaciones, por lo que podemos 
                            empezar a modificar los valores de la tabla de segmentos de 
                            nuestra maquina virtual
                            */
                           
                            // Inicializamos la tabla de segmentos
                            mv->ts.vec[0].base = 0;
                            mv->ts.vec[0].tamanio = tamCodigo;
                            mv->ts.vec[1].base = tamCodigo;
                            mv->ts.vec[1].tamanio = 16384 - tamCodigo;

                            // Volcamos en memoria el código
                            memcpy(mv->mem.ram, bufferCodigo, tamCodigo);
                            
                            //Modificamos los regisros
                            mv->regs.vec[26]= 0x00000000; //CS
                            mv->regs.vec[27]= 0x00010000; //DS
                            mv->regs.vec[0] = mv->regs.vec[26]; //IP = CS
                            ok = 1;
                        }
                    }
                }
            }

        }
        fclose(arch);
    }
    return ok;
}