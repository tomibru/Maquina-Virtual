; =====================================================================
; test_err_div0.asm  -  DIVISION POR CERO: la VM debe avisar y ABORTAR
; Caso severo: el divisor es AL = 0 mientras que EAX NO es cero (0x00001200).
; Si la VM leyera el registro completo en vez del sector AL, NO detectaria la division por cero.
; =====================================================================
\\DATA 16                           ; Data (no se usa, pero queda definido)
\\STACK 64                          ; pila
antes  EQU "Antes de dividir: todo bien\n"
nunca  EQU "ERROR DEL TEST: esto NO debe verse, la VM debio abortar antes\n"

main: mov edx, ks                   ; EDX = inicio de KS (offset 0 = 'antes')
      sys 4                         ; imprime 'Antes de dividir: todo bien'
      mov eax, 0x1200               ; EAX = 0x00001200  (AL = 0x00, AH = 0x12)
      mov ebx, 100                  ; dividendo
      div ebx, al                   ; EBX / AL  ->  AL vale 0  ->  DIVISION POR CERO
      mov edx, ks                   ; si llegamos aca, la VM NO aborto
      add edx, nunca                ; apunta al cartel de error del test
      sys 4                         ; no debe ejecutarse
      stop                          ; no debe ejecutarse
