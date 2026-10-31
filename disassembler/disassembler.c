#include <stdio.h>
#include <stdint.h>
#include "disassembler.h"
#include "mmu.h"
#include "mv_const.h"

const char *tabla_mnemonicos[32] = {
    [0x00] = "SYS",  [0x0F] = "STOP",
    [0x01] = "JMP",  [0x02] = "JP",  [0x03] = "JN",  [0x04] = "JZ",
    [0x05] = "JC",   [0x06] = "JV",  [0x07] = "JNP", [0x08] = "JNN",
    [0x09] = "JNZ",  [0x0A] = "NOT",
    [0x0B] = "PUSH", [0x0C] = "POP", [0x0D] = "CALL",[0x0E] = "RET",
    [0x10] = "MOV",  [0x11] = "ADD", [0x12] = "SUB", [0x13] = "MUL",
    [0x14] = "DIV",  [0x15] = "CMP", [0x16] = "AND", [0x17] = "OR",
    [0x18] = "XOR",  [0x19] = "SWAP",[0x1A] = "SHL", [0x1B] = "SHR",
    [0x1C] = "SAR",  [0x1D] = "LDL", [0x1E] = "LDH", [0x1F] = "RND"
};

// Nombres de registro para mostrar operandos de tipo registro/memoria.
// Las posiciones reservadas quedan en NULL.
const char *tabla_nombres_reg[32] = {
    [0] = "IP", [1] = "OPC", [2] = "OP1", [3] = "OP2",
    [4] = "LAR",[5] = "MAR", [6] = "MBR", [7] = "SP", [8] = "BP",
    [10]= "EAX",[11]= "EBX",[12]= "ECX",[13]= "EDX",
    [14]= "EEX",[15]= "EFX",[16]= "AC", [17]= "CC",
    [26]= "CS", [27]= "DS", [28]= "ES", [29]= "SS",
    [30]= "KS", [31]= "PS"
};

// Arma el nombre de un registro segun su sector: EAX / AL / AH / AX (idem EBX... EFX)
void nombre_registro(uint8_t cod_reg, uint8_t sector, char *destino){
    if(cod_reg >= EAX && cod_reg <= EFX && sector != 0){
        char letra = "ABCDEF"[cod_reg - EAX];   // EAX -> 'A', EBX -> 'B', ...
        char sufijo = (sector == 1) ? 'L' : (sector == 2) ? 'H' : 'X';
        sprintf(destino, "%c%c", letra, sufijo);
    }
    else if(tabla_nombres_reg[cod_reg] == NULL)
        sprintf(destino, "R%u", cod_reg);
    else
        sprintf(destino, "%s", tabla_nombres_reg[cod_reg]);
}

// Arma en 'destino' el texto de un operando ya decodificado (tipo + valor crudo)
void formatear_operando(uint8_t tipo, uint32_t valor, char *destino){
    if(tipo == 0){
        destino[0] = '\0';
    }
    else
    if(tipo == 1){ // Registro: 2 bits de sector + 5 bits de codigo
        nombre_registro(valor & 0x1F, (valor >> 5) & 0x03, destino);
    }
    else
    if(tipo == 2){ // Inmediato: 16 bits con signo
        int16_t inmediato = (int16_t)(valor & 0xFFFF);
        sprintf(destino, "%d", inmediato);
    }
    else{ // tipo == 3, Memoria: 16 bits offset + 3 bits tamaño + 5 bits reg
        uint8_t cod_reg = valor & 0x1F;
        int16_t offset = (int16_t)(valor >> 8);
        uint8_t modificador = (valor >> 5) & 0x07;
        const char *prefijo = (modificador == 1) ? "b" : (modificador == 2) ? "w" : (modificador == 4) ? "l" : "";
        char nombre[8];

        if(cod_reg == 0)
            sprintf(nombre, "DS");
        else
            nombre_registro(cod_reg, 0, nombre);

        if(offset >= 0)
            sprintf(destino, "%s[%s+%d]", prefijo, nombre, offset);
        else
            sprintf(destino, "%s[%s-%d]", prefijo, nombre, -offset);
    }
}

void imprimir_linea(int es_entry, uint32_t dir_fisica_inicio, uint8_t *bytes, int cant_bytes,
                            uint8_t opc, uint8_t tipoA, uint32_t datoA,
                            uint8_t tipoB, uint32_t datoB){
    char texto_a[24];
    char texto_b[24];
    int i;

    printf("%s[%04X] ", es_entry ? ">" : "", dir_fisica_inicio);

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

// Muestra las cadenas del Const Segment: [dir] bytes hex | "texto"
void desensamblar_constantes(Maquina_Virtual *mv){
    uint32_t ks = (uint32_t)mv->regs.vec[KS];

    if(ks != 0xFFFFFFFFu){
        Segmento seg = mv->ts.vec[(ks >> 16) & MASCARA_16_BITS];
        uint32_t i = 0;

        while(i < seg.tamanio){
            uint32_t inicio = i;
            uint32_t largo;
            uint32_t j;

            // La cadena llega hasta el '\0' inclusive (o hasta el fin del segmento)
            while(i < seg.tamanio && mv->mem.ram[seg.base + i] != 0)
                i++;
            if(i < seg.tamanio)
                i++; // incluye el '\0'
            largo = i - inicio;

            printf("[%04X] ", seg.base + inicio);

            // Hasta 7 bytes se muestran completos; si hay mas, los primeros 6 y ".."
            for(j = 0; j < largo && j < (largo > 7 ? 6u : 7u); j++){
                if(j > 0) printf(" ");
                printf("%02X", mv->mem.ram[seg.base + inicio + j]);
            }
            if(largo > 7)
                printf(" ..");

            printf(" | \"");
            for(j = 0; j < largo; j++){
                uint8_t c = mv->mem.ram[seg.base + inicio + j];
                if(c == 0)
                    continue; // el terminador no se dibuja
                putchar((c >= 32 && c <= 126) ? c : '.');
            }
            printf("\"\n");
        }
    }
}

void desensamblarCiclo(Maquina_Virtual *mv){
    uint32_t cs = (uint32_t)mv->regs.vec[CS];
    uint32_t ip = (uint32_t)mv->regs.vec[IP];

    desensamblar_constantes(mv);

    if(cs == 0xFFFFFFFFu)
        return;

    Segmento seg_codigo = mv->ts.vec[(cs >> 16) & MASCARA_16_BITS];
    uint32_t offset_actual = 0;

    while(offset_actual < seg_codigo.tamanio){

        uint32_t dir_fisica_inicio = seg_codigo.base + offset_actual;
        uint8_t primer_byte = mv->mem.ram[dir_fisica_inicio];
        uint8_t opc = 0, tipoA = 0, tipoB = 0;
        uint8_t bytes_leidos[8];
        int total_bytes = 0;
        uint32_t cursor = offset_actual + 1;
        uint32_t datoA = 0, datoB = 0;
        int i;

        bytes_leidos[total_bytes++] = primer_byte;

        // Descomponer el byte (mismo criterio que ejecutarCiclo)
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

        if(cursor + tipoA + tipoB > seg_codigo.tamanio){
            printf("[%04X] %02X | Error: instruccion truncada al final del Code Segment\n",
                   dir_fisica_inicio, primer_byte);
            return;
        }

        // Operando B primero (orden invertido, igual que en ejecucion), despues el A
        for(i = 0; i < tipoB; i++){
            uint8_t b = mv->mem.ram[seg_codigo.base + cursor];
            datoB = (datoB << 8) | b;
            bytes_leidos[total_bytes++] = b;
            cursor++;
        }
        for(i = 0; i < tipoA; i++){
            uint8_t b = mv->mem.ram[seg_codigo.base + cursor];
            datoA = (datoA << 8) | b;
            bytes_leidos[total_bytes++] = b;
            cursor++;
        }

        // '>' marca la instruccion donde apunta el IP (el entry point al arrancar)
        imprimir_linea(((ip >> 16) == (cs >> 16)) && ((ip & 0xFFFF) == offset_actual),
                       dir_fisica_inicio, bytes_leidos, total_bytes,
                       opc, tipoA, datoA, tipoB, datoB);

        offset_actual = cursor;
    }
}