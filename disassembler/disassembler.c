#include <stdio.h>
#include <stdint.h>
#include "disassembler.h"
#include "mmu.h"
#include "mv_const.h"

static const char *tabla_mnemonicos[32] = {
    [0x00] = "SYS",  [0x0F] = "STOP",
    [0x01] = "JMP",  [0x02] = "JP",  [0x03] = "JN",  [0x04] = "JZ",
    [0x05] = "JC",   [0x06] = "JV",  [0x07] = "JNP", [0x08] = "JNN",
    [0x09] = "JNZ",  [0x0A] = "NOT",
    [0x10] = "MOV",  [0x11] = "ADD", [0x12] = "SUB", [0x13] = "MUL",
    [0x14] = "DIV",  [0x15] = "CMP", [0x16] = "AND", [0x17] = "OR",
    [0x18] = "XOR",  [0x19] = "SWAP",[0x1A] = "SHL", [0x1B] = "SHR",
    [0x1C] = "SAR",  [0x1D] = "LDL", [0x1E] = "LDH", [0x1F] = "RND"
};

// Nombres de registro para mostrar operandos de tipo registro/memoria.
// Las posiciones reservadas quedan en NULL.
static const char *tabla_nombres_reg[32] = {
    [0] = "IP", [1] = "OPC", [2] = "OP1", [3] = "OP2",
    [4] = "LAR",[5] = "MAR", [6] = "MBR",
    [10]= "EAX",[11]= "EBX",[12]= "ECX",[13]= "EDX",
    [14]= "EEX",[15]= "EFX",[16]= "AC", [17]= "CC",
    [26]= "CS", [27]= "DS"
};

// Arma en 'destino' el texto de un operando ya decodificado (tipo + valor crudo)
static void formatear_operando(uint8_t tipo, uint32_t valor, char *destino){
    if(tipo == 0){
        destino[0] = '\0';
    }
    else
    if(tipo == 1){ // Registro: 5 bits de codigo en la parte baja
        uint8_t cod_reg = valor & 0x1F;
        const char *nombre = tabla_nombres_reg[cod_reg];
        if(nombre == NULL)
            sprintf(destino, "R%u", cod_reg);
        else
            sprintf(destino, "%s", nombre);
    }
    else
    if(tipo == 2){ // Inmediato: 16 bits con signo
        int16_t inmediato = (int16_t)(valor & 0xFFFF);
        sprintf(destino, "%d", inmediato);
    }
    else{ // tipo == 3, Memoria: 16 bits offset + 3 reservados + 5 bits reg
        uint8_t cod_reg = valor & 0x1F;
        int16_t offset = (int16_t)(valor >> 8);
        const char *nombre = (cod_reg == 0) ? "DS" : tabla_nombres_reg[cod_reg];

        if(nombre == NULL)
            nombre = "R?";

        if(offset >= 0)
            sprintf(destino, "[%s+%d]", nombre, offset);
        else
            sprintf(destino, "[%s-%d]", nombre, -offset);
    }
}

static void imprimir_linea(uint32_t dir_fisica_inicio, uint8_t *bytes, int cant_bytes,
                            uint8_t opc, uint8_t tipoA, uint32_t datoA,
                            uint8_t tipoB, uint32_t datoB){
    char texto_a[16];
    char texto_b[16];
    int i;

    printf("[%04X] ", dir_fisica_inicio);

    i = 0;
    while(i < cant_bytes){
        printf("%02X ", bytes[i]);
        i++;
    }

    printf("| %s", tabla_mnemonicos[opc]);

    formatear_operando(tipoA, datoA, texto_a);
    formatear_operando(tipoB, datoB, texto_b);

    if(tipoA != 0 && tipoB != 0)
        printf(" %s, %s\n", texto_a, texto_b);
    else
    if(tipoA != 0)
        printf(" %s\n", texto_a);
    else
        printf("\n");
}

void desensamblarCiclo(Maquina_Virtual *mv){
    uint16_t tam_codigo = (uint16_t)mv->ts.vec[0].tamanio;
    uint32_t offset_actual = 0;

    while(offset_actual < tam_codigo){

        uint32_t dir_fisica;
        uint32_t dir_fisica_inicio;
        uint8_t primer_byte;
        uint8_t opc = 0, tipoA = 0, tipoB = 0;
        uint8_t bytes_leidos[8];
        int total_bytes = 0;
        uint32_t cursor_logico;
        uint32_t datoA = 0, datoB = 0;
        int i;

        // 1- Leer primer byte de la instruccion
        if(!Traducir_DL_a_DF(mv, offset_actual, 1, 1, &dir_fisica)){
            printf("Error: fallo de segmento desensamblando en offset %u\n", offset_actual);
            return;
        }
        dir_fisica_inicio = dir_fisica;
        primer_byte = mv->mem.ram[dir_fisica];
        bytes_leidos[total_bytes++] = primer_byte;

        // 2- Descomponer el byte (mismo criterio que ejecutarCiclo)
        if(primer_byte & 0x10){
            tipoB = (primer_byte >> 6) & 0x03;
            tipoA = (primer_byte >> 4) & 0x03;
            opc = primer_byte & 0x1F;
        }
        else
        if((primer_byte & 0x0F) == 0x0F)
            opc = 0x0F;
        else{
            tipoA = (primer_byte >> 6) & 0x03;
            opc = primer_byte & 0x0F;
        }

        if(opc >= 32 || tabla_mnemonicos[opc] == NULL){
            printf("[%04X] %02X | Error: Instruccion invalida (Opcode %02X)\n",
                   dir_fisica_inicio, primer_byte, opc);
            return;
        }

        uint8_t bytes_b = tipoB;
        uint8_t bytes_a = tipoA;
        cursor_logico = offset_actual + 1;

        // 3- Leer operando B primero (orden invertido, igual que en ejecucion)
        i = 0;
        while(i < bytes_b){
            if(!Traducir_DL_a_DF(mv, cursor_logico, 1, 1, &dir_fisica)){
                printf("Error: fallo de segmento desensamblando en offset %u\n", cursor_logico);
                return;
            }
            uint8_t b = mv->mem.ram[dir_fisica];
            datoB = (datoB << 8) | b;
            bytes_leidos[total_bytes++] = b;
            cursor_logico++;
            i++;
        }

        // 4- Leer operando A
        i = 0;
        while(i < bytes_a){
            if(!Traducir_DL_a_DF(mv, cursor_logico, 1, 1, &dir_fisica)){
                printf("Error: fallo de segmento desensamblando en offset %u\n", cursor_logico);
                return;
            }
            uint8_t b = mv->mem.ram[dir_fisica];
            datoA = (datoA << 8) | b;
            bytes_leidos[total_bytes++] = b;
            cursor_logico++;
            i++;
        }

        imprimir_linea(dir_fisica_inicio, bytes_leidos, total_bytes,
                       opc, tipoA, datoA, tipoB, datoB);

        offset_actual = cursor_logico;
    }
}