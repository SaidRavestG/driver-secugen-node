#include <stdio.h>
#include <wchar.h>         // <- indispensable para wchar_t
#include "sgfplib.h"

// Usamos el manejador correcto proporcionado por SecuGen (HSGFPM)
HSGFPM sgfplib;

int sg_open_device() {
    DWORD ret;

    ret = SGFPM_Create(&sgfplib);
    if (ret != SGFDX_ERROR_NONE) return -1;

    ret = SGFPM_Init(sgfplib, SG_DEV_AUTO);
    if (ret != SGFDX_ERROR_NONE) return -2;

    ret = SGFPM_OpenDevice(sgfplib, 0);
    if (ret != SGFDX_ERROR_NONE) return -3;

    return 0;
}

int sg_set_led(int state) {
    DWORD ret = SGFPM_SetLedOn(sgfplib, state);
    return (ret == SGFDX_ERROR_NONE) ? 0 : -1;
}

void sg_close_device() {
    SGFPM_CloseDevice(sgfplib);
    SGFPM_Terminate(sgfplib);
}
