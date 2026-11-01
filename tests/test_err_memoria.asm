; =====================================================================
; test_err_memoria.asm  -  MEMORIA INSUFICIENTE (la memoria por defecto es 16 KiB = 16384 bytes)
; =====================================================================
\\DATA 20000                        ; 20000 bytes de Data: no entran en 16384
main: stop
