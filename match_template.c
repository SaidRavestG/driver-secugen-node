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
    unsigned char *tpl1 = (unsigned char *)malloc(378);
    unsigned char *tpl2 = (unsigned char *)malloc(378);
    
    if (!tpl1 || !tpl2) {
        printf("❌ Error: No se pudo asignar memoria\n");
        if (tpl1) free(tpl1);
        if (tpl2) free(tpl2);
        fclose(file1);
        fclose(file2);
        return -1;
    }

    size_t read1 = fread(tpl1, 1, 378, file1);
    size_t read2 = fread(tpl2, 1, 378, file2);
    
    fclose(file1);
    fclose(file2);

    if (read1 != 378 || read2 != 378) {
        printf("❌ Error: Lectura incompleta de templates (leído1: %zu, leído2: %zu)\n", read1, read2);
        free(tpl1);
        free(tpl2);
        return -1;
    }

    // Imprimir primeros 10 bytes de cada template
    printf("📋 Primeros 10 bytes de tpl1: ");
    for (int i = 0; i < 10; i++) printf("%02X ", tpl1[i]);
    printf("\n");

    printf("📋 Primeros 10 bytes de tpl2: ");
    for (int i = 0; i < 10; i++) printf("%02X ", tpl2[i]);
    printf("\n");

    // Inicializar el SDK de SecuGen
    HSGFPM sgfplib = NULL;  // Inicializar explícitamente a NULL
    DWORD ret = SGFPM_Create(&sgfplib);
    if (ret != SGFDX_ERROR_NONE || !sgfplib) {
        printf("❌ Error creando SGFPM: %lu\n", ret);
        free(tpl1);
        free(tpl2);
        return -1;
    }

    ret = SGFPM_Init(sgfplib, SG_DEV_AUTO);
    if (ret != SGFDX_ERROR_NONE) {
        printf("❌ Error inicializando SGFPM: %lu\n", ret);
        SGFPM_Terminate(sgfplib);
        free(tpl1);
        free(tpl2);
        return -1;
    }

    // Verificar que sgfplib no es NULL
    if (sgfplib == NULL) {
        printf("❌ Error: `sgfplib` no está inicializado correctamente.\n");
        return -1;
    }

    // Comparar templates directamente sin conversión
    BOOL matched = FALSE;
    DWORD score = 0;
    printf("⚖️ Comparando templates...\n");
    
    // Obtener puntaje de coincidencia primero
    ret = SGFPM_GetMatchingScore(sgfplib, tpl1, tpl2, &score);
    if (ret != SGFDX_ERROR_NONE) {
        printf("❌ Error obteniendo puntaje de coincidencia: %lu\n", ret);
        SGFPM_Terminate(sgfplib);
        free(tpl1);
        free(tpl2);
        return -1;
    }
    
    printf("📊 Puntaje de coincidencia: %lu\n", score);
    
    // Definir un umbral de puntaje mínimo
    const DWORD MIN_SCORE = 50;  // Ajusta este valor según sea necesario
    
    // Probar con diferentes niveles de seguridad
    int security_levels[] = {1, 2, 3, 4, 5};
    BOOL matched_any = FALSE;
    
    for (int i = 0; i < 5; i++) {
        BOOL current_matched = FALSE;
        ret = SGFPM_MatchAnsiTemplate(sgfplib, tpl1, SG_IMPTYPE_ANSI378, 
                                    tpl2, SG_IMPTYPE_ANSI378, 
                                    security_levels[i], &current_matched);
        
        if (ret != SGFDX_ERROR_NONE) {
            printf("❌ Error en comparación con nivel %d: %lu\n", security_levels[i], ret);
            continue;
        }
        
        printf("🔍 Nivel de seguridad %d: %s (Score: %lu)\n", 
               security_levels[i], 
               current_matched ? "Coincide" : "No coincide",
               score);
               
        if (current_matched) matched_any = TRUE;
    }

    // Considerar coincidencia si el puntaje es suficiente o si coincidió en algún nivel
    matched = (score >= MIN_SCORE) || matched_any;

    printf("\n📋 Resultado final:\n");
    printf("📊 Puntaje: %lu (Mínimo requerido: %lu)\n", score, MIN_SCORE);
    if (matched)
        printf("✅ Las huellas coinciden\n");
    else
        printf("❌ Las huellas NO coinciden\n");

    // Verificar el tamaño del template
    DWORD templateSize = 0;
    ret = SGFPM_GetTemplateSize(sgfplib, tpl1, &templateSize);
    if (ret == SGFDX_ERROR_NONE) {
        printf("📋 Tamaño calculado del template 1: %lu bytes\n", templateSize);
    }
    
    ret = SGFPM_GetTemplateSize(sgfplib, tpl2, &templateSize);
    if (ret == SGFDX_ERROR_NONE) {
        printf("📋 Tamaño calculado del template 2: %lu bytes\n", templateSize);
    }

    SGFPM_Terminate(sgfplib);
    free(tpl1);
    free(tpl2);
    return 0;
}
