; =====================================================================
; test_err_stack_underflow.asm  -  STACK UNDERFLOW por POP con la pila vacia
; OJO: la pila de main NO arranca vacia: la VM dejo 3 valores (retorno -1, argc, argv).
; Por eso los primeros 3 POP son LEGALES y recien el CUARTO da STACK UNDERFLOW.
; =====================================================================
\\DATA 16                           ; Data
\\STACK 64                          ; pila de 64 bytes
bien   EQU "3 POP legales (se llevaron retorno, argc y argv)\n"
nunca  EQU "ERROR DEL TEST: esto NO debe verse, la VM debio abortar antes\n"

main: pop eax                       ; POP 1 (legal): saca el retorno (-1)
      pop ebx                       ; POP 2 (legal): saca argc
      pop ecx                       ; POP 3 (legal): saca argv
      mov edx, ks
      sys 4                         ; imprime '3 POP legales'
      pop edx                       ; POP 4: la pila esta VACIA -> STACK UNDERFLOW
      mov edx, ks                   ; no debe ejecutarse
      add edx, nunca
      sys 4
      stop
