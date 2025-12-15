#ifndef SIMU_MBS_H
#define SIMU_MBS_H

/* include ------------------------------------------------------------------ */
#include "elab/edf/user/elab_motor.h"
#include "elab/midware/modbus/modbus.h"

/* public typedef ----------------------------------------------------------- */
typedef struct driver_simu_mbs
{
    mb_channel_t *ch;
    char dev_name[ELAB_NAME_SIZE];
    char drv_name[ELAB_NAME_SIZE];
} driver_simu_mbs_t;

/* public functions --------------------------------------------------------- */
/* Driver name format: simu_mbs.serial_name */
/* Driver name example: simu_mbs.simu_rs485_2_1 */
void drv_simu_mbs_init(driver_simu_mbs_t *simu_mbs,
                        const char *dev_name, const char *drv_name);

#endif

/* ----------------------------- end of file -------------------------------- */
