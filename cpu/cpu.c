#include "cpu.h"
#include "mmu.h"
#include "pila.h"
#include "imagen.h"
#include <stdlib.h>

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
    [0x0B] = ejecutar_PUSH,  // Apila 4 bytes
    [0x0C] = ejecutar_POP,   // Desapila 4 bytes
    [0x0D] = ejecutar_CALL,  // Llamada a subrutina
    [0x0E] = ejecutar_RET,   // Retorno de subrutina (0 operandos)

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
};

void modificar_flags_CC(Maquina_Virtual *mv, uint32_t resultado, uint8_t c, uint8_t v){
    uint32_t n = (resultado & 0x80000000) ? 1 : 0;
    uint32_t z = (resultado == 0) ? 1 : 0;

    mv->regs.vec[17] =(mv->regs.vec[17] & 0x0FFFFFFF) | ((n << 31) | (z << 30) | (c << 29) | (v << 28));  
}

void ejecutar_STOP(Maquina_Virtual *mv) {
    mv->regs.vec[0] = 0xFFFFFFFF; // Aborta la ejecución
    printf("Deteniendo programa...\n");
}

void ejecutar_JMP(Maquina_Virtual *mv) {
    uint32_t offset = obtener_valor_operando(mv, mv->regs.vec[2]); // OP1
    mv->regs.vec[0] = (mv->regs.vec[0] & 0xFFFF0000) | (offset & 0x0000FFFF);
}

void ejecutar_JP(Maquina_Virtual *mv) { // Positivo (> 0): N=0 y Z=0
    if (!(mv->regs.vec[17] & 0xC0000000)) {
        uint32_t offset = obtener_valor_operando(mv, mv->regs.vec[2]);
        mv->regs.vec[0] = (mv->regs.vec[0] & 0xFFFF0000) | (offset & 0x0000FFFF);
    }
}

void ejecutar_JN(Maquina_Virtual *mv) { // Negativo (< 0): N=1
    if (mv->regs.vec[17] & 0x80000000) {
        uint32_t offset = obtener_valor_operando(mv, mv->regs.vec[2]);
        mv->regs.vec[0] = (mv->regs.vec[0] & 0xFFFF0000) | (offset & 0x0000FFFF);
    }
}

void ejecutar_JZ(Maquina_Virtual *mv) { // Cero (== 0): Z=1
    if (mv->regs.vec[17] & 0x40000000) {
        uint32_t offset = obtener_valor_operando(mv, mv->regs.vec[2]);
        mv->regs.vec[0] = (mv->regs.vec[0] & 0xFFFF0000) | (offset & 0x0000FFFF);
    }
}

void ejecutar_JC(Maquina_Virtual *mv) { // Carry / Acarreo: C=1
    if (mv->regs.vec[17] & 0x20000000) {
        uint32_t offset = obtener_valor_operando(mv, mv->regs.vec[2]);
        mv->regs.vec[0] = (mv->regs.vec[0] & 0xFFFF0000) | (offset & 0x0000FFFF);
    }
}

void ejecutar_JV(Maquina_Virtual *mv) { // Overflow / Desbordamiento: V=1
    if (mv->regs.vec[17] & 0x10000000) {
        uint32_t offset = obtener_valor_operando(mv, mv->regs.vec[2]);
        mv->regs.vec[0] = (mv->regs.vec[0] & 0xFFFF0000) | (offset & 0x0000FFFF);
    }
}

void ejecutar_JNP(Maquina_Virtual *mv) { // Negativo o Cero (<= 0): N=1 o Z=1
    if (mv->regs.vec[17] & 0xC0000000) {
        uint32_t offset = obtener_valor_operando(mv, mv->regs.vec[2]);
        mv->regs.vec[0] = (mv->regs.vec[0] & 0xFFFF0000) | (offset & 0x0000FFFF);
    }
}

void ejecutar_JNN(Maquina_Virtual *mv) { // Positivo o Cero (>= 0): N=0
    if (!(mv->regs.vec[17] & 0x80000000)) {
        uint32_t offset = obtener_valor_operando(mv, mv->regs.vec[2]);
        mv->regs.vec[0] = (mv->regs.vec[0] & 0xFFFF0000) | (offset & 0x0000FFFF);
    }
}

void ejecutar_JNZ(Maquina_Virtual *mv) { // Distinto de cero (!= 0): Z=0
    if (!(mv->regs.vec[17] & 0x40000000)) {
        uint32_t offset = obtener_valor_operando(mv, mv->regs.vec[2]);
        mv->regs.vec[0] = (mv->regs.vec[0] & 0xFFFF0000) | (offset & 0x0000FFFF);
    }
}

void ejecutar_NOT(Maquina_Virtual *mv){
    uint32_t resultado = obtener_valor_operando(mv, mv->regs.vec[2]);
    resultado = ~resultado;
    //Modifico CC
    guardar_valor_operando(mv, mv->regs.vec[2],resultado);
    modificar_flags_CC(mv,resultado,0,0);
}

void ejecutar_MOV(Maquina_Virtual *mv){
    uint32_t op2 = obtener_valor_operando(mv, mv->regs.vec[3]);
    guardar_valor_operando(mv, mv->regs.vec[2],op2);
    modificar_flags_CC(mv, op2, 0,0);
}

void ejecutar_ADD(Maquina_Virtual *mv){
    uint32_t op1 = obtener_valor_operando(mv, mv->regs.vec[2]);
    uint32_t op2 = obtener_valor_operando(mv, mv->regs.vec[3]);
    uint32_t resultado = op1 + op2;
    guardar_valor_operando(mv,mv->regs.vec[2] ,resultado);
    
    // 4. Calcular Acarreo (C): vuelta completa sin signo (resultado menor a un sumando)
    uint8_t c = (resultado < op1) ? 1 : 0;

    // 5. Calcular Overflow (V): incoherencia de signos (Pos+Pos=Neg o Neg+Neg=Pos)
    uint8_t signo_op1 = (op1 >> 31) & 1;
    uint8_t signo_op2 = (op2 >> 31) & 1;
    uint8_t signo_res = (resultado >> 31) & 1;

    uint8_t v = (signo_op1 == signo_op2 && signo_res != signo_op1) ? 1 : 0;
    
    modificar_flags_CC(mv,resultado,c,v);
}

void ejecutar_SUB(Maquina_Virtual *mv){
    uint32_t op1 = obtener_valor_operando(mv, mv->regs.vec[2]);
    uint32_t op2 = obtener_valor_operando(mv, mv->regs.vec[3]);
    uint32_t resultado = op1 - op2;
    guardar_valor_operando(mv,mv->regs.vec[2] ,resultado);
    
    // 4. Calcular Acarreo (C): vuelta completa sin signo (resultado menor a un sumando)
    uint8_t c = (op1 >= op2) ? 1 : 0; // Préstamo (Borrow)

    // 5. Calcular Overflow (V): incoherencia de signos (Pos+Pos=Neg o Neg+Neg=Pos)
    uint8_t signo_op1 = (op1 >> 31) & 1;
    uint8_t signo_op2 = (op2 >> 31) & 1;
    uint8_t signo_res = (resultado >> 31) & 1;

    uint8_t v = (signo_op1 != signo_op2 && signo_res != signo_op1) ? 1 : 0; // Incoherencia de signos
    
    modificar_flags_CC(mv,resultado,c,v);
}

void ejecutar_CMP(Maquina_Virtual *mv){
    uint32_t op1 = obtener_valor_operando(mv, mv->regs.vec[2]);
    uint32_t op2 = obtener_valor_operando(mv, mv->regs.vec[3]);
    uint32_t resultado = op1 - op2;
    
    // 4. Calcular Acarreo (C): vuelta completa sin signo (resultado menor a un sumando)
    uint8_t c = (op1 >= op2) ? 1 : 0; // Préstamo (Borrow)

    // 5. Calcular Overflow (V): incoherencia de signos (Pos+Pos=Neg o Neg+Neg=Pos)
    uint8_t signo_op1 = (op1 >> 31) & 1;
    uint8_t signo_op2 = (op2 >> 31) & 1;
    uint8_t signo_res = (resultado >> 31) & 1;

    uint8_t v = (signo_op1 != signo_op2 && signo_res != signo_op1) ? 1 : 0; // Incoherencia de signos
    
    modificar_flags_CC(mv,resultado,c,v);
}

void ejecutar_MUL(Maquina_Virtual *mv){
    uint32_t op1 = obtener_valor_operando(mv, mv->regs.vec[2]);
    uint32_t op2 = obtener_valor_operando(mv, mv->regs.vec[3]);
    uint32_t resultado = op1 * op2;
    guardar_valor_operando(mv,mv->regs.vec[2] ,resultado);

    uint64_t producto_sin_signo = (uint64_t)op1 * (uint64_t)op2;
    
    //(C): Multiplicación limpia en 64 bits
    uint8_t c = (producto_sin_signo > 0xFFFFFFFF) ? 1 : 0;

    //(V): Evaluamos si el signo del resultado es lógicamente imposible
    int64_t producto_con_signo = (int64_t)(int32_t) op1 * (int64_t)(int32_t) op2;
    uint8_t v = (producto_con_signo < INT32_MIN || producto_con_signo > INT32_MAX) ? 1 : 0;
    modificar_flags_CC(mv,resultado,c,v);
}

void ejecutar_DIV(Maquina_Virtual *mv){
    uint32_t op1 = obtener_valor_operando(mv, mv->regs.vec[2]);
    uint32_t op2 = obtener_valor_operando(mv, mv->regs.vec[3]);
    if(op2 == 0){
        printf("Error: Division por 0.\n");
        mv->regs.vec[0] = 0xFFFFFFFF;
        return;
    }
    
    uint32_t resultado = op1 / op2;
    uint32_t resto = op1 % op2;
    
    guardar_valor_operando(mv,mv->regs.vec[2] ,resultado);
    mv->regs.vec[16] = resto;
    
    modificar_flags_CC(mv,resultado,0,0);
}

void ejecutar_AND(Maquina_Virtual *mv){
    uint32_t op1 = obtener_valor_operando(mv, mv->regs.vec[2]);
    uint32_t op2 = obtener_valor_operando(mv, mv->regs.vec[3]);
    uint32_t resultado = op1 & op2;
    guardar_valor_operando(mv,mv->regs.vec[2] ,resultado);
    
    modificar_flags_CC(mv,resultado,0,0);
}

void ejecutar_OR(Maquina_Virtual *mv){
    uint32_t op1 = obtener_valor_operando(mv, mv->regs.vec[2]);
    uint32_t op2 = obtener_valor_operando(mv, mv->regs.vec[3]);
    uint32_t resultado = op1 | op2;
    guardar_valor_operando(mv,mv->regs.vec[2] ,resultado);
    
    modificar_flags_CC(mv,resultado,0,0);
}

void ejecutar_XOR(Maquina_Virtual *mv){
    uint32_t op1 = obtener_valor_operando(mv, mv->regs.vec[2]);
    uint32_t op2 = obtener_valor_operando(mv, mv->regs.vec[3]);
    uint32_t resultado = op1 ^ op2;
    guardar_valor_operando(mv,mv->regs.vec[2] ,resultado);
    
    modificar_flags_CC(mv,resultado,0,0);
}

void ejecutar_SWAP(Maquina_Virtual *mv) {
    // 1. Obtener los valores actuales de OP1 y OP2
    uint32_t val1 = obtener_valor_operando(mv, mv->regs.vec[2]);
    uint32_t val2 = obtener_valor_operando(mv, mv->regs.vec[3]);

    // 2. Guardar el valor de OP2 en la ubicación de OP1
    guardar_valor_operando(mv, mv->regs.vec[2], val2);

    // 3. Guardar el valor de OP1 en la ubicación de OP2
    guardar_valor_operando(mv, mv->regs.vec[3], val1);

    // 4. El registro CC se modifica según el resultado asignado a OP1 (val2)
    modificar_flags_CC(mv, val2, 0, 0);
}

void ejecutar_SHL(Maquina_Virtual *mv){
    uint32_t op1 = obtener_valor_operando(mv, mv->regs.vec[2]);
    uint32_t op2 = obtener_valor_operando(mv, mv->regs.vec[3]);

    uint32_t resultado;
    uint8_t c, v;

    // Si se desplaza 32 posiciones o mas el registro queda en 0
    if(op2 >= 32) {
        resultado = 0;

        // El acarreo depende del último bit que se callo
        // Si se despalzo exactamente 32, el bit que se cayo era el bit 0 original
        // En casos donde el desplazamiento sea mayor el C quedara en 0
        if (op2 == 32)
            c = op1 & 1;
        else
            c = 0;

        // Si habia unos y se perdio todo hay desbordamiento
        v = (op1 != 0) ? 1: 0;
    }
    else {
        resultado = op1 << op2;

        c = (op2 == 0) ? 0 : ((op1 >> (32 - op2)) & 1);
        
        // Convertimos op1 a int32_t y lo multiplicamos por (2 elevado a la n) en 64 bits.
        int64_t multiplicacion_real = (int64_t)(int32_t)op1 * ((int64_t)1 << op2);
    
        // Si el resultado real se pasa de los límites de 32 bits con signo, hay desbordamiento.
        v = (multiplicacion_real < INT32_MIN || multiplicacion_real > INT32_MAX) ? 1 : 0;
    }

    guardar_valor_operando(mv,mv->regs.vec[2] ,resultado);
    modificar_flags_CC(mv,resultado,c,v);
}

void ejecutar_SHR(Maquina_Virtual *mv){
    uint32_t op1 = obtener_valor_operando(mv, mv->regs.vec[2]);
    uint32_t op2 = obtener_valor_operando(mv, mv->regs.vec[3]);

    uint32_t resultado;
    uint8_t c, v;

    if(op2 >= 32) {
        resultado = 0;
        if (op2 == 32) {
            c = (op1 >> 31) & 1;
            v = (op1 != 0) ? 1 : 0;
        }
    }
    else {

        if(op2 == 0) {
            resultado = op1;
        c = 0;
        v = 0; // no-op: no puede haber overflow
        }

        else  {
            resultado = op1 >> op2;
            // Acarreo: El último bit que salió por la derecha (estaba en la posición op2 - 1)
            c = (op2 == 0) ? 0 : ((op1 >> (op2 - 1)) & 1);
            // Si el número iriginal era negativo (bit 31 en 1), al volverse 0 hay desbordamiento (cambio de signo)
            v = (op1 & 0x80000000) ? 1 : 0;
        }
    }

    guardar_valor_operando(mv,mv->regs.vec[2] ,resultado);
    
    modificar_flags_CC(mv,resultado,c,v);
}

void ejecutar_SAR(Maquina_Virtual *mv){
    uint32_t op1 = obtener_valor_operando(mv, mv->regs.vec[2]);
    uint32_t op2 = obtener_valor_operando(mv, mv->regs.vec[3]);
    
    uint32_t resultado;
    uint8_t c;

    if (op2 >= 32) {
        resultado = (op1 & 0x80000000) ? 0xFFFFFFFF : 0;
        c = (op1 & 0x80000000) ? 1 : 0;
    }
    else {
        int32_t op1_con_signo = (int32_t)op1;
        resultado = (uint32_t)(op1_con_signo >> op2);
        c = (op2 == 0) ? 0 : ((op1 >> (op2-1)) & 1);
    }

    guardar_valor_operando(mv,mv->regs.vec[2] ,resultado);
    modificar_flags_CC(mv,resultado,c,0);
}

void ejecutar_LDH(Maquina_Virtual *mv){
    uint32_t op1 = obtener_valor_operando(mv, mv->regs.vec[2]);
    uint32_t op2 = obtener_valor_operando(mv, mv->regs.vec[3]);
    uint32_t resultado = (op1 & 0x0000FFFF) | ((op2 & 0x0000FFFF) << 16);
    guardar_valor_operando(mv,mv->regs.vec[2] ,resultado);
    
    modificar_flags_CC(mv,resultado,0,0);
}

void ejecutar_LDL(Maquina_Virtual *mv){
    uint32_t op1 = obtener_valor_operando(mv, mv->regs.vec[2]);
    uint32_t op2 = obtener_valor_operando(mv, mv->regs.vec[3]);
    uint32_t resultado = (op1 & 0xFFFF0000) | (op2 & 0x0000FFFF);
    guardar_valor_operando(mv,mv->regs.vec[2] ,resultado);
    
    modificar_flags_CC(mv,resultado,0,0);
}

void ejecutar_RND(Maquina_Virtual *mv) {
    uint32_t op1 = obtener_valor_operando(mv, mv->regs.vec[2]);
    uint32_t op2 = obtener_valor_operando(mv, mv->regs.vec[3]);

    uint32_t resultado = 0;
    if (op2 > 0) {
        resultado = rand() % (op2 + 1); // Rango de 0 a op2 
    }
    guardar_valor_operando(mv, mv->regs.vec[2], resultado);

    modificar_flags_CC(mv, resultado, 0, 0);
}

void ejecutar_SYS(Maquina_Virtual *mv) {
    uint32_t op = obtener_valor_operando(mv, mv->regs.vec[2]);
    uint16_t cant_valores = mv->regs.vec[12] & 0x0000FFFF;
    uint16_t tamanio = (mv->regs.vec[12] >> 16) & 0x0000FFFF;
    uint32_t eax = mv->regs.vec[10];

    // Mantebemos la direccion logica actual iniciada en EDX (reg. 13)
    uint32_t dir_fisica;
    uint32_t dir_logica_actual = mv->regs.vec[13];

    // --- READ (SYS 1) ---
    if (op == 1) { 
       for (int i = 0; i < cant_valores; i++) {

            // Traducimos en CADA iteracion para validar los limites del segmento por cada celda
            if(!Traducir_DL_a_DF(mv, dir_logica_actual, tamanio, 0, &dir_fisica)) {
                printf("Error: fallo de segmento en la lectura. \n");
                mv->regs.vec[0] = STOP;
                return;
            }
            
            printf("[%04X]: ", dir_fisica);
            uint32_t valor = 0;

            // Modos de lectura según los bits de EAX
            if (eax & 0x01) {        // Decimal
                scanf("%u", &valor);
            } else if (eax & 0x02) { // Carácter
                char c;
                scanf(" %c", &c);
                valor = (uint32_t)c;
            } else if (eax & 0x08) { // Hexadecimal
                scanf("%x", &valor);
            } else if (eax & 0x04) { // Octal
                scanf("%o", &valor);
            }

            // Guardado byte por byte (Big Endian)
            for (int b = tamanio - 1; b >= 0; b--) {
                mv->mem.ram[dir_fisica + b] = valor & 0xFF;
                valor >>= 8;
            }
                dir_logica_actual += tamanio;
        }
    } 
        // --- WRITE (SYS 2) ---
    else if (op == 2) { 
        for (int i = 0; i < cant_valores; i++) {

           if(!Traducir_DL_a_DF(mv, dir_logica_actual, tamanio, 0, &dir_fisica)) {
            printf("Error: fallo de segmento en la escritura. \n");
            mv->regs.vec[0] = STOP;
            return;
        }
            printf("[%04X]: ", dir_fisica);

            // Reconstrucción del valor desde la RAM (Big Endian)
            uint32_t valor = 0;
            for (int b = 0; b < tamanio; b++) {
                uint32_t byte_ram = (unsigned char)mv->mem.ram[dir_fisica + b];
                valor = (valor << 8) | byte_ram; // Mueve los bits hacia arriba y suma el nuevo byte
            }

            // Salida acumulativa de formatos según banderas activas en EAX
            if (eax & 0x10) { // Binario
                printf("0b");
                int total_bits = tamanio * 8;
                for (int bit = total_bits - 1; bit >= 0; bit--) {
                    printf("%u", (valor >> bit) & 1);
                }
                printf(" ");
            }
            if (eax & 0x08) { // Hexadecimal
                printf("0x%X ", valor);
            }
            if (eax & 0x04) { // Octal
                printf("0o%o ", valor);
            }
            if (eax & 0x02) { // ASCII
                // Cuando el carácter ASCII no es imprimible, escribe un punto (.) en su lugar
                unsigned char c = (unsigned char)valor;
                printf("%c ", (c >= 32 && c <= 126) ? c : '.');
            }
            if (eax & 0x01) { // Decimal
                printf("%d ", valor);
            }

            printf("\n");
            dir_logica_actual += tamanio;
        }
    } 
    // --- STRING WRITE (SYS 3) ---
    else if (op == 3) {
        uint32_t direccion = (uint32_t)mv->regs.vec[EDX];
        int32_t maximo = mv->regs.vec[ECX];
        uint32_t cantGuardados = 0;
        uint32_t dirFisica;
        int caracter = getchar();

        while(caracter != '\n' && caracter != EOF) {
            if (maximo == -1 || cantGuardados < (uint32_t)maximo) {
                if(!Traducir_DL_a_DF(mv, direccion + cantGuardados, 1, 0, &dirFisica)) {
                    mv->regs.vec[IP] = STOP;
                    return;
                }
                mv->mem.ram[dirFisica] = (uint8_t)caracter;
                cantGuardados++;
            }
            caracter = getchar();
        }

        //Se cierra el String con '\0' para poder imprimirlo despues
        if(!Traducir_DL_a_DF(mv, direccion + cantGuardados, 1, 0, &dirFisica)) {
            mv->regs.vec[IP] = STOP;
            return;
        }
        mv->mem.ram[dirFisica] = 0;
    } 
    // --- STRING WRITE (SYS 4) ---
    else if (op == 4) {
        uint32_t direccion = (uint32_t)mv->regs.vec[EDX];
        uint32_t dirFisica;
        uint8_t caracter;

        do {
            if(!Traducir_DL_a_DF(mv, direccion, 1, 0, &dirFisica)) {
                mv->regs.vec[IP] = STOP;
                return;
            }
            caracter = mv->mem.ram[dirFisica];
            if(caracter != 0)
                putchar(caracter);
            direccion++;
        } while (caracter != 0);
    }
    // --- CLEAR SCREEN (SYS 0) ---
    else if (op == 0) {
        #ifdef _WIN32
            system("cls");
        #else
            system("clear");
        #endif
    }

    else if (op == 0xF)
        ejecutar_breakpoint(mv); // se ignora si no hay .vmi
    
    else {
        printf("Error: Modo de SYS iválido (%u). Se esperaba 1 (READ) o 2 (WRITE). \n", op);
        mv->regs.vec[0] = STOP;
    }
}


// Cantidad de bytes que indica el modificador (3 bits) de un operando de memoria.
// 001 = byte, 010 = word, 100 = long. 000 = programa de la parte I (sin modificador): 4 bytes.
// Devuelve 0 si el codigo es invalido.
static int bytes_modificador(uint8_t cod) {
    int bytes = 0;
    if (cod == 0 || cod == 4) bytes = 4;
    else if (cod == 1)        bytes = 1;
    else if (cod == 2)        bytes = 2;
    return bytes;
}

// Para un operando de memoria calcula cuantos bytes se acceden y a que direccion fisica.
// Devuelve 0 (con mensaje) si el modificador es invalido o hay fallo de segmento.
static int resolver_operando_memoria(Maquina_Virtual *mv, uint32_t valor, int *cant_bytes, uint32_t *dir_fisica) {
    int ok = 0;
    uint8_t cod_reg = valor & 0x1F;
    int16_t offset  = (int16_t)(valor >> 8); // conserva el signo
    *cant_bytes = bytes_modificador((valor >> 5) & 0x07);

    if (*cant_bytes == 0)
        printf("Error: modificador de memoria invalido.\n");
    else {
        // Si no tiene registro (codigo 0) se usa el DS por defecto
        uint32_t reg_base = (cod_reg == 0) ? mv->regs.vec[DS] : mv->regs.vec[cod_reg];
        ok = Traducir_DL_a_DF(mv, reg_base + offset, *cant_bytes, 0, dir_fisica);
    }
    return ok;
}

uint32_t obtener_valor_operando(Maquina_Virtual *mv, uint32_t reg_op){
    uint8_t tipo = (reg_op >> 24) & 0xFF;
    uint32_t valor = reg_op & 0x00FFFFFF;
 
    switch (tipo) {
        case 0: //Ninguno
            return 0;
        
        case 1: { //Registro, con sector: 00 = 4 bytes, 01 = AL, 10 = AH, 11 = AX
            uint32_t r = mv->regs.vec[valor & 0x1F];
            switch ((valor >> 5) & 0x03) {
                case 1:  return (uint32_t)(int32_t)(int8_t)(r & 0xFF);         // 4to byte, con signo
                case 2:  return (uint32_t)(int32_t)(int8_t)((r >> 8) & 0xFF);  // 3er byte, con signo
                case 3:  return (uint32_t)(int32_t)(int16_t)(r & 0xFFFF);      // 2 bytes, con signo
                default: return r;
            }
        }

        case 2:
            if(valor & 0x8000)//Negativo
                return valor | 0xFFFF0000;
            else
                return valor;

        case 3: { //Memoria
            int cant = 0;
            uint32_t dir_fisica = 0;

            if(!resolver_operando_memoria(mv, valor, &cant, &dir_fisica)){
                mv->regs.vec[0] = 0xFFFFFFFF; // Abortar si hay fallo de segmento
                return 0;
            }
            //Leo 'cant' bytes (big endian)
            uint32_t res = 0;
            for(int i=0; i<cant ;i++)
                res = (res << 8) | mv->mem.ram[dir_fisica + i];

            //Extension de signo: byte y word se completan a 4 bytes propagando el signo
            if(cant == 1) res = (uint32_t)(int32_t)(int8_t)res;
            if(cant == 2) res = (uint32_t)(int32_t)(int16_t)res;

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

    if(tipo == 1){
        uint8_t cod = valor & 0x1F;
        uint32_t r = mv->regs.vec[cod];
        switch ((valor >> 5) & 0x03) {
            case 1:  r = (r & 0xFFFFFF00) | (resultado & 0xFF);              break; // AL
            case 2:  r = (r & 0xFFFF00FF) | ((resultado & 0xFF) << 8);       break; // AH
            case 3:  r = (r & 0xFFFF0000) | (resultado & 0xFFFF);            break; // AX
            default: r = resultado;                                          break; // registro completo
        }
        mv->regs.vec[cod] = r;
    }
    else
        if(tipo == 3){
            int cant = 0;
            uint32_t dir_fisica = 0;

            if(!resolver_operando_memoria(mv, valor, &cant, &dir_fisica)){
                mv->regs.vec[0] = 0xFFFFFFFF; // Abortar si hay fallo de segmento
                return;
            }

            //Actualizo MBR
            mv->regs.vec[6] = resultado;
            //Se truncan los bytes mas significativos: solo se guardan 'cant' bytes
            for(int i = cant - 1; i>=0 ; i--){
                mv->mem.ram[dir_fisica+ i] = resultado & 0xFF;
                resultado = resultado>> 8;
            }
        }
}

// ---------------- Manejo de pila ----------------

void ejecutar_PUSH(Maquina_Virtual *mv) {
    // Respeta el orden del enunciado: primero SP-4 y control de overflow...
    if (reservar_pila(mv)) {
        // ...recien despues se obtiene el operando (ya extendido a 4 bytes con signo)
        uint32_t valor = obtener_valor_operando(mv, mv->regs.vec[OP1]);
        if (mv->regs.vec[IP] != (int32_t)STOP) // el operando pudo fallar y abortar
            escribir_tope_pila(mv, valor);
    }
}

void ejecutar_POP(Maquina_Virtual *mv) {
    uint32_t valor;
    if (leer_tope_pila(mv, &valor)) {
        guardar_valor_operando(mv, mv->regs.vec[OP1], valor); // se truncan los bytes de mas
        mv->regs.vec[SP] += 4;                                // el SP se incrementa al final
    }
}

void ejecutar_CALL(Maquina_Virtual *mv) {
    // CALL x  ==  PUSH IP  +  JMP x   (el IP ya apunta a la instruccion siguiente)
    if (apilar(mv, (uint32_t)mv->regs.vec[IP])) {
        uint32_t offset = obtener_valor_operando(mv, mv->regs.vec[OP1]);
        mv->regs.vec[IP] = (mv->regs.vec[IP] & 0xFFFF0000) | (offset & 0x0000FFFF);
    }
}

void ejecutar_RET(Maquina_Virtual *mv) {
    // RET  ==  POP IP
    uint32_t valor;
    if (leer_tope_pila(mv, &valor)) {
        mv->regs.vec[IP] = (int32_t)valor;
        mv->regs.vec[SP] += 4;
    }
}

// 1 si el IP apunta dentro del Code Segment (misma entrada de tabla que CS)
static int ip_en_codigo(const Maquina_Virtual *mv) {
    uint32_t ip = (uint32_t)mv->regs.vec[IP];
    uint32_t cs = (uint32_t)mv->regs.vec[CS];
    return ip != STOP && cs != 0xFFFFFFFFu && (ip >> 16) == (cs >> 16);
}

void ejecutarCiclo(Maquina_Virtual *mv){
    uint32_t dir_fisica = 0;

    //La ejecucion del codigo se repite hasta que IP apunte a (-1) tras ejecutar un STOP
    while(ip_en_codigo(mv) && Traducir_DL_a_DF(mv, mv->regs.vec[0], 1, 1, &dir_fisica)){
            
        uint32_t ip_actual = mv->regs.vec[0];
        int paso_antes = mv->paso_a_paso; // ¿veniamos en modo paso a paso?
        mv->bp_hecho = 0;

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
        uint32_t cursor_logico = ip_actual +1;
        uint32_t datoA = 0;
        uint32_t datoB = 0;
        int i;
        //Lectura operando b
        for(i=0 ; i < bytes_b ; i++){
            if(!Traducir_DL_a_DF(mv, cursor_logico, 1, 1, &dir_fisica)){
                mv->regs.vec[0] = STOP; //Simulamos un IP = STOP para detener el ciclo de lectura
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
        
       if (opc < 32 && tabla_opc[opc] != NULL) {
            tabla_opc[opc](mv); // Ejecuta directamente la función mapeada en la tabla

            // Paso a paso: tras cada instruccion (que no haya sido ella misma un breakpoint) se vuelve a pausar
            if (paso_antes && !mv->bp_hecho && !abortado(mv))
                ejecutar_breakpoint(mv);
        } else {
            printf("Error: Instruccion invalida (Opcode %02X desconocida).\n", opc);
            mv->regs.vec[0] = 0xFFFFFFFF; // Aborta la VM por instrucción inválida
            return;
        }
    }
}