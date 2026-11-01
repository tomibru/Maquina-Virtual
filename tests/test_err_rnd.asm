; =====================================================================
; test_err_rnd.asm  -  RND con tope -1 (0xFFFFFFFF): el modulo (tope+1) vale 0 -> division por cero en C
; =====================================================================
\\DATA 16                           ; Data
\\STACK 64                          ; pila
main: rnd eax, -1                   ; tope = -1  ->  rand() % (0xFFFFFFFF + 1) = rand() % 0
      stop
