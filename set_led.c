#include <stdio.h>
#include <unistd.h>
#include "sgfplib.h"

int main() {
    HSGFPM sgfplib;
    DWORD ret;
    
    printf("Iniciando programa de captura...\n");
    
    ret = SGFPM_Create(&sgfplib);
    if (ret != SGFDX_ERROR_NONE) {
        printf("Error en SGFPM_Create: %lu\n", ret);
        return -1;
    }
    printf("SGFPM_Create OK\n");

    ret = SGFPM_Init(sgfplib, SG_DEV_AUTO);
    if (ret != SGFDX_ERROR_NONE) {
        printf("Error en SGFPM_Init: %lu\n", ret);
        return -1;
    }
    printf("SGFPM_Init OK\n");

    ret = SGFPM_OpenDevice(sgfplib, 0);
    if (ret != SGFDX_ERROR_NONE) {
        printf("Error en SGFPM_OpenDevice: %lu\n", ret);
        return -1;
    }
    printf("SGFPM_OpenDevice OK\n");

    printf("Encendiendo LED...\n");
    SGFPM_SetLedOn(sgfplib, 1);
    sleep(2); // Tiempo para colocar el dedo

    unsigned char imgBuffer[400 * 400]; // Captura de imagen
    unsigned char tplBuffer[378]; // Template final
    SGFingerInfo fingerInfo = { SG_FINGPOS_UK, 100, SG_IMPTYPE_LP, 1 };

    printf("Capturando imagen...\n");
    DWORD timeout = 10000; // 10 segundos en milisegundos
    SGFPM_SetAutoOnIRLedTouchOn(sgfplib, timeout, 1);
    ret = SGFPM_GetImage(sgfplib, imgBuffer);
    SGFPM_SetLedOn(sgfplib, 0);
    if (ret != SGFDX_ERROR_NONE) {
        printf("Error en SGFPM_GetImage: %lu\n", ret);
        return -1;
    }
    printf("SGFPM_GetImage OK\n");

    printf("Creando template...\n");
    ret = SGFPM_CreateTemplate(sgfplib, &fingerInfo, imgBuffer, tplBuffer);
    if (ret != SGFDX_ERROR_NONE) {
        printf("Error en SGFPM_CreateTemplate: %lu\n", ret);
        return -1;
    }
    printf("SGFPM_CreateTemplate OK\n");

    printf("Enviando template a stdout...\n");
    fwrite(tplBuffer, 1, sizeof(tplBuffer), stdout); // ⚠️ SOLO salida binaria
    printf("Cerrando dispositivo...\n");
    SGFPM_CloseDevice(sgfplib);
    SGFPM_Terminate(sgfplib);
    printf("Finalizado correctamente\n");
    return 0;
}
