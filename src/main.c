#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "mv.h"

typedef void (*InstruccionFunc)(Maquina_Virtual *mv);
InstruccionFunc tabla_opc[32]={
    [0x00] = ejecutar_SYS,   // Llamadas al sistema (READ / WRITE)
    [0x0F] = ejecutar_STOP,  // Detener ejecución

    // Un operando - Saltos condicionales e incondicionales
    [0x01] = ejecutar_JMP,   // Salto incondicional
    [0x02] = ejecutar_JP,    // Salto si es Positivo (> 0)
    [0x03] = ejecutar_JN,    // Salto si es Negativo (< 0)
    [0x04] = ejecutar_JZ,    // Salto si es Cero (== 0)
    [0x05] = ejecutar_JC,    // Salto si hay Acarreo (C == 1)
    [0x06] = ejecutar_JV,    // Salto si hay Overflow (V == 1)
    [0x07] = ejecutar_JNP,   // Salto si es Negativo o Cero (<= 0)
    [0x08] = ejecutar_JNN,   // Salto si es Positivo o Cero (>= 0)
    [0x09] = ejecutar_JNZ,   // Salto si no es Cero (!= 0)
    [0x0A] = ejecutar_NOT,   // Negación bit a bit

    // Dos operandos - Operaciones aritméticas, lógicas y especiales
    [0x10] = ejecutar_MOV,   // Asignación / Copia
    [0x11] = ejecutar_ADD,   // Suma
    [0x12] = ejecutar_SUB,   // Resta
    [0x13] = ejecutar_MUL,   // Multiplicación
    [0x14] = ejecutar_DIV,   // División entera y resto
    [0x15] = ejecutar_CMP,   // Comparación (modifica CC)
    [0x16] = ejecutar_AND,   // AND bit a bit
    [0x17] = ejecutar_OR,    // OR bit a bit
    [0x18] = ejecutar_XOR,   // XOR bit a bit
    [0x19] = ejecutar_SWAP,  // Intercambio de valores
    [0x1A] = ejecutar_SHL,   // Desplazamiento a la izquierda
    [0x1B] = ejecutar_SHR,   // Desplazamiento a la derecha sin signo
    [0x1C] = ejecutar_SAR,   // Desplazamiento a la derecha con signo
    [0x1D] = ejecutar_LDL,   // Carga los 2 bytes más bajos
    [0x1E] = ejecutar_LDH,   // Carga los 2 bytes más altos
    [0x1F] = ejecutar_RND    // Número aleatorio
}

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

uint32_t obtener_valor_operando(Maquina_Virtual *mv, uint32_t reg_op){
    uint8_t tipo = (reg_op >> 24) & 0xFF;
    uint32_t valor = reg_op & 0x00FFFFFF;
 
    switch (tipo) {
        case 0: //Ninguno
            return 0;
        
        case 1: //Registro
            return mv->regs.vec[valor & 0x1F];

        case 2:
            if(valor & 0x8000)//Negativo
                return valor | 0xFFFF0000;
            else
                return valor;

        case 3: { //Memoria, direccion logica relativa al DS
            uint8_t cod_reg = valor & 0x1F;
            uint16_t offset = (uint16_t)(valor >> 8);

            //Si no tiene registro se usa el DS por defecto
            uint32_t reg_base = (cod_reg == 0) ? mv->regs.vec[27]:  mv->regs.vec[cod_reg];
            uint32_t dir_logica = reg_base + offset;
            uint32_t dir_fisica = 0;
            
            if(! Traducir_DL_a_DF(mv, dir_logica, 4, 0,&dir_fisica)){
                mv->regs.vec[0] = 0xFFFFFFFF; // Abortar si hay fallo de segmento
                return 0;
            }
            //Leo los 4 bytes
            uint32_t res = 0;
            for(int i=0; i<4 ;i++)
                res = (res << 8) | mv->mem.ram[dir_fisica + i];

            //Actualizo MBR con el valor leido
            mv->regs.vec[6] = res;
            return res;
        }
        default:
            return 0;
    }
}

void guardar_valor_operando(Maquina_Virtual *mv, uint32_t reg_op, uint32_t resultado){
            uint8_t tipo = (reg_op >> 24) & 0xFF;
            uint32_t valor = reg_op & 0x00FFFFFF;
            
            if(tipo == 1)
                mv->regs.vec[ valor & 0x1F ] = resultado;
            else
                if(tipo == 3){
                    uint8_t cod_reg = valor & 0x1F;
                    uint16_t offset = (uint16_t)(valor >> 8);

                    uint32_t reg_base = (cod_reg == 0) ? mv->regs.vec[27] : mv->regs.vec[cod_reg];
                    uint32_t dir_logica = reg_base + offset;
                    uint32_t dir_fisica = 0;
                    
                    if(! Traducir_DL_a_DF(mv, dir_logica, 4, 0,&dir_fisica)){
                        mv->regs.vec[0] = 0xFFFFFFFF; // Abortar si hay fallo de segmento
                        return 0;
                    }

                    //Actualizo MBR
                    mv->regs.vec[6] = resultado;
                    for(int i = 3; i>=0 ; i--){
                        mv->mem.ram[dir_fisica+ i] = resultado & 0xFF;
                        resultado = resultado>> 8;
                    }
                    
                }
}
   

void ejecutarCiclo(Maquina_Virtual *mv){
    uint32_t dir_fisica = 0;

    //La ejecucion del codigo se repite hasta que IP apunte a (-1) tras ejecutar un STOP
    while(mv->regs.vec[0] != 0xFFFFFFFF && Traducir_DL_a_DF(mv, mv->regs.vec[0], 1, 1, &dir_fisica)){
            
        uint32_t ip_actual = mv->regs.vec[0];

        //1- Leer primer byte de la instruccion en la memoria
        uint8_t primer_byte = mv->mem.ram[dir_fisica];

        uint8_t opc = 0;
        uint8_t tipoA = 0;
        uint8_t tipoB = 0;

        //2- Descomponer el byte con mascaras

         // Si el bit 4 está en 1 (0x10), es una instrucción de 2 operandos
        if(primer_byte & 0x10){
            tipoB = (primer_byte >> 6) & 0x03;
            tipoA = (primer_byte >> 4) & 0x03;
            opc = primer_byte & 0x1F;
        }
        else
        // Si los 4 bits bajos son 1 (0x0F), es la instrucción STOP
            if((primer_byte & 0x0F) == 0x0F)
                 opc = 0x0F;
            //Operacion de un operando
            else{//Operacion de un operando
                tipoA = (primer_byte >> 6) & 0x03;
                opc = primer_byte & 0x0F;
            }
        // El tamaño de los operandos en bytes coincide con su código binario (0, 1, 2 o 3).
        // Esto nos servirá para saber cuántos bytes más debemos leer en el siguiente paso.
        uint8_t bytes_a = tipoA;
        uint8_t bytes_b = tipoB;
        
        // 3- Cursor para leer los bytes adicionales de la instrucción (comienza en ip_actual + 1)
        uint8_t cursor_logico = ip_actual +1;
        uint32_t datoA = 0;
        uint32_t datoB = 0;
        int i;
        //Lectura operando b
        for(i=0 ; i < bytes_b ; i++){
            if(!Traducir_DL_a_DF(mv, cursor_logico, 1, 1, &dir_fisica)){
                mv->regs.vec[0] = 0xFFFFFFFF; //Simulamos un IP = STOP para detener el ciclo de lectura
                return;
            }else{
                datoB = (datoB << 8) | mv->mem.ram[dir_fisica];
                cursor_logico++;
            }
        }
        //Lectura operando a
        for(i=0 ; i < bytes_a ; i++){
            if(!Traducir_DL_a_DF(mv, cursor_logico, 1, 1, &dir_fisica)){
                mv->regs.vec[0] = 0xFFFFFFFF; //Simulamos un IP = STOP para detener el ciclo de lectura
                return;
            }else{
                datoA = (datoA << 8) | mv->mem.ram[dir_fisica];
                cursor_logico++;
            }
        }  
        //Actualizo IP para la proxima lectura
        mv->regs.vec[0] = cursor_logico;
        //Actualizo registros OPC, OP1 y OP2
        mv->regs.vec[1] = opc;//OPC
        mv->regs.vec[2] = (tipoA << 24) | (datoA & 0x00FFFFFF) ;//OP1 (Byte mas significativo el tipo de operando y los 3 restantes el dato)
        mv->regs.vec[3] = (tipoB << 24) | (datoB & 0x00FFFFFF) ;//OP2 (Byte mas significativo el tipo de operando y los 3 restantes el dato)
            
    }
}
