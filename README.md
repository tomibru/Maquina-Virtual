# Máquina Virtual

Trabajo Práctico de **Fundamentos de la Arquitectura de Computadoras** — Facultad de Ingeniería, UNMDP (2026).

Emulador de una computadora de 32 bits escrito en **C**. Toma un programa en código máquina (`.vmx`), generado por el traductor `vmt` que provee la cátedra a partir de un archivo Assembler (`.asm`), lo carga en una RAM virtual y lo ejecuta instrucción por instrucción. También puede guardar y retomar el estado completo de la máquina mediante archivos de imagen (`.vmi`).

## Integrantes

- Bruses, Tomás
- Fernandez Monteverde, Dolores
- Ferrando, Ignacio

## Índice

- [Flujo de trabajo](#flujo-de-trabajo)
- [Características](#características)
- [Estructura del proyecto](#estructura-del-proyecto)
- [Arquitectura de la máquina](#arquitectura-de-la-máquina)
- [Conjunto de instrucciones](#conjunto-de-instrucciones)
- [Llamadas al sistema (SYS)](#llamadas-al-sistema-sys)
- [Depuración](#depuración)
- [Errores detectados](#errores-detectados)
- [Formatos de archivo](#formatos-de-archivo)

## Flujo de trabajo

```
programa.asm ──► vmt ──► programa.vmx ──► vmx ──► resultados
                                            │
                                            └──► programa.vmi ──► vmg (debugger)
```

1. **Traducción:** `vmt` (provisto por la cátedra) convierte el `.asm` en un binario `.vmx`.
2. **Ejecución:** `vmx` (este proyecto) lee el `.vmx`, arma la memoria, los segmentos y los registros, y emula el programa.
3. **Depuración (opcional):** al ejecutar un breakpoint (`SYS 0xF`), `vmx` vuelca el estado a un `.vmi`, que `vmg` (también provisto por la cátedra) muestra en tiempo real.

## Características

**Parte I**
- Memoria principal de 16 KiB y tabla de descriptores de segmentos (8 entradas).
- 32 registros de 4 bytes, incluyendo el registro de condición `CC` (flags N, Z, C, V).
- Ciclo fetch → decode → execute con tabla de saltos (`tabla_opc`) para las instrucciones.
- Traducción de direcciones lógicas a físicas con verificación de límites de segmento.
- Llamadas al sistema `READ` y `WRITE` con soporte de modos decimal, octal, hexadecimal, binario y caracteres.
- Disassembler (`-d`).

**Parte II**
- Memoria de tamaño variable (`m=M`, en KiB).
- Hasta seis segmentos: Param, Const, Code, Data, Extra y Stack.
- Pila con `PUSH`, `POP`, `CALL` y `RET`, con detección de *stack overflow* y *underflow*.
- Modificadores de operandos de memoria (`b[]`, `w[]`, `l[]`) y pseudónimos de registros (`AL`, `AH`, `AX`, …).
- Pasaje de parámetros a la subrutina principal (`-p`) mediante el Param Segment.
- Llamadas al sistema `STRING READ`, `STRING WRITE`, `CLEAR SCREEN` y `BREAKPOINT`.
- Archivos de imagen `.vmi` para guardar y retomar la ejecución.
- Compatibilidad con cabeceras de **versión 1 y 2** del `.vmx`.
- Disassembler ampliado: constantes string, entry point (`>`), pseudónimos y modificadores.

## Estructura del proyecto

```
Maquina-Virtual/
├── main.c                      # Punto de entrada: parseo de args, carga y ejecución
├── mv_const.h                  # Constantes y códigos de registros
├── Makefile
├── args/                       # Parseo de la línea de comandos
├── cpu/                        # Decodificador, ejecución de instrucciones y syscalls
├── mmu/                        # Traducción lógica → física y fallos de segmento
├── pila/                       # Operaciones de pila (PUSH/POP/CALL/RET)
├── imagen/                     # Guardado/carga de .vmi y breakpoints
├── disassembler/               # Modo -d
├── validacionArchivo/          # Lectura y validación del .vmx, armado de segmentos
├── componentes/                # Memoria, registros, tabla de segmentos y estructura de la MV
├── vmt / vmt.exe               # Traductor (provisto por la cátedra)
└── vmg / vmg.exe               # Debugger (provisto por la cátedra)
```

| Módulo | Responsabilidad |
| --- | --- |
| `cpu` | Fetch/decode de instrucciones, operandos, flags de `CC`, las instrucciones y las llamadas al sistema. |
| `mmu` | Traduce direcciones lógicas a físicas y lanza *fallo de segmento* ante accesos inválidos. |
| `pila` | Reserva, escritura y lectura del tope de la pila, con control de overflow/underflow. |
| `validacionArchivo` | Valida la cabecera `VMX26`, calcula tamaños, arma el Param Segment, la tabla de segmentos y la pila inicial. |
| `imagen` | Serializa y restaura el estado de la máquina; implementa el breakpoint interactivo. |
| `disassembler` | Recorre el Code Segment y el Const Segment e imprime el código desensamblado. |
| `args` | Interpreta `.vmx`, `.vmi`, `m=M`, `-d` y `-p`. |
| `componentes` | Estructuras de datos de la máquina (memoria, registros, tabla de segmentos). |

## Arquitectura de la máquina

### Memoria y segmentos

La memoria es un arreglo de bytes de tamaño configurable (16 KiB por defecto). Los segmentos se cargan de forma contigua y en este orden; solo se crean los que tienen tamaño mayor a cero:

```
Param ─► Const ─► Code ─► Data ─► Extra ─► Stack
```

Los tamaños de Data, Extra y Stack se definen con las directivas `\\DATA`, `\\EXTRA` y `\\STACK` en el `.asm` (1024 por defecto). Si el proceso no entra en memoria, la máquina informa **memoria insuficiente** y aborta.

Cada entrada de la **tabla de descriptores** ocupa 4 bytes: 2 de base y 2 de tamaño. Las entradas no usadas valen `0xFFFFFFFF`.

### Direcciones lógicas y físicas

Un puntero lógico son 4 bytes: 2 para el **código de segmento** (índice en la tabla) y 2 para el **offset**. La MMU obtiene la base del segmento, le suma el offset y verifica que el acceso completo (cantidad de bytes incluida) caiga dentro del segmento.

### Registros

| Código | Nombre | Uso |
| :-: | --- | --- |
| 0 | `IP` | Puntero a la próxima instrucción |
| 1–3 | `OPC`, `OP1`, `OP2` | Instrucción en curso (código de operación y operandos) |
| 4–6 | `LAR`, `MAR`, `MBR` | Acceso a memoria |
| 7–8 | `SP`, `BP` | Manejo de pila |
| 10–15 | `EAX` … `EFX` | Propósito general |
| 16 | `AC` | Acumulador (también guarda el resto en `DIV`) |
| 17 | `CC` | Código de condición: bits **N Z C V** |
| 26–31 | `CS`, `DS`, `ES`, `SS`, `KS`, `PS` | Punteros al inicio de cada segmento (`-1` si el segmento no existe) |

Los registros `EAX`–`EFX` admiten pseudónimos de sector: `EAX` (4 bytes), `AX` (2 bytes), `AH` y `AL` (1 byte).

### Pila

Crece hacia direcciones menores. `SP` arranca apuntando justo después del Stack Segment. `PUSH`, `POP`, `CALL` y `RET` siempre trabajan con 4 bytes en *big endian*. Al iniciar, la pila contiene (de tope a fondo): dirección de retorno (`-1`), `argc` y `argv` (o `-1` si no hay parámetros).

## Conjunto de instrucciones

| Operandos | Instrucciones |
| --- | --- |
| **Dos** | `MOV` `ADD` `SUB` `MUL` `DIV` `CMP` `AND` `OR` `XOR` `SWAP` `SHL` `SHR` `SAR` `LDL` `LDH` `RND` |
| **Uno** | `SYS` `JMP` `JP` `JN` `JZ` `JC` `JV` `JNP` `JNN` `JNZ` `NOT` `PUSH` `POP` `CALL` |
| **Ninguno** | `STOP` `RET` |

Las instrucciones tienen longitud variable. El primer byte codifica los tipos de operando y el código de operación; los operandos (registro, inmediato o memoria) se almacenan a continuación, en orden inverso al del Assembler.

Las operaciones `MOV, ADD, SUB, MUL, DIV, CMP, AND, OR, XOR, SWAP, SHL, SHR, SAR` y `NOT` actualizan los flags del registro `CC`.

## Llamadas al sistema (SYS)

| Código | Nombre | Descripción |
| :-: | --- | --- |
| `0x0` | CLEAR SCREEN | Limpia la pantalla. |
| `0x1` | READ | Lee valores del teclado a partir de `EDX`. `ECX` indica cantidad (parte baja) y tamaño de celda (parte alta); `EAX` indica el formato (decimal, caracteres, octal, hexadecimal, binario). |
| `0x2` | WRITE | Escribe valores a partir de `EDX`, con los mismos registros que `READ`. Admite varios formatos a la vez. |
| `0x3` | STRING READ | Lee una cadena del teclado y la guarda en `EDX`; `ECX` es el máximo de caracteres (`-1` = sin límite). |
| `0x4` | STRING WRITE | Imprime la cadena que empieza en `EDX` hasta el `\0`. |
| `0xF` | BREAKPOINT | Pausa y genera la imagen `.vmi` (se ignora si no se indicó un `.vmi`). |

En `READ` y `WRITE` se muestra un prompt `[XXXX]:` con la dirección física de cada celda.

## Depuración

Cuando se ejecuta un breakpoint y se pasó un `.vmi`, la máquina guarda su estado y espera una acción:

| Tecla | Acción |
| :-: | --- |
| `g` | Continuar hasta el próximo breakpoint o el final. |
| `q` | Abortar dejando intacto el `.vmi`, para poder retomarlo después. |
| `Enter` | Ejecutar una sola instrucción y volver a frenar (paso a paso). |

El debugger `vmg` monitorea el `.vmi` y muestra el código desensamblado, la posición de `IP`, los registros y la tabla de segmentos.

## Errores detectados

Ante cualquiera de estos errores, la máquina informa el problema y aborta la ejecución:

- **Instrucción inválida:** código de operación inexistente.
- **División por cero:** segundo operando de `DIV` igual a 0.
- **Fallo de segmento:** segmento inexistente o no inicializado, o acceso fuera de los límites del segmento.
- **Memoria insuficiente:** el proceso no entra en la memoria principal.
- **Stack overflow / underflow:** `PUSH`/`CALL` sin espacio, o `POP`/`RET` con la pila vacía.
- **Archivo inválido:** identificador, versión o cabecera incorrectos, o archivo truncado.

## Formatos de archivo

**Programa `.vmx`**

| Bytes | Campo | Valor |
| :-: | --- | --- |
| 0–4 | Identificador | `"VMX26"` |
| 5 | Versión | `1` o `2` |
| 6–7 | Tamaño del Code Segment | — |
| 8–9 | Tamaño del Data Segment | — *(v2)* |
| 10–11 | Tamaño del Extra Segment | — *(v2)* |
| 12–13 | Tamaño del Stack Segment | — *(v2)* |
| 14–15 | Tamaño del Const Segment | — *(v2)* |
| 16–17 | Offset del entry point | — *(v2)* |

A la cabecera le siguen el código máquina y el contenido del Const Segment. La versión 1 solo trae el tamaño del código.

**Imagen `.vmi`**

| Sección | Tamaño (bytes) |
| --- | :-: |
| Cabecera (`"VMI26"`, versión 1, tamaño de memoria en KiB) | 8 |
| Registros (32 × 4) | 128 |
| Tabla de descriptores de segmentos (8 × 4) | 32 |
| Memoria principal | variable |