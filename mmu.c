#include "mmu.h"

//Fase 2: Traductor de Direcciones (MMU) y Validacio*nes
//FLUJO DE LA MMU (Memory Managment Unit)
//Filtro de seguridad de la CPU para interactura con la Memoria RAM

int Traducir_DL_a_DF(Maquina_Virtual *mv, uint32_t dir_logica, uint16_t cant_bytes, int Codesegment, uint32_t *dir_fisica_calculada) {
    
    //Descomposicion de la direccion logica(IR)
    uint16_t segmento = (dir_logica >> 16) & MASCARA_16_BITS;
    uint16_t desplazamiento= dir_logica & MASCARA_16_BITS;
    int devolucion=0;

    //Primera validacion. Segmento excede la tabla de segmentos
    if(segmento >= CANT_SEGMENTOS){
        printf("Error: Fallo de segmento. El segmento %u excede el tamaño de la tabla.\n", segmento);
    }
    else{

        //Segunda validacion. Entrada deshabilitada o erronea, segmento = -1
        if(mv->ts.vec[segmento].base == SEGMENTO_LIBRE || mv->ts.vec[segmento].tamanio == SEGMENTO_LIBRE) {
            printf("Error: Fallo de segmento. El segmento %u no esta inicializado.\n", segmento);
        }
        else{

            //Tercera validacion. Direccion fisica fuera del segmento
            if((uint32_t)desplazamiento + cant_bytes > (uint32_t)mv->ts.vec[segmento].tamanio) {
                //Solo muestra mensaje de error si es acceso a los datos.
                //Si es lectura de instruccion(codesegment=1), no imprime error y devuelve 0 para terminar el bucle
                if(!Codesegment)     
                    printf("Error: Fallo del segmento, acceso fuera de los limites en segmento %u\n",segmento);
            }
            else{
                //Traduccion a direccion fisica
                uint32_t dir_fisica = (uint32_t)mv->ts.vec[segmento].base + desplazamiento;

                //Registro de estado, si estamos en DataSegment (Actualizamos LAR y MAR)
                if(!Codesegment){
                    mv->regs.vec[4]= dir_logica; //LAR
                    mv->regs.vec[5]= ((uint32_t)cant_bytes << 16) | (dir_fisica & MASCARA_16_BITS); //MAR parte alta: cantidad de bytes que se quieren acceder, parte baja: direccion fisica
                }

                *dir_fisica_calculada=dir_fisica;
                devolucion=1;
            }
        }
    }
    return devolucion;
}