; =====================================================================
; test_err_seg_noinit.asm  -  acceso a un segmento que NO EXISTE (Extra Segment de tamaño 0)
; Con EXTRA = 0 el registro ES debe quedar en -1 (0xFFFFFFFF). Usarlo debe dar FALLO DE SEGMENTO.
; =====================================================================
\\DATA 16                           ; Data
\\EXTRA 0                           ; NO hay Extra Segment -> ES = -1
\\STACK 64                          ; pila
nunca  EQU "ERROR DEL TEST: esto NO debe verse, la VM debio abortar antes\n"

main: mov eax, 5                    ; valor cualquiera
      mov l[es+0], eax              ; ES = -1 -> segmento 0xFFFF (no existe en la tabla) -> FALLO DE SEGMENTO
      mov edx, ks                   ; no debe ejecutarse
      sys 4
      stop
