; =====================================================================
; test_stack_rutinas.asm  -  PUSH/POP, CALL/RET, parametros con BP y recursividad
; AUTOVERIFICABLE: imprime 'OK' o 'FALLO' por cada caso.
;   Resultado esperado: TODO 'OK', luego la linea 'argc' y fin SIN mensajes extra
;   (main termina con RET, NO con STOP: no debe salir 'Deteniendo programa...')
; =====================================================================
\\DATA 128                         ; Data: celdas de trabajo (ds+60..ds+84) y del verificador (ds+112)
\\STACK 2048                       ; pila grande: la recursion profunda necesita ~1200 bytes
\\EXTRA 16                         ; Extra: lo uso para el ejemplo POP w[ES+4]

; ---- textos (Const Segment) ----
s_ok   EQU "OK     "
s_fa   EQU "FALLO  "
s_nl   EQU "\n"
s_hdr  EQU "   (obtenido, esperado):\n"
n_a0    EQU "pila inicial: [SP+0] = -1 (retorno de main)"
n_a1    EQU "PUSH resta exactamente 4 a SP"
n_a2    EQU "Big Endian: [SP+0] = 0x11 (byte MAS significativo)"
n_a3    EQU "Big Endian: [SP+1] = 0x22"
n_a4    EQU "Big Endian: [SP+3] = 0x44 (byte MENOS significativo)"
n_a5    EQU "los 4 bytes juntos = 0x11223344"
n_a6    EQU "POP suma exactamente 4 a SP"
n_a7    EQU "POP trajo 0x11223344"
n_a8    EQU "POP NO borra: el valor sigue en [SP-4]"
n_a9    EQU "PUSH AX apila 0xFFFFABCD (4 bytes con signo)"
n_a10   EQU "POP w[ES+4]: ES[4] = 0xAB"
n_a11   EQU "POP w[ES+4]: ES[5] = 0xCD"
n_a12   EQU "POP w[ES+4]: ES[6] sigue en 0 (no escribio de mas)"
n_a13   EQU "PUSH w[ds+0]=0x80FF apila 0xFFFF80FF"
n_a14   EQU "PUSH -2 apila 0xFFFFFFFE"
n_a15   EQU "PUSH AL=0xF0 apila 0xFFFFFFF0"
n_a16   EQU "LIFO: 1er POP = 3"
n_a17   EQU "LIFO: 2do POP = 2"
n_a18   EQU "LIFO: 3er POP = 1"
n_a19   EQU "la pila quedo balanceada: SP = SP inicial"
n_b1    EQU "suma(10,20) = 30 (parametros leidos con BP)"
n_b2    EQU "resta(100,58) = 42 (orden de parametros correcto)"
n_b3    EQU "variables locales con BP negativo: 7+5 = 12"
n_b4    EQU "llamada anidada: triple(7) = 21"
n_b5    EQU "BP del main intacto tras las llamadas (0x7777)"
n_b6    EQU "SP balanceado tras todas las llamadas"
n_c1    EQU "fact(5) = 120"
n_c2    EQU "fact(1) = 1 (caso base)"
n_c3    EQU "fact(10) = 3628800"
n_c4    EQU "fib(10) = 55"
n_c5    EQU "recursion profunda: sumn(100) = 5050"
n_c6    EQU "SP balanceado tras la recursion"

; ============================ PROGRAMA ============================
main:

; ============ A) PUSH / POP: siempre 4 bytes, Big Endian ============
      mov l[ds+64], l[sp+4]           ; guardo argc (lo dejo la VM en la pila de main)
      mov l[ds+68], l[sp+8]           ; guardo argv (puntero al arreglo de parametros)
      mov l[ds+60], sp                ; guardo el SP inicial: al final debe quedar IGUAL (pila balanceada)
; ---- pila inicial: [SP+0] = -1 (retorno de main)
      mov eax, l[sp+0]                ; lee 4 bytes del tope
      mov ebx, -1                     ; esperado = 0xFFFFFFFF
      mov efx, n_a0                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
; ---- PUSH eax = 0x11223344
      mov eax, 0                      ; valor a apilar (se arma con ldh/ldl)
      ldh eax, 0x1122                 ; parte alta = 0x1122
      ldl eax, 0x3344                 ; parte baja = 0x3344
      mov ecx, sp                     ; SP antes del PUSH
      push eax                        ; apila 4 bytes
      mov edx, sp                     ; SP despues del PUSH
      sub ecx, edx                    ; ecx = SP_antes - SP_despues
; ---- PUSH resta exactamente 4 a SP
      mov eax, ecx                    ; la diferencia de SP
      mov ebx, 4                      ; esperado = 0x4
      mov efx, n_a1                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
; ---- Big Endian: [SP+0] = 0x11 (byte MAS significativo)
      mov eax, b[sp+0]                ; 1er byte: direccion mas baja
      mov ebx, 17                     ; esperado = 0x11
      mov efx, n_a2                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
; ---- Big Endian: [SP+1] = 0x22
      mov eax, b[sp+1]                ; 2do byte
      mov ebx, 34                     ; esperado = 0x22
      mov efx, n_a3                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
; ---- Big Endian: [SP+3] = 0x44 (byte MENOS significativo)
      mov eax, b[sp+3]                ; 4to byte
      mov ebx, 68                     ; esperado = 0x44
      mov efx, n_a4                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
; ---- los 4 bytes juntos = 0x11223344
      mov eax, l[sp+0]                ; long completo
      mov ebx, 0                      ; esperado = 0x11223344 (se arma con ldh/ldl)
      ldh ebx, 0x1122                 ; parte alta = 0x1122
      ldl ebx, 0x3344                 ; parte baja = 0x3344
      mov efx, n_a5                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
; ---- POP ecx (no debe borrar la memoria)
      mov edx, sp                     ; SP antes del POP
      pop ecx                         ; saca 4 bytes hacia ECX
      mov eex, sp                     ; SP despues del POP
      sub eex, edx                    ; eex = SP_despues - SP_antes
      mov l[ds+72], l[sp-4]           ; capturo YA el valor que quedo 'debajo' del tope (un CALL lo pisaria)
      mov l[ds+76], ecx               ; guardo lo que trajo el POP
; ---- POP suma exactamente 4 a SP
      mov eax, eex                    ; la diferencia de SP
      mov ebx, 4                      ; esperado = 0x4
      mov efx, n_a6                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
; ---- POP trajo 0x11223344
      mov eax, l[ds+76]               ; valor popeado
      mov ebx, 0                      ; esperado = 0x11223344 (se arma con ldh/ldl)
      ldh ebx, 0x1122                 ; parte alta = 0x1122
      ldl ebx, 0x3344                 ; parte baja = 0x3344
      mov efx, n_a7                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
; ---- POP NO borra: el valor sigue en [SP-4]
      mov eax, l[ds+72]               ; valor que quedo bajo el tope
      mov ebx, 0                      ; esperado = 0x11223344 (se arma con ldh/ldl)
      ldh ebx, 0x1122                 ; parte alta = 0x1122
      ldl ebx, 0x3344                 ; parte baja = 0x3344
      mov efx, n_a8                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
; ---- PUSH AX (ejemplo del enunciado): EAX = 0x1234ABCD
      mov eax, 0                      ; EAX (se arma con ldh/ldl)
      ldh eax, 0x1234                 ; parte alta = 0x1234
      ldl eax, 0xABCD                 ; parte baja = 0xABCD
      push ax                         ; AX = 0xABCD se extiende con signo: apila 0xFFFFABCD
; ---- PUSH AX apila 0xFFFFABCD (4 bytes con signo)
      mov eax, l[sp+0]                ; lee lo apilado
      mov ebx, -21555                 ; esperado = 0xFFFFABCD
      mov efx, n_a9                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
      pop w[es+4]                     ; saca 0xFFFFABCD y guarda SOLO 2 bytes en ES+4 (trunca)
; ---- POP w[ES+4]: ES[4] = 0xAB
      mov eax, b[es+4]                ; byte (con signo)
      mov ebx, -85                    ; esperado = 0xFFFFFFAB
      mov efx, n_a10                  ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
; ---- POP w[ES+4]: ES[5] = 0xCD
      mov eax, b[es+5]                ; byte (con signo)
      mov ebx, -51                    ; esperado = 0xFFFFFFCD
      mov efx, n_a11                  ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
; ---- POP w[ES+4]: ES[6] sigue en 0 (no escribio de mas)
      mov eax, b[es+6]                ; debe seguir en 0
      mov ebx, 0                      ; esperado = 0x0
      mov efx, n_a12                  ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
; ---- PUSH de operandos chicos: igual apilan 4 bytes con signo
      mov eax, 0xF0                   ; AL = 0xF0
      push al                         ; apila AL (1 byte) -> 4 bytes
      push -2                         ; apila el inmediato -2
      mov w[ds+0], 0x7FFF             ; w[DS+0] = 0x7FFF
      mov b[ds+0], 0x80               ; ahora w[DS+0] = 0x80FF
      push w[ds+0]                    ; apila la word de memoria 0x80FF
; ---- PUSH w[ds+0]=0x80FF apila 0xFFFF80FF
      mov eax, l[sp+0]                ; tope
      mov ebx, -32513                 ; esperado = 0xFFFF80FF
      mov efx, n_a13                  ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
; ---- PUSH -2 apila 0xFFFFFFFE
      mov eax, l[sp+4]                ; segundo desde el tope
      mov ebx, -2                     ; esperado = 0xFFFFFFFE
      mov efx, n_a14                  ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
; ---- PUSH AL=0xF0 apila 0xFFFFFFF0
      mov eax, l[sp+8]                ; tercero desde el tope
      mov ebx, -16                    ; esperado = 0xFFFFFFF0
      mov efx, n_a15                  ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
      add sp, 12                      ; tiro esos 3 valores
; ---- orden LIFO
      push 1                          ; apila 1
      push 2                          ; apila 2
      push 3                          ; apila 3
      pop eax                         ; debe salir 3
      pop ecx                         ; debe salir 2
      pop edx                         ; debe salir 1
      mov l[ds+80], ecx               ; guardo el 2 (la llamada a verificar pisa ECX)
      mov l[ds+84], edx               ; guardo el 1
; ---- LIFO: 1er POP = 3
      mov eax, eax                    ; (ya esta en EAX)
      mov ebx, 3                      ; esperado = 0x3
      mov efx, n_a16                  ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
; ---- LIFO: 2do POP = 2
      mov eax, l[ds+80]               
      mov ebx, 2                      ; esperado = 0x2
      mov efx, n_a17                  ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
; ---- LIFO: 3er POP = 1
      mov eax, l[ds+84]               
      mov ebx, 1                      ; esperado = 0x1
      mov efx, n_a18                  ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
; ---- la pila quedo balanceada: SP = SP inicial
      mov eax, sp                     ; SP actual
      mov ebx, l[ds+60]               ; esperado = el SP INICIAL guardado al principio
      mov efx, n_a19                  ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO

; ============ B) CALL a subrutinas con parametros por pila, leidos con BP ============
; convencion: el que llama apila los parametros de DERECHA a IZQUIERDA, hace CALL y limpia la pila
; dentro de la subrutina: [BP+0]=BP viejo  [BP+4]=direccion de retorno  [BP+8]=1er parametro  [BP+12]=2do ...
      mov bp, 0x7777                  ; BP del 'main': la subrutina NO debe cambiarlo (lo guarda con push/pop)
      push 20                         ; 2do parametro
      push 10                         ; 1er parametro
      call suma                       ; suma(10, 20)
      add sp, 8                       ; el que llama limpia los 2 parametros
; ---- suma(10,20) = 30 (parametros leidos con BP)
      mov eax, eax                    ; resultado ya esta en EAX
      mov ebx, 30                     ; esperado = 0x1E
      mov efx, n_b1                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
      push 58                         ; 2do parametro (b)
      push 100                        ; 1er parametro (a)
      call resta                      ; resta(a=100, b=58)
      add sp, 8                       ; limpio parametros
; ---- resta(100,58) = 42 (orden de parametros correcto)
      mov eax, eax                    
      mov ebx, 42                     ; esperado = 0x2A
      mov efx, n_b2                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
      call locales                    ; subrutina con variables LOCALES en [BP-4] y [BP-8]
; ---- variables locales con BP negativo: 7+5 = 12
      mov eax, eax                    
      mov ebx, 12                     ; esperado = 0xC
      mov efx, n_b3                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
      push 7                          ; parametro
      call triple                     ; triple(7) llama DOS veces a suma desde adentro
      add sp, 4                       ; limpio
; ---- llamada anidada: triple(7) = 21
      mov eax, eax                    
      mov ebx, 21                     ; esperado = 0x15
      mov efx, n_b4                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
; ---- BP del main intacto tras las llamadas (0x7777)
      mov eax, bp                     ; BP actual
      mov ebx, 30583                  ; esperado = 0x7777
      mov efx, n_b5                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
; ---- SP balanceado tras todas las llamadas
      mov eax, sp                     ; SP actual
      mov ebx, l[ds+60]               ; esperado = el SP INICIAL guardado al principio
      mov efx, n_b6                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO

; ============ C) RECURSIVIDAD ============
      push 5                          ; n = 5
      call fact                       ; factorial recursivo
      add sp, 4                       ; limpio
; ---- fact(5) = 120
      mov eax, eax                    
      mov ebx, 120                    ; esperado = 0x78
      mov efx, n_c1                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
      push 1                          ; n = 1 (caso base)
      call fact                       
      add sp, 4                       ; limpio
; ---- fact(1) = 1 (caso base)
      mov eax, eax                    
      mov ebx, 1                      ; esperado = 0x1
      mov efx, n_c2                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
      push 10                         ; n = 10
      call fact                       
      add sp, 4                       ; limpio
; ---- fact(10) = 3628800
      mov eax, eax                    
      mov ebx, 0                      ; esperado = 0x375F00 (se arma con ldh/ldl)
      ldh ebx, 0x0037                 ; parte alta = 0x0037
      ldl ebx, 0x5F00                 ; parte baja = 0x5F00
      mov efx, n_c3                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
      push 10                         ; n = 10
      call fib                        ; fibonacci doblemente recursivo (~177 llamadas)
      add sp, 4                       ; limpio
; ---- fib(10) = 55
      mov eax, eax                    
      mov ebx, 55                     ; esperado = 0x37
      mov efx, n_c4                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
      push 100                        ; n = 100: 100 marcos de 12 bytes anidados
      call sumn                       ; sumn(n) = n + sumn(n-1)
      add sp, 4                       ; limpio
; ---- recursion profunda: sumn(100) = 5050
      mov eax, eax                    
      mov ebx, 5050                   ; esperado = 0x13BA
      mov efx, n_c5                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO
; ---- SP balanceado tras la recursion
      mov eax, sp                     ; SP actual
      mov ebx, l[ds+60]               ; esperado = el SP INICIAL guardado al principio
      mov efx, n_c6                   ; nombre del caso
      call verificar                  ; compara EAX con EBX -> OK / FALLO

; ============ D) argc / argv que la VM dejo para main ============
      mov eax, l[ds+64]               ; argc guardado al inicio
      cmp eax, 0                      ; ¿hay parametros (-p)?
      jz fin_args                     ; no hay -> salto
      ; hay parametros: imprimo argv[1] (el primer parametro; argv[0] es el nombre del .vmx)
      mov edx, l[ds+68]               ; EDX = puntero a argv (arreglo de punteros de 4 bytes)
      add edx, 4                      ; apunta a argv[1]
      mov edx, l[edx+0]               ; EDX = puntero al string del 1er parametro
      sys 4                           ; imprime el 1er parametro
      mov eex, s_nl                   
      call imprimir_ks                ; salto de linea
fin_args: mov edx, ds                     ; EDX = inicio de Data
      add edx, 64                     ; apunto a la celda donde guarde argc
      mov ecx, 1                      ; 1 celda
      ldh ecx, 4                      ; de 4 bytes
      mov eax, 1                      ; formato decimal
      sys 2                           ; muestra 'argc = N'
      ret                             ; RET de main: encuentra el -1 que dejo la VM y TERMINA SIN mensaje

; ======================== SUBRUTINAS ========================
; suma(a,b): devuelve a+b en EAX
suma: push bp                           ; guardo el BP de quien me llamo
      mov bp, sp                        ; BP = base de mi marco
      mov eax, l[bp+8]                  ; 1er parametro (a)
      add eax, l[bp+12]                 ; + 2do parametro (b)
      pop bp                            ; restauro BP
      ret                               ; vuelvo (saca la direccion de retorno)

; resta(a,b): devuelve a-b en EAX
resta: push bp
      mov bp, sp
      mov eax, l[bp+8]                  ; a
      sub eax, l[bp+12]                 ; a - b
      pop bp
      ret

; locales(): usa dos variables locales en la pila y devuelve su suma
locales: push bp
      mov bp, sp
      sub sp, 8                         ; reservo 8 bytes para 2 locales
      mov l[bp-4], 7                    ; local1 = 7
      mov l[bp-8], 5                    ; local2 = 5
      mov eax, l[bp-4]
      add eax, l[bp-8]                  ; 7 + 5
      mov sp, bp                        ; libero las locales
      pop bp
      ret

; triple(x): suma(x,x) y despues suma(ese,x)  -> llamadas anidadas
triple: push bp
      mov bp, sp
      push l[bp+8]                      ; 2do parametro de suma: x
      push l[bp+8]                      ; 1er parametro de suma: x
      call suma                         ; EAX = x + x
      add sp, 8
      push l[bp+8]                      ; 2do parametro: x
      push eax                          ; 1er parametro: x+x
      call suma                         ; EAX = (x+x) + x
      add sp, 8
      pop bp
      ret

; fact(n): n! recursivo
fact: push bp
      mov bp, sp
      mov eax, l[bp+8]                  ; n
      cmp eax, 1
      jp recurre                        ; n > 1 -> recursion
      mov eax, 1                        ; caso base: 1
      jmp fact_fin
recurre: sub eax, 1                     ; n - 1
      push eax
      call fact                         ; EAX = fact(n-1)
      add sp, 4
      mul eax, l[bp+8]                  ; * n
fact_fin: pop bp
      ret

; fib(n): fibonacci recursivo (n<2 -> n)
fib:  push bp
      mov bp, sp
      sub sp, 4                         ; local: guarda fib(n-1)
      mov eax, l[bp+8]                  ; n
      cmp eax, 2
      jn fib_base                       ; n < 2 -> devuelvo n
      sub eax, 1
      push eax
      call fib                          ; fib(n-1)
      add sp, 4
      mov l[bp-4], eax                  ; lo guardo (la proxima llamada pisa EAX)
      mov eax, l[bp+8]
      sub eax, 2
      push eax
      call fib                          ; fib(n-2)
      add sp, 4
      add eax, l[bp-4]                  ; fib(n-1) + fib(n-2)
      jmp fib_fin
fib_base: mov eax, l[bp+8]              ; fib(0)=0, fib(1)=1
fib_fin: mov sp, bp
      pop bp
      ret

; sumn(n): n + sumn(n-1), con sumn(0)=0  -> recursion PROFUNDA
sumn: push bp
      mov bp, sp
      mov eax, l[bp+8]
      cmp eax, 0
      jz sumn_fin                       ; n == 0 -> devuelvo 0
      sub eax, 1
      push eax
      call sumn
      add sp, 4
      add eax, l[bp+8]                  ; + n
sumn_fin: pop bp
      ret

; ---- verificador ----
imprimir_ks: mov edx, ks                ; EDX = inicio del Const Segment
      add edx, eex                      ; + offset del texto
      sys 4
      ret
verificar: mov l[ds+112], eax           ; guardo obtenido
      mov l[ds+116], ebx                ; guardo esperado
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
      mov eax, l[ds+112]
      cmp eax, l[ds+116]
      jz ver_r
      mov eex, s_hdr
      call imprimir_ks
      mov edx, ds
      add edx, 112
      mov ecx, 2
      ldh ecx, 4
      mov eax, 9
      sys 2
ver_r: ret

