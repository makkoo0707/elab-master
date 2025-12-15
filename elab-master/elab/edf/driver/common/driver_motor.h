#ifndef DRIVER_MOTOR_H
#define DRIVER_MOTOR_H

/* include ------------------------------------------------------------------ */
#include "elab/edf/user/elab_motor.h"
#include "elab/midware/modbus/modbus.h"

/* public typedef ----------------------------------------------------------- */
typedef struct driver_motor_zlac
{
    elab_motor_t device;
    char dev_name[ELAB_NAME_SIZE];
    char drv_name[ELAB_NAME_SIZE];
    uint8_t address;
    uint8_t id;
    bool ready;
    mb_channel_t *mb_ch;

    uint16_t cmd_speed_reg_addr;
    uint16_t act_speed_reg_addr;
    uint16_t error_reg_addr;
} driver_motor_zlac_t;

/* public functions --------------------------------------------------------- */
/* Driver name format: zlac_d.rs232(485)_1.address(two char).motor_id(one_char) */
/* Driver name example: zlac_d.rs485_1.02.1 */
void drv_motor_zlac_d_init(driver_motor_zlac_t *motor,
                            const char *dev_name, const char *drv_name);

#endif

/* ----------------------------- end of file -------------------------------- */
