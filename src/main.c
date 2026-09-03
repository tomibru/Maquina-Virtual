#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// Ejemplo mínimo: lectura de un archivo binario e impresión de bytes
int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <programa.bin>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    FILE *f = fopen(filename, "rb");
    if (!f) {
        perror("fopen");
        return 1;
    }

    // Determinar tamaño
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    uint8_t *mem = malloc(size);
    if (!mem) {
        perror("malloc");
        fclose(f);
        return 1;
    }

    if (fread(mem, 1, size, f) != (size_t)size) {
        perror("fread");
        free(mem);
        fclose(f);
        return 1;
    }

    fclose(f);

    // Por ahora: imprimir bytes leídos
    for (long i = 0; i < size; ++i) {
        printf("%02X ", mem[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n");

    free(mem);
    return 0;
}
