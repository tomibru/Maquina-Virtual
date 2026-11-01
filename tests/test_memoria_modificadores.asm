; =====================================================================
; test_memoria_modificadores.asm  -  accesos b / w / l, signo, truncado, bordes, DS <-> ES
; AUTOVERIFICABLE: cada caso imprime 'OK' o 'FALLO'. Resultado esperado: TODO 'OK' y fin silencioso.
; Memoria en BIG ENDIAN: el byte MAS significativo va en la direccion MAS baja.
; =====================================================================
\\DATA 32                          ; Data de SOLO 32 bytes: asi los bordes (ds+28..ds+31) son los reales
\\EXTRA 64                         ; Extra: ES+0..31 para las pruebas, ES+56 y ES+60 para el verificador
\\STACK 128                        ; pila para los CALL

; ---- textos (Const Segment) ----
s_ok   EQU "OK     "
s_fa   EQU "FALLO  "
s_nl   EQU "\n"
s_hdr  EQU "   (obtenido, esperado):\n"
n_m1    EQU "l[ds+0] devuelve el valor completo"
n_m2    EQU "b[ds+0] = 0x11 (1er byte = el MAS significativo)"
n_m3    EQU "b[ds+1] = 0x22 (el SEGUNDO byte)"
n_m4    EQU "b[ds+3] = 0x44 (el ultimo byte)"
n_m5    EQU "w[ds+0] = 0x1122 (OJO: en Big Endian esta es la mitad ALTA)"
n_m6    EQU "w[ds+2] = 0x3344 (la mitad BAJA del long)"
n_m7    EQU "lectura NO alineada: w[ds+1] = 0x2233"
n_s1    EQU "b[ds+4]=0x80 se extiende a 0xFFFFFF80"
n_s2    EQU "b[ds+5]=0xFF se extiende a 0xFFFFFFFF"
n_s3    EQU "b[ds+6]=0x7F queda positivo 0x0000007F"
n_s4    EQU "w[ds+4]=0x80FF se extiende a 0xFFFF80FF"
n_s5    EQU "w[ds+6]=0x7F01 queda positivo"
n_s6    EQU "lectura l NO alineada: l[ds+1] = 0x22334480"
n_t1    EQU "b[ds+8] <- EAX guarda solo 0x44: l[ds+8] = 0x44000000"
n_t2    EQU "w[ds+12] <- EAX guarda solo 0x3344: l[ds+12] = 0x33440000"
n_t3    EQU "b en el medio: l[ds+16] = 0xAA11CCDD"
n_t4    EQU "w en la mitad baja: l[ds+16] = 0xAA112222"
n_t5    EQU "b <- -1 escribe 0xFF y nada mas: l[ds+20] = 0xFF000000"
n_t6    EQU "add b desborda sin acarrear: l[ds+24] = 0xAA00BBCC"
n_d1    EQU "l[edx-4] (offset negativo) = l[ds+4] = 0x80FF7F01"
n_d2    EQU "l[edx+4] = l[ds+12] = 0x33440000"
n_e1    EQU "l[ds+28] en el borde: se escribe y se lee"
n_e2    EQU "b[ds+31] = 0x7E (ultimo byte del segmento)"
n_e3    EQU "w[ds+30] = 0x1234 (ultima word)"
n_e4    EQU "el borde NO invadio el Extra Segment: b[es+0] sigue en 0"
n_x1    EQU "DS -> ES con l: l[es+0] = 0x11223344"
n_x2    EQU "DS(l) -> ES(b): b[es+4] = 0x44"
n_x3    EQU "DS(l) -> ES(w): w[es+6] = 0x3344"
n_x4    EQU "DS(w) -> ES(w): w[es+8] = 0x1122"
n_x5    EQU "SWAP DS<->ES: l[ds+0] = 0x00007788"
n_x6    EQU "SWAP DS<->ES: l[es+16] = 0x11223344"
n_x7    EQU "DS no se altero por escribir en ES: l[ds+4] = 0x80FF7F01"

; ============================ PROGRAMA ============================
main: mov eex, 0                     ; (arranque)

; ============ A) escribir 32 bits con l[...] y leerlos de a partes (BIG ENDIAN: el byte mas significativo va en la direccion mas baja) ============
      mov eax, 0                      ; EAX = 0x11223344 (se arma con ldh/ldl)
      ldh eax, 0x1122                 ; parte alta = 0x1122
      ldl eax, 0x3344                 ; parte baja = 0x3344
      mov l[ds+0], eax                ; escribe los 4 bytes en DS+0..3: 11 22 33 44
; ---- l[ds+0] devuelve el valor completo
      mov eax, l[ds+0]                ; lee 4 bytes
      mov ebx, 0                      ; esperado = 0x11223344 (se arma con ldh/ldl)
      ldh ebx, 0x1122                 ; parte alta = 0x1122
      ldl ebx, 0x3344                 ; parte baja = 0x3344
      mov efx, n_m1                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
; ---- b[ds+0] = 0x11 (1er byte = el MAS significativo)
      mov eax, b[ds+0]                ; lee 1 byte
      mov ebx, 17                     ; esperado = 0x11
      mov efx, n_m2                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
; ---- b[ds+1] = 0x22 (el SEGUNDO byte)
      mov eax, b[ds+1]                ; lee 1 byte
      mov ebx, 34                     ; esperado = 0x22
      mov efx, n_m3                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
; ---- b[ds+3] = 0x44 (el ultimo byte)
      mov eax, b[ds+3]                ; lee 1 byte
      mov ebx, 68                     ; esperado = 0x44
      mov efx, n_m4                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
; ---- w[ds+0] = 0x1122 (OJO: en Big Endian esta es la mitad ALTA)
      mov eax, w[ds+0]                ; lee 2 bytes
      mov ebx, 4386                   ; esperado = 0x1122
      mov efx, n_m5                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
; ---- w[ds+2] = 0x3344 (la mitad BAJA del long)
      mov eax, w[ds+2]                ; lee 2 bytes
      mov ebx, 13124                  ; esperado = 0x3344
      mov efx, n_m6                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
; ---- lectura NO alineada: w[ds+1] = 0x2233
      mov eax, w[ds+1]                ; word que empieza en el byte 1
      mov ebx, 8755                   ; esperado = 0x2233
      mov efx, n_m7                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO

; ============ B) extension de signo al LEER b / w ============
      mov eax, 0                      ; EAX = 0x80FF7F01 (se arma con ldh/ldl)
      ldh eax, 0x80FF                 ; parte alta = 0x80FF
      ldl eax, 0x7F01                 ; parte baja = 0x7F01
      mov l[ds+4], eax                ; DS+4..7 = 80 FF 7F 01
; ---- b[ds+4]=0x80 se extiende a 0xFFFFFF80
      mov eax, b[ds+4]                ; byte negativo
      mov ebx, -128                   ; esperado = 0xFFFFFF80
      mov efx, n_s1                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
; ---- b[ds+5]=0xFF se extiende a 0xFFFFFFFF
      mov eax, b[ds+5]                ; byte negativo
      mov ebx, -1                     ; esperado = 0xFFFFFFFF
      mov efx, n_s2                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
; ---- b[ds+6]=0x7F queda positivo 0x0000007F
      mov eax, b[ds+6]                ; byte positivo
      mov ebx, 127                    ; esperado = 0x7F
      mov efx, n_s3                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
; ---- w[ds+4]=0x80FF se extiende a 0xFFFF80FF
      mov eax, w[ds+4]                ; word negativa
      mov ebx, -32513                 ; esperado = 0xFFFF80FF
      mov efx, n_s4                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
; ---- w[ds+6]=0x7F01 queda positivo
      mov eax, w[ds+6]                ; word positiva
      mov ebx, 32513                  ; esperado = 0x7F01
      mov efx, n_s5                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
; ---- lectura l NO alineada: l[ds+1] = 0x22334480
      mov eax, l[ds+1]                ; bytes 22 33 44 80
      mov ebx, 0                      ; esperado = 0x22334480 (se arma con ldh/ldl)
      ldh ebx, 0x2233                 ; parte alta = 0x2233
      ldl ebx, 0x4480                 ; parte baja = 0x4480
      mov efx, n_s6                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO

; ============ C) TRUNCAR al ESCRIBIR b / w (solo se guardan los bytes bajos del valor) ============
      mov eax, 0                      ; EAX = 0x11223344 (se arma con ldh/ldl)
      ldh eax, 0x1122                 ; parte alta = 0x1122
      ldl eax, 0x3344                 ; parte baja = 0x3344
      mov l[ds+8], 0                  ; DS+8..11 = 0
      mov b[ds+8], eax                ; escribe SOLO el byte bajo (0x44) en DS+8
; ---- b[ds+8] <- EAX guarda solo 0x44: l[ds+8] = 0x44000000
      mov eax, l[ds+8]                ; lee el long
      mov ebx, 0                      ; esperado = 0x44000000 (se arma con ldh/ldl)
      ldh ebx, 0x4400                 ; parte alta = 0x4400
      mov efx, n_t1                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
      mov eax, 0                      ; EAX = 0x11223344 (se arma con ldh/ldl)
      ldh eax, 0x1122                 ; parte alta = 0x1122
      ldl eax, 0x3344                 ; parte baja = 0x3344
      mov l[ds+12], 0                 ; DS+12..15 = 0
      mov w[ds+12], eax               ; escribe SOLO la word baja (0x3344) en DS+12..13
; ---- w[ds+12] <- EAX guarda solo 0x3344: l[ds+12] = 0x33440000
      mov eax, l[ds+12]               ; lee el long
      mov ebx, 0                      ; esperado = 0x33440000 (se arma con ldh/ldl)
      ldh ebx, 0x3344                 ; parte alta = 0x3344
      mov efx, n_t2                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
      mov eax, 0                      ; EAX = 0xAABBCCDD (se arma con ldh/ldl)
      ldh eax, 0xAABB                 ; parte alta = 0xAABB
      ldl eax, 0xCCDD                 ; parte baja = 0xCCDD
      mov l[ds+16], eax               ; DS+16..19 = AA BB CC DD
      mov b[ds+17], 0x11              ; pisa SOLO el 2do byte
; ---- b en el medio: l[ds+16] = 0xAA11CCDD
      mov eax, l[ds+16]               ; lee el long
      mov ebx, 0                      ; esperado = 0xAA11CCDD (se arma con ldh/ldl)
      ldh ebx, 0xAA11                 ; parte alta = 0xAA11
      ldl ebx, 0xCCDD                 ; parte baja = 0xCCDD
      mov efx, n_t3                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
      mov w[ds+18], 0x2222            ; pisa SOLO los 2 bytes de abajo
; ---- w en la mitad baja: l[ds+16] = 0xAA112222
      mov eax, l[ds+16]               ; lee el long
      mov ebx, 0                      ; esperado = 0xAA112222 (se arma con ldh/ldl)
      ldh ebx, 0xAA11                 ; parte alta = 0xAA11
      ldl ebx, 0x2222                 ; parte baja = 0x2222
      mov efx, n_t4                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
      mov l[ds+20], 0                 ; DS+20..23 = 0
      mov b[ds+20], -1                ; inmediato -1 guardado en 1 byte: 0xFF
; ---- b <- -1 escribe 0xFF y nada mas: l[ds+20] = 0xFF000000
      mov eax, l[ds+20]               ; lee el long
      mov ebx, 0                      ; esperado = 0xFF000000 (se arma con ldh/ldl)
      ldh ebx, 0xFF00                 ; parte alta = 0xFF00
      mov efx, n_t5                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
      mov eax, 0                      ; EAX = 0xAAFFBBCC (se arma con ldh/ldl)
      ldh eax, 0xAAFF                 ; parte alta = 0xAAFF
      ldl eax, 0xBBCC                 ; parte baja = 0xBBCC
      mov l[ds+24], eax               ; DS+24..27 = AA FF BB CC
      add b[ds+25], 1                 ; 0xFF + 1 dentro de UN byte: debe quedar 0x00 y NO acarrear al byte de arriba
; ---- add b desborda sin acarrear: l[ds+24] = 0xAA00BBCC
      mov eax, l[ds+24]               ; lee el long
      mov ebx, 0                      ; esperado = 0xAA00BBCC (se arma con ldh/ldl)
      ldh ebx, 0xAA00                 ; parte alta = 0xAA00
      ldl ebx, 0xBBCC                 ; parte baja = 0xBBCC
      mov efx, n_t6                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO

; ============ D) direccionamiento por REGISTRO puntero y offsets negativos ============
      mov edx, ds                     ; EDX = puntero logico al inicio de Data
      add edx, 8                      ; EDX apunta a DS+8
; ---- l[edx-4] (offset negativo) = l[ds+4] = 0x80FF7F01
      mov eax, l[edx-4]               ; lee DS+4
      mov ebx, 0                      ; esperado = 0x80FF7F01 (se arma con ldh/ldl)
      ldh ebx, 0x80FF                 ; parte alta = 0x80FF
      ldl ebx, 0x7F01                 ; parte baja = 0x7F01
      mov efx, n_d1                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
; ---- l[edx+4] = l[ds+12] = 0x33440000
      mov edx, ds                     ; OJO: el verificador anterior piso EDX, lo recargo
      add edx, 8                      ; EDX apunta a DS+8
      mov eax, l[edx+4]               ; lee DS+12
      mov ebx, 0                      ; esperado = 0x33440000 (se arma con ldh/ldl)
      ldh ebx, 0x3344                 ; parte alta = 0x3344
      mov efx, n_d2                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO

; ============ E) bordes EXACTOS del Data Segment (DATA = 32 bytes: validos ds+0 ... ds+31) ============
      mov eax, 0                      ; EAX = 0xCAFEBABE (se arma con ldh/ldl)
      ldh eax, 0xCAFE                 ; parte alta = 0xCAFE
      ldl eax, 0xBABE                 ; parte baja = 0xBABE
      mov l[ds+28], eax               ; ultimo long valido: bytes 28..31
; ---- l[ds+28] en el borde: se escribe y se lee
      mov eax, l[ds+28]               ; lee
      mov ebx, 0                      ; esperado = 0xCAFEBABE (se arma con ldh/ldl)
      ldh ebx, 0xCAFE                 ; parte alta = 0xCAFE
      ldl ebx, 0xBABE                 ; parte baja = 0xBABE
      mov efx, n_e1                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
      mov b[ds+31], 0x7E              ; ultimo byte valido
; ---- b[ds+31] = 0x7E (ultimo byte del segmento)
      mov eax, b[ds+31]               ; lee
      mov ebx, 126                    ; esperado = 0x7E
      mov efx, n_e2                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
      mov w[ds+30], 0x1234            ; ultima word valida: bytes 30..31 (pisa el 0x7E)
; ---- w[ds+30] = 0x1234 (ultima word)
      mov eax, w[ds+30]               ; lee
      mov ebx, 4660                   ; esperado = 0x1234
      mov efx, n_e3                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
; ---- el borde NO invadio el Extra Segment: b[es+0] sigue en 0
      mov eax, b[es+0]                ; primer byte de ES (contiguo fisicamente a DS)
      mov ebx, 0                      ; esperado = 0x0
      mov efx, n_e4                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO

; ============ F) cruzar datos entre DS y ES ============
      mov l[es+0], l[ds+0]            ; memoria -> memoria: copia 4 bytes de DS a ES
; ---- DS -> ES con l: l[es+0] = 0x11223344
      mov eax, l[es+0]                ; lee
      mov ebx, 0                      ; esperado = 0x11223344 (se arma con ldh/ldl)
      ldh ebx, 0x1122                 ; parte alta = 0x1122
      ldl ebx, 0x3344                 ; parte baja = 0x3344
      mov efx, n_x1                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
      mov b[es+4], l[ds+0]            ; long -> byte: se trunca (queda el byte bajo)
; ---- DS(l) -> ES(b): b[es+4] = 0x44
      mov eax, b[es+4]                ; lee
      mov ebx, 68                     ; esperado = 0x44
      mov efx, n_x2                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
      mov w[es+6], l[ds+0]            ; long -> word: queda la word baja
; ---- DS(l) -> ES(w): w[es+6] = 0x3344
      mov eax, w[es+6]                ; lee
      mov ebx, 13124                  ; esperado = 0x3344
      mov efx, n_x3                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
      mov w[es+8], w[ds+0]            ; word -> word
; ---- DS(w) -> ES(w): w[es+8] = 0x1122
      mov eax, w[es+8]                ; lee
      mov ebx, 4386                   ; esperado = 0x1122
      mov efx, n_x4                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
      mov l[es+16], 0x7788            ; ES+16 = 0x00007788
      swap l[ds+0], l[es+16]          ; intercambia dos memorias de segmentos DISTINTOS
; ---- SWAP DS<->ES: l[ds+0] = 0x00007788
      mov eax, l[ds+0]                ; lee
      mov ebx, 30600                  ; esperado = 0x7788
      mov efx, n_x5                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
; ---- SWAP DS<->ES: l[es+16] = 0x11223344
      mov eax, l[es+16]               ; lee
      mov ebx, 0                      ; esperado = 0x11223344 (se arma con ldh/ldl)
      ldh ebx, 0x1122                 ; parte alta = 0x1122
      ldl ebx, 0x3344                 ; parte baja = 0x3344
      mov efx, n_x6                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
; ---- DS no se altero por escribir en ES: l[ds+4] = 0x80FF7F01
      mov eax, l[ds+4]                ; lee
      mov ebx, 0                      ; esperado = 0x80FF7F01 (se arma con ldh/ldl)
      ldh ebx, 0x80FF                 ; parte alta = 0x80FF
      ldl ebx, 0x7F01                 ; parte baja = 0x7F01
      mov efx, n_x7                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
      ret                                   ; fin limpio de main (RET con -1 en la pila)

; ======================== SUBRUTINAS ========================
imprimir_ks: mov edx, ks                ; EDX = inicio del Const Segment
      add edx, eex                      ; + offset del texto
      sys 4                             ; escribe hasta el \0
      ret

; verificar: EAX = obtenido, EBX = esperado, EFX = nombre  (celdas de trabajo en ES+56 y ES+60)
verificar: mov l[es+56], eax
      mov l[es+60], ebx
      cmp eax, ebx
      jnz ver_f
      mov eex, s_ok
      call imprimir_ks
      jmp ver_n
ver_f: mov eex, s_fa
      call imprimir_ks
ver_n: mov eex, efx
      call imprimir_ks
      mov eex, s_nl
      call imprimir_ks
      mov eax, l[es+56]
      cmp eax, l[es+60]
      jz ver_r
      mov eex, s_hdr
      call imprimir_ks
      mov edx, es
      add edx, 56
      mov ecx, 2
      ldh ecx, 4
      mov eax, 9
      sys 2
ver_r: ret

