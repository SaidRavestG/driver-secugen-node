#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sgfplib.h"

// Definir TRUE y FALSE si no están definidos
#ifndef TRUE
    #define TRUE 1
#endif

#ifndef FALSE
    #define FALSE 0
#endif

// Definir constantes manualmente si no existen en el SDK
#ifndef SG_IMPTYPE_ANSI378
    #define SG_IMPTYPE_ANSI378 0x0101  // ANSI-378
#endif

#ifndef SGFDX_SECURITY_MEDIUM
    #define SGFDX_SECURITY_MEDIUM 3 // Nivel de seguridad medio
#endif

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Uso: %s <template1> <template2>\n", argv[0]);
        return -1;
    }

    // Abrir archivos de templates
    FILE *file1 = fopen(argv[1], "rb");
    FILE *file2 = fopen(argv[2], "rb");
    if (!file1 || !file2) {
        printf("❌ Error abriendo archivos\n");
        return -1;
    }

    // Verificar tamaño de archivos
    fseek(file1, 0, SEEK_END);
    long size1 = ftell(file1);
    fseek(file2, 0, SEEK_END);
    long size2 = ftell(file2);
    rewind(file1);
    rewind(file2);

    printf("📏 Tamaño de tpl1: %ld bytes\n", size1);
    printf("📏 Tamaño de tpl2: %ld bytes\n", size2);

    if (size1 != 378 || size2 != 378) {
        printf("❌ Error: Los templates no tienen el tamaño correcto (378 bytes).\n");
        fclose(file1);
        fclose(file2);
        return -1;
    }

    // Leer templates en memoria
    unsigned char tpl1[378], tpl2[378];
    fread(tpl1, sizeof(tpl1), 1, file1);
    fread(tpl2, sizeof(tpl2), 1, file2);
    fclose(file1);
    fclose(file2);

    // Imprimir primeros 10 bytes de cada template
    printf("📋 Primeros 10 bytes de tpl1: ");
    for (int i = 0; i < 10; i++) printf("%02X ", tpl1[i]);
    printf("\n");

    printf("📋 Primeros 10 bytes de tpl2: ");
    for (int i = 0; i < 10; i++) printf("%02X ", tpl2[i]);
    printf("\n");

    // Inicializar el SDK de SecuGen
    HSGFPM sgfplib;
    DWORD ret = SGFPM_Create(&sgfplib);
    if (ret != SGFDX_ERROR_NONE) {
        printf("❌ Error creando SGFPM: %lu\n", ret);
        return -1;
    }

    ret = SGFPM_Init(sgfplib, SG_DEV_AUTO);
    if (ret != SGFDX_ERROR_NONE) {
        printf("❌ Error inicializando SGFPM: %lu\n", ret);
        SGFPM_Terminate(sgfplib);
        return -1;
    }

    // Verificar que sgfplib no es NULL
    if (sgfplib == NULL) {
        printf("❌ Error: `sgfplib` no está inicializado correctamente.\n");
        return -1;
    }

    // Comparar templates directamente sin conversión
    BOOL matched = FALSE;
    printf("⚖️ Comparando templates...\n");
    ret = SGFPM_MatchAnsiTemplate(sgfplib, tpl1, SG_IMPTYPE_ANSI378, tpl2, SG_IMPTYPE_ANSI378, SGFDX_SECURITY_MEDIUM, &matched);

    // Manejar errores antes de acceder a `matched`
    if (ret != SGFDX_ERROR_NONE) {
        printf("❌ Error comparando templates: %lu\n", ret);
        SGFPM_Terminate(sgfplib);
        return -1;
    }

    // Mostrar resultado de la comparación
    if (matched)
        printf("✅ Las huellas coinciden\n");
    else
        printf("❌ Las huellas NO coinciden\n");

    SGFPM_Terminate(sgfplib);
    return 0;
}
