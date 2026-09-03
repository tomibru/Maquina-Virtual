# Máquina Virtual — Trabajo práctico

Trabajo práctico en grupo: implementar una máquina virtual en C que lea un formato de lenguaje máquina (binario o texto) y ejecute operaciones simples.

Características iniciales:
- Lenguaje objetivo: instrucciones simples (cargar, almacenar, sumar, restar, saltos, I/O)
- Implementación en C (ANSI C / C11 compatible)
- Compilación con Makefile

Cómo empezar:
1. Clonar el repositorio:
   git clone https://github.com/tomibru/Maquina-Virtual.git
2. Entrar al directorio del proyecto:
   cd Maquina-Virtual
3. Compilar:
   make
4. Ejecutar (ejemplo):
   ./bin/vm program.bin

Estructura propuesta:
- src/: código fuente en C
- docs/: especificación del lenguaje máquina y notas
- tests/: (vacío por ahora) programas de prueba

Contribuciones:
Lee CONTRIBUTING.md para las reglas del grupo.

Autores: Grupo de trabajo
