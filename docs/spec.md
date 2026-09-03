# Especificación del lenguaje máquina (esqueleto)

Este archivo documenta el formato del programa que la máquina virtual debe leer.

Formato de instrucción propuesto (por ejemplo):
- Cada instrucción ocupa 2 bytes (16 bits) o 4 bytes (32 bits) según diseño.
- Formato simple de 1 byte opcode + 1 byte operando:
  - 0x01 LOAD <reg>
  - 0x02 STORE <reg>
  - 0x03 ADD <reg>
  - 0x04 SUB <reg>
  - 0x05 JMP <addr>
  - 0x06 JZ <addr>  ; salto si cero
  - 0x07 OUT <reg>
  - 0xFF HALT

Registros:
- R0..R7 (8 registros generales)
- PC (program counter)
- SP (stack pointer) opcional

Memoria:
- Memoria de datos lineal (por ejemplo 64 KiB)

Formato de archivo:
- Encabezado opcional (firma, tamaño)
- Bloque de código: secuencia de instrucciones
- Bloque de datos opcional

Anota aquí las decisiones de diseño de grupo y ejemplos de programas.
