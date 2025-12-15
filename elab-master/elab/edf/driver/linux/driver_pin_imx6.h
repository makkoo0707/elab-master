
/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef DRIVER_PIN_IMX6_H
#define DRIVER_PIN_IMX6_H

/* include ------------------------------------------------------------------ */
#include "elab/edf/normal/elab_pin.h"

/* public define ------------------------------------------------------------ */
#define DRV_PIN_PATH_MAX                        (128)

/* public typedef ----------------------------------------------------------- */
typedef struct driver_pin_imx6
{
    elab_pin_t device;
    const char *dev_name;
    const char *drv_name;
    int32_t id;
    int32_t fd_device;
    int32_t fd_direct;
    bool status;
    bool is_out;
    char device_path[DRV_PIN_PATH_MAX];
    char direct_path[DRV_PIN_PATH_MAX];
    char gpio_name[16];
} driver_pin_imx6_t;

/* Driver name example: INP.5.12 */
/* Type INP, IPU, IPD, OPP, OOD */
void drv_pin_init(driver_pin_imx6_t *pin,
                    const char *dev_name, const char *drv_name);

#endif

/* ----------------------------- end of file -------------------------------- */
