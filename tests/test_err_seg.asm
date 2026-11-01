; =====================================================================
; test_err_seg.asm  -  FALLO DE SEGMENTO por acceso mas alla del Data Segment
; DATA = 16 bytes (validos ds+0 ... ds+15).
; Primero se hacen los accesos LEGALES en el borde (deben andar) y despues uno ILEGAL.
; =====================================================================
\\DATA 16                           ; Data de 16 bytes
\\STACK 64                          ; pila
ok1    EQU "Accesos en el borde OK (l[ds+12], b[ds+15])\n"
nunca  EQU "ERROR DEL TEST: esto NO debe verse, la VM debio abortar antes\n"

main: mov eax, 0x1234               ; valor cualquiera
      mov l[ds+12], eax             ; LEGAL: ocupa ds+12..15 (el ultimo long valido)
      mov b[ds+15], 1               ; LEGAL: ultimo byte del segmento
      mov edx, ks
      sys 4                         ; imprime 'Accesos en el borde OK'
      mov ebx, l[ds+14]             ; ILEGAL: ocupa ds+14..17 -> se pasa 2 bytes del limite de 16
      mov edx, ks                   ; no debe ejecutarse
      add edx, nunca
      sys 4                         ; no debe ejecutarse
      stop
