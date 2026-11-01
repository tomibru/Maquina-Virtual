; =====================================================================
; test_err_overflow_call.asm  -  STACK OVERFLOW provocado por CALL (recursion infinita)
; El enunciado dice que CALL tambien debe detectar el overflow.
; =====================================================================
\\DATA 16                           ; Data
\\STACK 64                          ; pila de 64 bytes (aguanta 13 CALL)
main: call main                     ; se llama a si mismo para siempre: cada CALL apila 4 bytes
