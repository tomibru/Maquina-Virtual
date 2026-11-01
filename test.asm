; =====================================================================
; test_alu_flags.asm  -  Aritmetica, logica, flags (CC) y registros parciales
; AUTOVERIFICABLE: cada caso imprime  'OK' + nombre  o  'FALLO' + nombre
;   Resultado esperado en una maquina correcta: TODAS las lineas dicen OK
; CC = bits N(31) Z(30) C(29) V(28). Se compara el nibble: N=8 Z=4 C=2 V=1
; =====================================================================
\\DATA 64                          ; 64 bytes de Data (lo uso para volcar valores si algo falla)
\\STACK 128                        ; pila de 128 bytes (CALL/RET la usan)

; ---- textos (van al Const Segment; el operando vale el OFFSET dentro de KS) ----
s_ok   EQU "OK     "
s_fa   EQU "FALLO  "
s_nl   EQU "\n"
s_hdr  EQU "   (obtenido, CC obtenido, esperado, CC esperado):\n"
n_a1    EQU "ADD 5+3 (sin flags)"
n_a2    EQU "ADD 0x7FFFFFFF+1 (V: pos+pos=neg)"
n_a3    EQU "ADD 0xFFFFFFFF+1 (C y Z: acarreo sin signo)"
n_a4    EQU "ADD 0x80000000+0x80000000 (C, V y Z)"
n_s1    EQU "SUB 5-3 (sin flags)"
n_s2    EQU "SUB 3-5 (C: pide prestamo, N)"
n_s3    EQU "SUB 0x80000000-1 (V: neg-pos=pos)"
n_s4    EQU "SUB 7-7 (Z, sin prestamo)"
n_m1    EQU "MUL 6*7 (sin flags)"
n_m2    EQU "MUL 0x10000*0x10000 (C, V y Z: 2^32)"
n_m3    EQU "MUL 0x40000000*2 (V sin C)"
n_d1q   EQU "DIV 17/5 -> cociente"
n_d1r   EQU "DIV 17/5 -> resto (AC)"
n_d2q   EQU "DIV -17/5 (con signo) -> cociente"
n_d2r   EQU "DIV -17/5 (con signo) -> resto (AC)"
n_l1    EQU "SHL 1<<4 (sin flags)"
n_l2    EQU "SHL 0x80000000<<1 (C: sale un 1; Z; V)"
n_l3    EQU "SHL 0x40000000<<1 (V sin C; N)"
n_l4    EQU "SHL 0x80000001<<32 (justo 32: sale el bit 0 -> C)"
n_l5    EQU "SHL 3<<33 (mas de 32: C=0)"
n_r1    EQU "SHR 0x11>>1 (C: sale el bit 0)"
n_r2    EQU "SHR 8>>40 (desplazar mas de 32: queda 0)"
n_t1    EQU "SAR -8>>1 (conserva el signo)"
n_t2    EQU "SAR -1>>1 (C: sale un 1; N)"
n_t3    EQU "SAR 0x80000000>>40 (>=32 con signo: todo 1s)"
n_t4    EQU "SAR 0x7FFFFFFF>>40 (>=32 positivo: 0)"
n_g1    EQU "AND 0x0F0F & 0x00FF"
n_g2    EQU "OR 0x0F00 | 0x00F0"
n_g3    EQU "XOR x ^ x (Z)"
n_g4    EQU "NOT 0 (N)"
n_p1    EQU "mov al: solo cambia el byte bajo"
n_p2    EQU "mov ah: solo cambia el 2do byte"
n_p3    EQU "mov ax: solo cambia los 2 bytes bajos"
n_p4    EQU "add al desborda SIN acarrear a AH"
n_p5    EQU "sub ah (0-1) SIN prestar de la parte alta"
n_p6    EQU "lectura de AL con signo (0xF0 -> 0xFFFFFFF0)"
n_p7    EQU "lectura de AH sin signo (0x56 -> 0x00000056)"
n_p8    EQU "lectura de AX con signo (0xF0F0 -> 0xFFFFF0F0)"

; ============================ PROGRAMA ============================
main: mov eex, 0                     ; (arranque)
; ======================== ADD ========================
; ---- ADD 5+3 (sin flags) ----
      mov eax, 5                    ; op1 = 5
      mov eex, 3                    ; op2 = 3
      add eax, eex                  ; EAX = 5 + 3
      mov ecx, cc                   ; copio CC YA (cualquier instruccion despues lo pisaria)
      shr ecx, 28                   ; me quedo solo con el nibble N Z C V (N=8 Z=4 C=2 V=1)
      mov ebx, 8                    ; resultado esperado = 0x8
      mov edx, 0                    ; CC esperado (nibble) = 0
      mov efx, n_a1                 ; nombre del caso
      call verificar                ; compara y escribe OK o FALLO

; ---- ADD 0x7FFFFFFF+1 (V: pos+pos=neg) ----
      mov eax, 0                    ; op1 = mayor positivo (se arma con ldh/ldl)
      ldh eax, 0x7FFF               ; parte ALTA de eax = 0x7FFF
      ldl eax, 0xFFFF               ; parte BAJA de eax = 0xFFFF
      mov eex, 1                    ; op2 = 1
      add eax, eex                  ; desborda en complemento a 2
      mov ecx, cc                   ; copio CC YA (cualquier instruccion despues lo pisaria)
      shr ecx, 28                   ; me quedo solo con el nibble N Z C V (N=8 Z=4 C=2 V=1)
      mov ebx, 0                    ; resultado esperado = 0x80000000 (se arma con ldh/ldl)
      ldh ebx, 0x8000               ; parte ALTA de ebx = 0x8000
      mov edx, 9                    ; CC esperado (nibble) = 9
      mov efx, n_a2                 ; nombre del caso
      call verificar                ; compara y escribe OK o FALLO

; ---- ADD 0xFFFFFFFF+1 (C y Z: acarreo sin signo) ----
      mov eax, -1                   ; op1 = 0xFFFFFFFF
      mov eex, 1                    ; op2 = 1
      add eax, eex                  ; da la vuelta completa: 0 con acarreo
      mov ecx, cc                   ; copio CC YA (cualquier instruccion despues lo pisaria)
      shr ecx, 28                   ; me quedo solo con el nibble N Z C V (N=8 Z=4 C=2 V=1)
      mov ebx, 0                    ; resultado esperado = 0x0
      mov edx, 6                    ; CC esperado (nibble) = 6
      mov efx, n_a3                 ; nombre del caso
      call verificar                ; compara y escribe OK o FALLO

; ---- ADD 0x80000000+0x80000000 (C, V y Z) ----
      mov eax, 0                    ; op1 = menor negativo (se arma con ldh/ldl)
      ldh eax, 0x8000               ; parte ALTA de eax = 0x8000
      mov eex, 0                    ; op2 = menor negativo (se arma con ldh/ldl)
      ldh eex, 0x8000               ; parte ALTA de eex = 0x8000
      add eax, eex                  ; neg+neg=0: acarreo y desborde
      mov ecx, cc                   ; copio CC YA (cualquier instruccion despues lo pisaria)
      shr ecx, 28                   ; me quedo solo con el nibble N Z C V (N=8 Z=4 C=2 V=1)
      mov ebx, 0                    ; resultado esperado = 0x0
      mov edx, 7                    ; CC esperado (nibble) = 7
      mov efx, n_a4                 ; nombre del caso
      call verificar                ; compara y escribe OK o FALLO

; ======================== SUB ========================
; ---- SUB 5-3 (sin flags) ----
      mov eax, 5                    ; op1
      mov eex, 3                    ; op2
      sub eax, eex                  ; EAX = 5 - 3
      mov ecx, cc                   ; copio CC YA (cualquier instruccion despues lo pisaria)
      shr ecx, 28                   ; me quedo solo con el nibble N Z C V (N=8 Z=4 C=2 V=1)
      mov ebx, 2                    ; resultado esperado = 0x2
      mov edx, 0                    ; CC esperado (nibble) = 0
      mov efx, n_s1                 ; nombre del caso
      call verificar                ; compara y escribe OK o FALLO

; ---- SUB 3-5 (C: pide prestamo, N) ----
      mov eax, 3                    ; op1
      mov eex, 5                    ; op2
      sub eax, eex                  ; 3 < 5: necesita prestamo -> C=1
      mov ecx, cc                   ; copio CC YA (cualquier instruccion despues lo pisaria)
      shr ecx, 28                   ; me quedo solo con el nibble N Z C V (N=8 Z=4 C=2 V=1)
      mov ebx, -2                   ; resultado esperado = 0xFFFFFFFE
      mov edx, 10                   ; CC esperado (nibble) = 10
      mov efx, n_s2                 ; nombre del caso
      call verificar                ; compara y escribe OK o FALLO

; ---- SUB 0x80000000-1 (V: neg-pos=pos) ----
      mov eax, 0                    ; op1 (se arma con ldh/ldl)
      ldh eax, 0x8000               ; parte ALTA de eax = 0x8000
      mov eex, 1                    ; op2
      sub eax, eex                  ; desborda hacia positivo
      mov ecx, cc                   ; copio CC YA (cualquier instruccion despues lo pisaria)
      shr ecx, 28                   ; me quedo solo con el nibble N Z C V (N=8 Z=4 C=2 V=1)
      mov ebx, 0                    ; resultado esperado = 0x7FFFFFFF (se arma con ldh/ldl)
      ldh ebx, 0x7FFF               ; parte ALTA de ebx = 0x7FFF
      ldl ebx, 0xFFFF               ; parte BAJA de ebx = 0xFFFF
      mov edx, 1                    ; CC esperado (nibble) = 1
      mov efx, n_s3                 ; nombre del caso
      call verificar                ; compara y escribe OK o FALLO

; ---- SUB 7-7 (Z, sin prestamo) ----
      mov eax, 7                    ; op1
      mov eex, 7                    ; op2
      sub eax, eex                  ; resultado cero
      mov ecx, cc                   ; copio CC YA (cualquier instruccion despues lo pisaria)
      shr ecx, 28                   ; me quedo solo con el nibble N Z C V (N=8 Z=4 C=2 V=1)
      mov ebx, 0                    ; resultado esperado = 0x0
      mov edx, 4                    ; CC esperado (nibble) = 4
      mov efx, n_s4                 ; nombre del caso
      call verificar                ; compara y escribe OK o FALLO

; ======================== MUL ========================
; ---- MUL 6*7 (sin flags) ----
      mov eax, 6                    ; op1
      mov eex, 7                    ; op2
      mul eax, eex                  ; EAX = 42
      mov ecx, cc                   ; copio CC YA (cualquier instruccion despues lo pisaria)
      shr ecx, 28                   ; me quedo solo con el nibble N Z C V (N=8 Z=4 C=2 V=1)
      mov ebx, 42                   ; resultado esperado = 0x2A
      mov edx, 0                    ; CC esperado (nibble) = 0
      mov efx, n_m1                 ; nombre del caso
      call verificar                ; compara y escribe OK o FALLO

; ---- MUL 0x10000*0x10000 (C, V y Z: 2^32) ----
      mov eax, 0                    ; op1 = 65536 (se arma con ldh/ldl)
      ldh eax, 0x0001               ; parte ALTA de eax = 0x0001
      mov eex, 0                    ; op2 = 65536 (se arma con ldh/ldl)
      ldh eex, 0x0001               ; parte ALTA de eex = 0x0001
      mul eax, eex                  ; 2^32 no entra en 32 bits: queda 0
      mov ecx, cc                   ; copio CC YA (cualquier instruccion despues lo pisaria)
      shr ecx, 28                   ; me quedo solo con el nibble N Z C V (N=8 Z=4 C=2 V=1)
      mov ebx, 0                    ; resultado esperado = 0x0
      mov edx, 7                    ; CC esperado (nibble) = 7
      mov efx, n_m2                 ; nombre del caso
      call verificar                ; compara y escribe OK o FALLO

; ---- MUL 0x40000000*2 (V sin C) ----
      mov eax, 0                    ; op1 = 2^30 (se arma con ldh/ldl)
      ldh eax, 0x4000               ; parte ALTA de eax = 0x4000
      mov eex, 2                    ; op2 = 2
      mul eax, eex                  ; 2^31: entra sin signo pero NO con signo
      mov ecx, cc                   ; copio CC YA (cualquier instruccion despues lo pisaria)
      shr ecx, 28                   ; me quedo solo con el nibble N Z C V (N=8 Z=4 C=2 V=1)
      mov ebx, 0                    ; resultado esperado = 0x80000000 (se arma con ldh/ldl)
      ldh ebx, 0x8000               ; parte ALTA de ebx = 0x8000
      mov edx, 9                    ; CC esperado (nibble) = 9
      mov efx, n_m3                 ; nombre del caso
      call verificar                ; compara y escribe OK o FALLO

; ======================== DIV ========================
; ---- DIV 17/5 ----
      mov eax, 17                   ; dividendo
      mov eex, 5                    ; divisor
      div eax, eex                  ; EAX = cociente, AC = resto
      mov efx, ac                   ; copio el resto (AC) antes de que lo pise otra instruccion
      mov ecx, cc                   ; copio CC
      shr ecx, 28                   ; nibble NZCV
      mov ebx, 3                    ; cociente esperado = 3
      mov edx, 0                    ; CC esperado
      mov l[ds+16], efx             ; guardo el resto en memoria (efx lo necesito para el nombre)
      mov efx, n_d1q                ; nombre del caso (cociente)
      call verificar                ; verifica cociente + CC
      mov eax, l[ds+16]             ; recupero el resto obtenido
      mov ebx, 2                    ; resto esperado = 2
      mov efx, n_d1r                ; nombre del caso (resto)
      call verificar_r              ; verifica solo el valor (sin CC)

; ---- DIV -17/5 (con signo) ----
      mov eax, -17                  ; dividendo
      mov eex, 5                    ; divisor
      div eax, eex                  ; EAX = cociente, AC = resto
      mov efx, ac                   ; copio el resto (AC) antes de que lo pise otra instruccion
      mov ecx, cc                   ; copio CC
      shr ecx, 28                   ; nibble NZCV
      mov ebx, -3                   ; cociente esperado = -3
      mov edx, 8                    ; CC esperado
      mov l[ds+16], efx             ; guardo el resto en memoria (efx lo necesito para el nombre)
      mov efx, n_d2q                ; nombre del caso (cociente)
      call verificar                ; verifica cociente + CC
      mov eax, l[ds+16]             ; recupero el resto obtenido
      mov ebx, -2                   ; resto esperado = -2
      mov efx, n_d2r                ; nombre del caso (resto)
      call verificar_r              ; verifica solo el valor (sin CC)

; ======================== SHL / SHR / SAR ========================
; ---- SHL 1<<4 (sin flags) ----
      mov eax, 1                    ; op1
      shl eax, 4                    ; EAX = 16
      mov ecx, cc                   ; copio CC YA (cualquier instruccion despues lo pisaria)
      shr ecx, 28                   ; me quedo solo con el nibble N Z C V (N=8 Z=4 C=2 V=1)
      mov ebx, 16                   ; resultado esperado = 0x10
      mov edx, 0                    ; CC esperado (nibble) = 0
      mov efx, n_l1                 ; nombre del caso
      call verificar                ; compara y escribe OK o FALLO

; ---- SHL 0x80000000<<1 (C: sale un 1; Z; V) ----
      mov eax, 0                    ; op1 (se arma con ldh/ldl)
      ldh eax, 0x8000               ; parte ALTA de eax = 0x8000
      shl eax, 1                    ; el bit 31 se expulsa
      mov ecx, cc                   ; copio CC YA (cualquier instruccion despues lo pisaria)
      shr ecx, 28                   ; me quedo solo con el nibble N Z C V (N=8 Z=4 C=2 V=1)
      mov ebx, 0                    ; resultado esperado = 0x0
      mov edx, 7                    ; CC esperado (nibble) = 7
      mov efx, n_l2                 ; nombre del caso
      call verificar                ; compara y escribe OK o FALLO

; ---- SHL 0x40000000<<1 (V sin C; N) ----
      mov eax, 0                    ; op1 (se arma con ldh/ldl)
      ldh eax, 0x4000               ; parte ALTA de eax = 0x4000
      shl eax, 1                    ; el bit 30 pasa al signo
      mov ecx, cc                   ; copio CC YA (cualquier instruccion despues lo pisaria)
      shr ecx, 28                   ; me quedo solo con el nibble N Z C V (N=8 Z=4 C=2 V=1)
      mov ebx, 0                    ; resultado esperado = 0x80000000 (se arma con ldh/ldl)
      ldh ebx, 0x8000               ; parte ALTA de ebx = 0x8000
      mov edx, 9                    ; CC esperado (nibble) = 9
      mov efx, n_l3                 ; nombre del caso
      call verificar                ; compara y escribe OK o FALLO

; ---- SHL 0x80000001<<32 (justo 32: sale el bit 0 -> C) ----
      mov eax, 0                    ; op1 (se arma con ldh/ldl)
      ldh eax, 0x8000               ; parte ALTA de eax = 0x8000
      ldl eax, 0x0001               ; parte BAJA de eax = 0x0001
      mov eex, 32                   ; cantidad = 32
      shl eax, eex                  ; con 32 el ultimo bit expulsado es el bit 0 original
      mov ecx, cc                   ; copio CC YA (cualquier instruccion despues lo pisaria)
      shr ecx, 28                   ; me quedo solo con el nibble N Z C V (N=8 Z=4 C=2 V=1)
      mov ebx, 0                    ; resultado esperado = 0x0
      mov edx, 7                    ; CC esperado (nibble) = 7
      mov efx, n_l4                 ; nombre del caso
      call verificar                ; compara y escribe OK o FALLO

; ---- SHL 3<<33 (mas de 32: C=0) ----
      mov eax, 3                    ; op1
      mov eex, 33                   ; cantidad = 33
      shl eax, eex                  ; todo se perdio: C=0, Z=1, V=1
      mov ecx, cc                   ; copio CC YA (cualquier instruccion despues lo pisaria)
      shr ecx, 28                   ; me quedo solo con el nibble N Z C V (N=8 Z=4 C=2 V=1)
      mov ebx, 0                    ; resultado esperado = 0x0
      mov edx, 5                    ; CC esperado (nibble) = 5
      mov efx, n_l5                 ; nombre del caso
      call verificar                ; compara y escribe OK o FALLO

; ---- SHR 0x11>>1 (C: sale el bit 0) ----
      mov eax, 17                   ; op1 = 10001b
      shr eax, 1                    ; sale un 1 por la derecha
      mov ecx, cc                   ; copio CC YA (cualquier instruccion despues lo pisaria)
      shr ecx, 28                   ; me quedo solo con el nibble N Z C V (N=8 Z=4 C=2 V=1)
      mov ebx, 8                    ; resultado esperado = 0x8
      mov edx, 2                    ; CC esperado (nibble) = 2
      mov efx, n_r1                 ; nombre del caso
      call verificar                ; compara y escribe OK o FALLO

; ---- SHR 8>>40 (desplazar mas de 32: queda 0) ----
      mov eax, 8                    ; op1
      mov eex, 40                   ; cantidad = 40 (>32)
      shr eax, eex                  ; todo se pierde; C y V deben quedar en 0
      mov ecx, cc                   ; copio CC YA (cualquier instruccion despues lo pisaria)
      shr ecx, 28                   ; me quedo solo con el nibble N Z C V (N=8 Z=4 C=2 V=1)
      mov ebx, 0                    ; resultado esperado = 0x0
      mov edx, 4                    ; CC esperado (nibble) = 4
      mov efx, n_r2                 ; nombre del caso
      call verificar                ; compara y escribe OK o FALLO

; ---- SAR -8>>1 (conserva el signo) ----
      mov eax, -8                   ; op1 = 0xFFFFFFF8
      sar eax, 1                    ; relleno con unos
      mov ecx, cc                   ; copio CC YA (cualquier instruccion despues lo pisaria)
      shr ecx, 28                   ; me quedo solo con el nibble N Z C V (N=8 Z=4 C=2 V=1)
      mov ebx, -4                   ; resultado esperado = 0xFFFFFFFC
      mov edx, 8                    ; CC esperado (nibble) = 8
      mov efx, n_t1                 ; nombre del caso
      call verificar                ; compara y escribe OK o FALLO

; ---- SAR -1>>1 (C: sale un 1; N) ----
      mov eax, -1                   ; op1 = 0xFFFFFFFF
      sar eax, 1                    ; sigue siendo -1; sale un 1
      mov ecx, cc                   ; copio CC YA (cualquier instruccion despues lo pisaria)
      shr ecx, 28                   ; me quedo solo con el nibble N Z C V (N=8 Z=4 C=2 V=1)
      mov ebx, -1                   ; resultado esperado = 0xFFFFFFFF
      mov edx, 10                   ; CC esperado (nibble) = 10
      mov efx, n_t2                 ; nombre del caso
      call verificar                ; compara y escribe OK o FALLO

; ---- SAR 0x80000000>>40 (>=32 con signo: todo 1s) ----
      mov eax, 0                    ; op1 negativo (se arma con ldh/ldl)
      ldh eax, 0x8000               ; parte ALTA de eax = 0x8000
      mov eex, 40                   ; cantidad = 40
      sar eax, eex                  ; queda 0xFFFFFFFF y C=1
      mov ecx, cc                   ; copio CC YA (cualquier instruccion despues lo pisaria)
      shr ecx, 28                   ; me quedo solo con el nibble N Z C V (N=8 Z=4 C=2 V=1)
      mov ebx, -1                   ; resultado esperado = 0xFFFFFFFF
      mov edx, 10                   ; CC esperado (nibble) = 10
      mov efx, n_t3                 ; nombre del caso
      call verificar                ; compara y escribe OK o FALLO

; ---- SAR 0x7FFFFFFF>>40 (>=32 positivo: 0) ----
      mov eax, 0                    ; op1 positivo (se arma con ldh/ldl)
      ldh eax, 0x7FFF               ; parte ALTA de eax = 0x7FFF
      ldl eax, 0xFFFF               ; parte BAJA de eax = 0xFFFF
      mov eex, 40                   ; cantidad = 40
      sar eax, eex                  ; queda 0 y C=0
      mov ecx, cc                   ; copio CC YA (cualquier instruccion despues lo pisaria)
      shr ecx, 28                   ; me quedo solo con el nibble N Z C V (N=8 Z=4 C=2 V=1)
      mov ebx, 0                    ; resultado esperado = 0x0
      mov edx, 4                    ; CC esperado (nibble) = 4
      mov efx, n_t4                 ; nombre del caso
      call verificar                ; compara y escribe OK o FALLO

; ======================== AND / OR / XOR / NOT ========================
; ---- AND 0x0F0F & 0x00FF ----
      mov eax, 3855                 ; op1
      mov eex, 255                  ; op2
      and eax, eex                  ; EAX = 0x000F
      mov ecx, cc                   ; copio CC YA (cualquier instruccion despues lo pisaria)
      shr ecx, 28                   ; me quedo solo con el nibble N Z C V (N=8 Z=4 C=2 V=1)
      mov ebx, 15                   ; resultado esperado = 0xF
      mov edx, 0                    ; CC esperado (nibble) = 0
      mov efx, n_g1                 ; nombre del caso
      call verificar                ; compara y escribe OK o FALLO

; ---- OR 0x0F00 | 0x00F0 ----
      mov eax, 3840                 ; op1
      mov eex, 240                  ; op2
      or eax, eex                   ; EAX = 0x0FF0
      mov ecx, cc                   ; copio CC YA (cualquier instruccion despues lo pisaria)
      shr ecx, 28                   ; me quedo solo con el nibble N Z C V (N=8 Z=4 C=2 V=1)
      mov ebx, 4080                 ; resultado esperado = 0xFF0
      mov edx, 0                    ; CC esperado (nibble) = 0
      mov efx, n_g2                 ; nombre del caso
      call verificar                ; compara y escribe OK o FALLO

; ---- XOR x ^ x (Z) ----
      mov eax, 4660                 ; op1
      mov eex, 4660                 ; op2 = igual
      xor eax, eex                  ; un valor XOR si mismo = 0
      mov ecx, cc                   ; copio CC YA (cualquier instruccion despues lo pisaria)
      shr ecx, 28                   ; me quedo solo con el nibble N Z C V (N=8 Z=4 C=2 V=1)
      mov ebx, 0                    ; resultado esperado = 0x0
      mov edx, 4                    ; CC esperado (nibble) = 4
      mov efx, n_g3                 ; nombre del caso
      call verificar                ; compara y escribe OK o FALLO

; ---- NOT 0 (N) ----
      mov eax, 0                    ; op1 = 0
      not eax                       ; invierte todos los bits
      mov ecx, cc                   ; copio CC YA (cualquier instruccion despues lo pisaria)
      shr ecx, 28                   ; me quedo solo con el nibble N Z C V (N=8 Z=4 C=2 V=1)
      mov ebx, -1                   ; resultado esperado = 0xFFFFFFFF
      mov edx, 8                    ; CC esperado (nibble) = 8
      mov efx, n_g4                 ; nombre del caso
      call verificar                ; compara y escribe OK o FALLO

; ======================== OPERANDOS PARCIALES ========================
; ---- mov al: solo cambia el byte bajo ----
      mov eax, 0                    ; EAX inicial (se arma con ldh/ldl)
      ldh eax, 0x1234               ; parte ALTA de eax = 0x1234
      ldl eax, 0x5678               ; parte BAJA de eax = 0x5678
      mov al, 0xFF                  ; AL = 0xFF
      mov ebx, 0                    ; EAX esperado = 0x123456FF (se arma con ldh/ldl)
      ldh ebx, 0x1234               ; parte ALTA de ebx = 0x1234
      ldl ebx, 0x56FF               ; parte BAJA de ebx = 0x56FF
      mov efx, n_p1                 ; nombre del caso
      call verificar_r              ; verifica solo el valor (sin CC)

; ---- mov ah: solo cambia el 2do byte ----
      mov eax, 0                    ; EAX inicial (se arma con ldh/ldl)
      ldh eax, 0x1234               ; parte ALTA de eax = 0x1234
      ldl eax, 0x56FF               ; parte BAJA de eax = 0x56FF
      mov ah, 0xAA                  ; AH = 0xAA
      mov ebx, 0                    ; EAX esperado = 0x1234AAFF (se arma con ldh/ldl)
      ldh ebx, 0x1234               ; parte ALTA de ebx = 0x1234
      ldl ebx, 0xAAFF               ; parte BAJA de ebx = 0xAAFF
      mov efx, n_p2                 ; nombre del caso
      call verificar_r              ; verifica solo el valor (sin CC)

; ---- mov ax: solo cambia los 2 bytes bajos ----
      mov eax, 0                    ; EAX inicial (se arma con ldh/ldl)
      ldh eax, 0x1234               ; parte ALTA de eax = 0x1234
      ldl eax, 0xAAFF               ; parte BAJA de eax = 0xAAFF
      mov ax, 0x1111                ; AX = 0x1111
      mov ebx, 0                    ; EAX esperado = 0x12341111 (se arma con ldh/ldl)
      ldh ebx, 0x1234               ; parte ALTA de ebx = 0x1234
      ldl ebx, 0x1111               ; parte BAJA de ebx = 0x1111
      mov efx, n_p3                 ; nombre del caso
      call verificar_r              ; verifica solo el valor (sin CC)

; ---- add al desborda SIN acarrear a AH ----
      mov eax, 0                    ; EAX inicial (se arma con ldh/ldl)
      ldh eax, 0x1234               ; parte ALTA de eax = 0x1234
      ldl eax, 0x56FF               ; parte BAJA de eax = 0x56FF
      add al, 1                     ; 0xFF+1 = 0x00 dentro de AL
      mov ebx, 0                    ; EAX esperado = 0x12345600 (se arma con ldh/ldl)
      ldh ebx, 0x1234               ; parte ALTA de ebx = 0x1234
      ldl ebx, 0x5600               ; parte BAJA de ebx = 0x5600
      mov efx, n_p4                 ; nombre del caso
      call verificar_r              ; verifica solo el valor (sin CC)

; ---- sub ah (0-1) SIN prestar de la parte alta ----
      mov eax, 0                    ; EAX inicial (se arma con ldh/ldl)
      ldh eax, 0x1234               ; parte ALTA de eax = 0x1234
      ldl eax, 0x0056               ; parte BAJA de eax = 0x0056
      sub ah, 1                     ; AH = 0x00-1 = 0xFF
      mov ebx, 0                    ; EAX esperado = 0x1234FF56 (se arma con ldh/ldl)
      ldh ebx, 0x1234               ; parte ALTA de ebx = 0x1234
      ldl ebx, 0xFF56               ; parte BAJA de ebx = 0xFF56
      mov efx, n_p5                 ; nombre del caso
      call verificar_r              ; verifica solo el valor (sin CC)

; ---- lectura de AL con signo (0xF0 -> 0xFFFFFFF0) ----
      mov eax, 0                    ; EAX con AL = 0xF0 (se arma con ldh/ldl)
      ldh eax, 0x1234               ; parte ALTA de eax = 0x1234
      ldl eax, 0x56F0               ; parte BAJA de eax = 0x56F0
      mov eax, al                   ; lee solo AL y extiende el signo
      mov ebx, -16                  ; EAX esperado = 0xFFFFFFF0
      mov efx, n_p6                 ; nombre del caso
      call verificar_r              ; verifica solo el valor (sin CC)

; ---- lectura de AH sin signo (0x56 -> 0x00000056) ----
      mov eax, 0                    ; EAX con AH = 0x56 (se arma con ldh/ldl)
      ldh eax, 0x1234               ; parte ALTA de eax = 0x1234
      ldl eax, 0x56F0               ; parte BAJA de eax = 0x56F0
      mov eax, ah                   ; lee solo AH
      mov ebx, 86                   ; EAX esperado = 0x56
      mov efx, n_p7                 ; nombre del caso
      call verificar_r              ; verifica solo el valor (sin CC)

; ---- lectura de AX con signo (0xF0F0 -> 0xFFFFF0F0) ----
      mov eax, 0                    ; EAX con AX = 0xF0F0 (se arma con ldh/ldl)
      ldh eax, 0x1234               ; parte ALTA de eax = 0x1234
      ldl eax, 0xF0F0               ; parte BAJA de eax = 0xF0F0
      mov eax, ax                   ; lee AX y extiende el signo
      mov ebx, -3856                ; EAX esperado = 0xFFFFF0F0
      mov efx, n_p8                 ; nombre del caso
      call verificar_r              ; verifica solo el valor (sin CC)

      stop                            ; fin normal

; ======================== SUBRUTINAS ========================
; imprimir_ks: imprime el texto cuyo OFFSET en KS esta en EEX
imprimir_ks: mov edx, ks                     ; EDX = puntero al inicio del Const Segment
      add edx, eex                           ; + offset del texto
      sys 4                                  ; escribe hasta el \0
      ret

; verificar: EAX=resultado obtenido  EBX=esperado  ECX=CC obtenido  EDX=CC esperado  EFX=nombre
verificar: mov l[ds+0], eax                  ; guardo los 4 valores para poder mostrarlos si falla
      mov l[ds+4], ecx
      mov l[ds+8], ebx
      mov l[ds+12], edx
      cmp eax, ebx                           ; ¿resultado igual al esperado?
      jnz falla                              ; no -> FALLO
      cmp ecx, edx                           ; ¿CC igual al esperado?
      jnz falla                              ; no -> FALLO
      mov eex, s_ok                          ; todo bien
      call imprimir_ks                       ; escribe 'OK     '
      jmp nombre
falla: mov eex, s_fa
      call imprimir_ks                       ; escribe 'FALLO  '
nombre: mov eex, efx
      call imprimir_ks                       ; escribe el nombre del caso
      mov eex, s_nl
      call imprimir_ks                       ; salto de linea
      mov eax, l[ds+0]                       ; releo lo obtenido
      cmp eax, l[ds+8]
      jnz mostrar                            ; si el resultado fallo, muestro valores
      mov eax, l[ds+4]
      cmp eax, l[ds+12]
      jnz mostrar                            ; si el CC fallo, muestro valores
      ret
mostrar: mov eex, s_hdr
      call imprimir_ks                       ; escribe el encabezado
      mov edx, ds                            ; EDX = inicio de Data
      mov ecx, 4
      ldh ecx, 4                             ; ECX = 4 celdas de 4 bytes
      mov eax, 9                             ; formato: hexadecimal + decimal
      sys 2                                  ; muestra los 4 valores guardados
      ret

; verificar_r: igual que verificar pero solo compara EAX con EBX (sin CC)
verificar_r: mov l[ds+0], eax
      mov l[ds+8], ebx
      cmp eax, ebx
      jnz falla_r
      mov eex, s_ok
      call imprimir_ks
      jmp nombre_r
falla_r: mov eex, s_fa
      call imprimir_ks
nombre_r: mov eex, efx
      call imprimir_ks
      mov eex, s_nl
      call imprimir_ks
      mov eax, l[ds+0]
      cmp eax, l[ds+8]
      jnz mostrar_r
      ret
mostrar_r: mov edx, ds
      mov ecx, 3
      ldh ecx, 4
      mov eax, 9
      sys 2                                  ; muestra obtenido / (celda 1 sin usar) / esperado
      ret