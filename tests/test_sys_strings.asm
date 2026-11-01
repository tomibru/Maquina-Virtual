; =====================================================================
; test_sys_strings.asm  -  Const Segment, SYS 4 (escribir), SYS 3 (leer), SYS 0 (limpiar), SYS F (breakpoint)
; Es INTERACTIVO: hay que tipear 4 lineas EXACTAS (ver cada pedido en pantalla):
;     [1] 12345          [2] abcdefgh          [3] hola mundo cruel          [4] (solo Enter)
; Despues de cada lectura se verifica en memoria (OK / FALLO) y se imprime lo guardado entre [ ]
; =====================================================================
\\DATA 128                          ; Data: buffers de lectura (ds+0, ds+16, ds+32, ds+64) y celdas del verificador (ds+112)
\\STACK 256                         ; pila para los CALL
\\EXTRA 16                          ; Extra (no se usa, pero queda definido)

; ---- constantes (van al Const Segment; cada nombre vale el OFFSET dentro de KS) ----
t_hola EQU "Hola desde el Const Segment (SYS 4)\n"
t_seg  EQU "Segunda constante: prueba el offset dentro de KS\n"
t_esc  EQU "Escapes: [\tTAB] [barra: \\] [comilla: ']\n"
p1     EQU "\n[1] Escribi EXACTAMENTE 5 letras, ej 12345 (ECX=5): "
p2     EQU "[2] Escribi MAS de 5 letras, ej abcdefgh (ECX=5 corta): "
p3     EQU "[3] Escribi lo que quieras, ej hola mundo cruel (ECX=-1): "
p4     EQU "[4] Apreta solo Enter (cadena vacia, ECX=5): "
g1     EQU "    guardado: ["
g2     EQU "]\n"
p5     EQU "\n[5] Apreta Enter para limpiar la pantalla (SYS 0): "
t_bp1  EQU "Pantalla limpia. Ahora SYS F (breakpoint #1)\n"
t_bp2  EQU "Seguiste despues del breakpoint #1. Ahora SYS F (breakpoint #2)\n"
t_fin  EQU "FIN: seguiste despues del breakpoint #2\n"
s_ok   EQU "OK     "
s_fa   EQU "FALLO  "
s_nl   EQU "\n"
s_hdr  EQU "   (obtenido, esperado):\n"
c1     EQU "[1] limite exacto: ds[4] = '5'"
c2     EQU "[1] limite exacto: ds[5] = 0 (terminador justo despues)"
c3     EQU "[1] limite exacto: ds[6] = '#' (no se piso nada mas)"
c4     EQU "[2] se corto en 5: ds[16+4] = 'e'"
c5     EQU "[2] se corto en 5: ds[16+5] = 0"
c6     EQU "[2] se corto en 5: ds[16+6] = '#' (las letras 'fgh' NO se guardaron)"
c7     EQU "[3] ECX=-1 guarda todo: ds[32+15] = 'l'"
c8     EQU "[3] ECX=-1: terminador en ds[32+16] = 0"
c9     EQU "[3] las letras 'fgh' del [2] NO se colaron en la lectura [3]: ds[32] = 'h'"
c10    EQU "[4] linea vacia: ds[64] = 0"
c11    EQU "[4] linea vacia: ds[65] = '#' (no escribio de mas)"

main: mov edx, ks                   ; EDX = inicio de KS  (offset 0 = primera constante)
      sys 4                         ; imprime t_hola
      mov edx, ks                   ; EDX = inicio de KS
      add edx, t_seg                ; + offset de la 2da constante
      sys 4                         ; imprime t_seg (prueba que el offset dentro de KS funciona)
      mov edx, ks
      add edx, t_esc
      sys 4                         ; imprime t_esc (tabulacion, barra y comilla)

      mov edx, ds                   ; relleno TODA la Data con '#' para detectar escrituras de mas
      mov ecx, 128                  ; 128 bytes
fill: mov b[edx+0], 0x23            ; '#' = 0x23
      add edx, 1                    ; siguiente byte
      sub ecx, 1                    ; uno menos
      jnz fill                      ; repite hasta ECX = 0

      ; ---------- lectura 1: limite EXACTO (5 letras, ECX = 5) ----------
      mov eex, p1
      call imprimir_ks              ; pide texto
      mov edx, ds                   ; EDX = donde guardar (ds+0)
      mov ecx, 5                    ; ECX = maximo 5 caracteres
      sys 3                         ; lee una linea del teclado
      mov eex, g1
      call imprimir_ks              ; '    guardado: ['
      mov edx, ds
      sys 4                         ; imprime lo guardado
      mov eex, g2
      call imprimir_ks              ; ']'
      mov eax, b[ds+4]
      mov ebx, 0x35                 ; '5'
      mov efx, c1
      call verificar
      mov eax, b[ds+5]
      mov ebx, 0
      mov efx, c2
      call verificar
      mov eax, b[ds+6]
      mov ebx, 0x23
      mov efx, c3
      call verificar

      ; ---------- lectura 2: se pasa del limite (ECX = 5) ----------
      mov eex, p2
      call imprimir_ks
      mov edx, ds
      add edx, 16                   ; guardar en ds+16
      mov ecx, 5
      sys 3                         ; 'abcdefgh' -> guarda 'abcde', DESCARTA 'fgh'
      mov eex, g1
      call imprimir_ks
      mov edx, ds
      add edx, 16
      sys 4
      mov eex, g2
      call imprimir_ks
      mov eax, b[ds+20]
      mov ebx, 0x65                 ; 'e'
      mov efx, c4
      call verificar
      mov eax, b[ds+21]
      mov ebx, 0
      mov efx, c5
      call verificar
      mov eax, b[ds+22]
      mov ebx, 0x23
      mov efx, c6
      call verificar

      ; ---------- lectura 3: sin limite (ECX = -1) ----------
      mov eex, p3
      call imprimir_ks
      mov edx, ds
      add edx, 32                   ; guardar en ds+32
      mov ecx, -1                   ; -1 = sin limite
      sys 3
      mov eex, g1
      call imprimir_ks
      mov edx, ds
      add edx, 32
      sys 4
      mov eex, g2
      call imprimir_ks
      mov eax, b[ds+47]
      mov ebx, 0x6C                 ; 'l' (ultima letra de 'hola mundo cruel')
      mov efx, c7
      call verificar
      mov eax, b[ds+48]
      mov ebx, 0
      mov efx, c8
      call verificar
      mov eax, b[ds+32]
      mov ebx, 0x68                 ; 'h' (si fuera 'f' se habria colado el resto de la lectura 2)
      mov efx, c9
      call verificar

      ; ---------- lectura 4: linea vacia ----------
      mov eex, p4
      call imprimir_ks
      mov edx, ds
      add edx, 64
      mov ecx, 5
      sys 3                         ; solo Enter -> cadena vacia
      mov eax, b[ds+64]
      mov ebx, 0
      mov efx, c10
      call verificar
      mov eax, b[ds+65]
      mov ebx, 0x23
      mov efx, c11
      call verificar

      ; ---------- SYS 0 (limpiar pantalla) y SYS F (breakpoint) ----------
      mov eex, p5
      call imprimir_ks
      mov edx, ds
      add edx, 96
      mov ecx, 2
      sys 3                         ; espera un Enter (guarda como maximo 2 letras en ds+96)
      sys 0                         ; LIMPIA LA PANTALLA (se pierde todo lo anterior)
      mov eex, t_bp1
      call imprimir_ks
      sys 0xF                       ; BREAKPOINT #1  (sin .vmi se ignora; con .vmi pausa y guarda la imagen)
      mov eex, t_bp2
      call imprimir_ks
      sys 0xF                       ; BREAKPOINT #2
      mov eex, t_fin
      call imprimir_ks
      ret                           ; fin limpio de main (RET con -1 en la pila)

; ---- imprimir_ks: imprime el texto cuyo OFFSET en KS esta en EEX ----
imprimir_ks: mov edx, ks
      add edx, eex
      sys 4
      ret

; ---- verificar: EAX = obtenido, EBX = esperado, EFX = nombre ----
verificar: mov l[ds+112], eax
      mov l[ds+116], ebx
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
