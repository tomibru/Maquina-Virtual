# Máquina Virtual — Trabajo práctico
Trabajo Práctico de Fundamentos de la Arquitectura de Computadoras (UNMDP, 2026)

El Equipo
* Bruses Tomas
* Fernandez Monteverde Dolores
* Ferrando Ignacio

Este proyecto consiste en emular desde cero una computadora de 32 bits en lenguaje C. Básicamente, agarramos el código máquina (*.vmx) que sale del traductor de la cátedra, lo cargamos en una RAM virtual de 16 KiB, armamos la tabla de segmentos y nos ponemos a ejecutar un ciclo fetch-decode-execute que procesa 26 instrucciones a puro manejo de punteros y operaciones a nivel de bits.

Estructura del Código

cpu.c / cpu.h: El corazón de la bestia. Junta el decodificador, la tabla de saltos (tabla_opc) y las funciones de ejecución de las 26 instrucciones (aritméticas, lógicas, saltos y SYS).   
mmu.c / mmu.h: La encargada de traducir direcciones lógicas a físicas y vigilar que nadie se pase de vivo queriendo pisar memoria ajena (¡hola, fallos de segmento!).   
mv.c / mv.h: El núcleo que inicializa los registros (IP, CS, DS, CC, etc.), levanta la cabecera VMX26 del binario y arranca la máquina. 

Compilaion y ejecucion

Para compilar todo el proyecto al mismo momento hay que escribir lo siguiente en la terminal:
    gcc -o vmx *.c -Wall -Wextra

Modos de uso

Ejecución normal
    ./vmx programa.vmx

Modo Disassembler
    ./vmx programa.vmx -d