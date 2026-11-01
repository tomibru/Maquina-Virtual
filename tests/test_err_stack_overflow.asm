; =====================================================================
; test_err_stack_overflow.asm  -  STACK OVERFLOW por PUSH infinito
; STACK = 64 bytes. La VM ya uso 12 al arrancar (retorno -1, argc, argv) -> quedan 52 = EXACTAMENTE 13 pushes.
; El PUSH numero 14 debe dar STACK OVERFLOW.
; =====================================================================
\\DATA 16                           ; Data
\\STACK 64                          ; pila de 64 bytes
bien   EQU "13 PUSH legales hechos (la pila quedo llena)\n"
nunca  EQU "ERROR DEL TEST: esto NO debe verse, la VM debio abortar antes\n"

main: mov ecx, 13                   ; contador: 13 pushes legales
llena: push eax                     ; apila 4 bytes
      sub ecx, 1                    ; uno menos
      jnz llena                     ; repite hasta ECX = 0
      mov edx, ks
      sys 4                         ; imprime '13 PUSH legales hechos'
infin: push eax                     ; PUSH numero 14: SP baja por debajo de SS -> STACK OVERFLOW
      jmp infin                     ; bucle infinito (nunca deberia llegar a repetir)
      mov edx, ks                   ; no debe ejecutarse
      add edx, nunca
      sys 4
      stop
