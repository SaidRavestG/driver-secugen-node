#include <stdio.h>
#include <unistd.h>
#include "sgfplib.h"

int main() {
    HSGFPM sgfplib;
    DWORD ret = SGFPM_Create(&sgfplib);
    if (ret != SGFDX_ERROR_NONE) return -1;

    ret = SGFPM_Init(sgfplib, SG_DEV_AUTO);
    if (ret != SGFDX_ERROR_NONE) return -1;

    ret = SGFPM_OpenDevice(sgfplib, 0);
    if (ret != SGFDX_ERROR_NONE) return -1;

    SGFPM_SetLedOn(sgfplib, 1);
    sleep(2); // Tiempo para colocar el dedo

    unsigned char imgBuffer[400 * 400]; // Captura de imagen
    unsigned char tplBuffer[378]; // Template final
    SGFingerInfo fingerInfo = { SG_FINGPOS_UK, 100, SG_IMPTYPE_LP, 1 };

    ret = SGFPM_GetImage(sgfplib, imgBuffer);
    SGFPM_SetLedOn(sgfplib, 0);
    if (ret != SGFDX_ERROR_NONE) return -1;

    ret = SGFPM_CreateTemplate(sgfplib, &fingerInfo, imgBuffer, tplBuffer);
    if (ret != SGFDX_ERROR_NONE) return -1;

    fwrite(tplBuffer, 1, sizeof(tplBuffer), stdout); // ⚠️ SOLO salida binaria
    SGFPM_CloseDevice(sgfplib);
    SGFPM_Terminate(sgfplib);
    return 0;
}
