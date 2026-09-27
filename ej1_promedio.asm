inicio:
    mov edx, DS
    mov ebx, 0
    mov eex, 0
    ldh ecx, 0x04
    ldl ecx, 0x01

leer:
    mov eax, 0x01
    sys 0x1
    mov eax, [edx]

    cmp eax, 0
    jz finalizar

    add ebx, eax
    add eex, 1
    jmp leer

finalizar:
    cmp eex, 0
    jz imprimir

    div ebx, eex

imprimir:
    mov [edx], ebx
    mov eax, 0x01
    ldh ecx, 0x04
    ldl ecx, 0x01
    sys 0x2

    stop