#include <stdio.h>
#include "sgfplib.h"

int main() {
    printf("✅ Iniciando prueba del SDK SecuGen...\n");

    HSGFPM sgfplib;
    DWORD ret = SGFPM_Create(&sgfplib);
    if (ret != SGFDX_ERROR_NONE) {
        fprintf(stderr, "❌ Error creando SGFPM: %lu\n", ret);
        return -1;
    }

    ret = SGFPM_Init(sgfplib, SG_DEV_AUTO);
    if (ret != SGFDX_ERROR_NONE) {
        fprintf(stderr, "❌ Error inicializando SGFPM: %lu\n", ret);
        return -1;
    }

    printf("🔍 Intentando abrir el lector de huellas...\n");
    ret = SGFPM_OpenDevice(sgfplib, 0);  // Intentar abrir el primer dispositivo
    if (ret != SGFDX_ERROR_NONE) {
        fprintf(stderr, "❌ No se pudo abrir el lector de huellas. Código: %lu\n", ret);
        return -1;
    }

    printf("📌 Dispositivo abierto con éxito!\n");

    SGDeviceInfoParam deviceInfo;
    ret = SGFPM_GetDeviceInfo(sgfplib, &deviceInfo);
    if (ret != SGFDX_ERROR_NONE) {
        fprintf(stderr, "❌ Error obteniendo información del dispositivo: %lu\n", ret);
        return -1;
    }

    printf("📋 Dispositivo detectado: Modelo %ld, Serial: %s\n", (long)deviceInfo.DeviceID, deviceInfo.DeviceSN);

    SGFPM_Terminate(sgfplib);
    return 0;
}
