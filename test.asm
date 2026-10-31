start: MOV EAX, 0x7FFF             ; Carga un valor positivo cercano al limite de 16 bits.
    LDH EAX, 0x7FFF             ; Forma EAX = 0x7FFF7FFF sin depender de inmediato de 32 bits.
    ADD EAX, 1                  ; Suma 1: el resultado firmado no desborda 32 bits.
    MOV EAX, 0x7FFF             ; Reinicia el acumulador para una suma con overflow real.
    LDH EAX, 0xFFFF             ; Forma EAX = 0x7FFFFFFF (INT32_MAX).
    ADD EAX, 1                  ; Esperado: EAX=0x80000000 y V=1.
    JV mul_test                 ; Confirma V mediante un salto condicional.
    JMP fail_v_add              ; Si no salta, el flag V de ADD no se activo.

mul_test: MOV EAX, 0x4000             ; Carga la mitad baja del multiplicando.
    LDH EAX, 0x0000             ; Forma EAX = 0x00004000.
    MOV EBX, 2                  ; Carga el multiplicador.
    MUL EAX, EBX                ; El producto 32768 cabe en 32 bits; V debe quedar apagado.
    MOV EAX, 0x7FFF             ; Carga la mitad baja de INT32_MAX.
    LDH EAX, 0xFFFF             ; EAX = 0x7FFFFFFF.
    MUL EAX, 2                  ; INT32_MAX*2 excede INT32_MAX: debe activar V.
    JV sub_test                 ; Verifica el overflow firmado de MUL.
    JMP fail_v_mul              ; Informa que MUL no activo V.

sub_test: MOV EAX, 1                  ; Carga el minuendo.
    SUB EAX, 2                  ; 1-2 necesita prestamo: segun la especificacion C debe indicar prestamo.
    JC shift_test               ; Verifica la interpretacion de carry implementada por la VM.
    JMP fail_c_sub              ; Informa si no se observa el carry esperado.

shift_test: MOV EAX, 0x4000             ; Prepara un bit alto para SHL.
    LDH EAX, 0x0000             ; EAX = 0x00004000.
    SHL EAX, 1                  ; Expulsa el bit 31? No; resultado 0x8000, carry=0.
    MOV EAX, 0x0001             ; Prepara un bit que sale por SHL al desplazar 32 lugares.
    SHL EAX, 32                 ; Caso limite: el codigo C define C como bit 0 original.
    JC shift_right              ; Comprueba el carry de SHL 32.
    JMP fail_c_shl              ; Informa si no se activo carry.

shift_right: MOV EAX, 3                  ; Prepara bits bajos para forzar un bit expulsado por SHR.
    SHR EAX, 1                  ; Esperado: EAX=1 y C=1 por el bit 0 expulsado.
    JC sar_test                 ; Comprueba el carry de SHR.
    JMP fail_c_shr              ; Informa si SHR no activo carry.
sar_test: MOV EAX, 0x8000             ; Restaura el valor negativo.
    LDH EAX, 0x0000             ; EAX = 0x80000000.
    SAR EAX, 1                  ; Esperado: EAX=0xC0000000 y C=0.
    MOV EBX, 0x00FF             ; Valor de mascara para AND/OR/XOR.
    AND EAX, EBX                ; Prueba AND.
    OR EAX, EBX                 ; Prueba OR.
    XOR EAX, EBX                ; Prueba XOR.
    JMP partial_test            ; Sigue con sectores parciales del registro.

partial_test: MOV EAX, 0x1234             ; Carga palabra baja inicial.
    LDH EAX, 0x5678             ; EAX = 0x12345678.
    MOV AL, 0xAA                ; Cambia solo el byte bajo; esperado EAX=0x123456AA.
    MOV AH, 0xBB                ; Cambia solo el byte siguiente; esperado EAX=0x1234BBAA.
    MOV AX, 0xCCDD              ; Cambia solo los 16 bits bajos; esperado EAX=0x1234CCDD.
    JMP done                    ; Finaliza exitosamente.

fail_v_add: MOV EAX, 0xBAD1             ; Codigo de estado: ADD no activo V.
    JMP done                    ; Detiene la prueba en el final.
fail_v_mul: MOV EAX, 0xBAD2             ; Codigo de estado: MUL no activo V.
    JMP done                    ; Detiene la prueba en el final.
fail_c_sub: MOV EAX, 0xBAD3             ; Codigo de estado: SUB no activo C con prestamo.
    JMP done                    ; Detiene la prueba en el final.
fail_c_shl: MOV EAX, 0xBAD4             ; Codigo de estado: SHL no activo C.
    JMP done                    ; Detiene la prueba en el final.
fail_c_shr: MOV EAX, 0xBAD5             ; Codigo de estado: SHR no activo C.

done: SYS 0xF                     ; Breakpoint; requiere ejecutar con archivo .vmi para detenerse aqui.
    RET                         ; Retorna de main; la pila inicial debe contener -1.
