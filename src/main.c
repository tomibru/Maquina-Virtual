#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "mv.h"

void validacion_arch (char *ruta_archivo, Maquina_Virtual *mv);


int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("Error: Debes ingresar el nombre del archivo. Uso: %s <archivo.vmx>\n", argv[0]);
        return 1;
    }
    
    Maquina_Virtual mv;
    validacion_arch(argv[1], &mv); //Valida VMX26
}



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

            //Leemos el sexto byte, que indica la versión
            uint8_t version;
            fread(&version, sizeof(uint8_t), 1, arch);

            // Validamos que sea version 1
            if (version!=1)
                printf("Error: Archivo inválido. Versión incorrecta.\n");
            else {
                inicializarMV(mv);
                //Leemos el tamaño del código
                uint16_t tamCodigo;
                fread(&tamCodigo, sizeof(uint16_t), 1, arch);

                /*
                El archivo pasó todas las validaciones, por lo que podemos 
                empezar a modificar los valores de la tabla de segmentos de 
                nuestra maquina virtual
                */

                mv->ts.vec[0].base = 0;
                mv->ts.vec[0].tamanio = tamCodigo;
                mv->ts.vec[1].base = tamCodigo;
                mv->ts.vec[1].tamanio = 16384 - tamCodigo;

                //Leemos todo el codigo y lo volcamos en la memoria
                fread(mv->mem.ram, sizeof(uint8_t), tamCodigo, arch);

                //Modificamos los regisros

                //
                mv->regs.vec[26]= 0x00000000; //CS
                mv->regs.vec[27]= 0x00010000; //DS
                mv->regs.vec[0] = mv->regs.vec[26]; //IP = CS
            }

        }
        fclose(arch);
    }
}


//Fase 2: Traductor de Direcciones (MMU) y Validacio*nes
//FLUJO DE LA MMU (Memory Managment Unit)
//Filtro de seguridad de la CPU para interactura con la Memoria RAM

int Traducir_DL_a_DF(Maquina_Virtual *mv, uint32_t dir_logica, uint16_t cant_bytes, int Codesegment, uint32_t *dir_fisica_calculada){
    //Descomposicion de la direccion logica(IR)
    uint16_t segmento=(dir_logica >> 16) & 0xFFFF;
    uint16_t desplazamiento= dir_logica & 0xFFFF;
    int devolucion=0;
    //Primera validacion, segmento excede la tabla de segmentos

    if(segmento>=8){
        printf("Error: Fallo de segmento. El segmento %u excede el tamaño de la tabla.\n", segmento);
    }
    else{
        //Segunda validacion, entrada deshabilitada o erronea segmento=-1
        if(mv->ts.vec[segmento].base==0xFFFF || mv->ts.vec[segmento].tamanio==0xFFFF){
            printf("Error: Fallo de segmento. El segmento %u no esta inicializado.\n", segmento);
        }
        else{
            //Tercera validacion, direccion fisica fuera del segmento
            if((uint32_t)desplazamiento + cant_bytes > (uint32_t)mv->ts.vec[segmento].tamanio){
                //Solo muestra mensaje de error si es acceso a los datos.
                //Si es lectura de instruccion(codesegment=1), no imprime error y devuelve 0 para terminar el bucle
                if(!Codesegment)     
                    printf("Error: Fallo del segmento, acceso fuera de los limites en segmento %u\n",segmento);
            }
            else{
                //Traduccion a direccion fisica
                uint32_t dir_fisica=(uint32_t)mv->ts.vec[segmento].base + desplazamiento;

                //Registro de estado, si estamos en DataSegment (Actualizamos LAR y MAR)
                if(!Codesegment){
                    mv->regs.vec[4]= dir_logica; //LAR
                    mv->regs.vec[5]= ((uint32_t)cant_bytes<<16) | (dir_fisica & 0xFFFF); //MAR parte alta: cantidad de bytes que se quieren acceder, parte baja: direccion fisica
                }

                *dir_fisica_calculada=dir_fisica;
                devolucion=1;
            }
        }
    }
    return devolucion;
}