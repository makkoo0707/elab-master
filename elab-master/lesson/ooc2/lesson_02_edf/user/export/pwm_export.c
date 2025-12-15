
/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include "../driver/drv_pwm.h"
#include "elab/common/elab_export.h"
#include "export.h"

/* public functions --------------------------------------------------------- */
static void driver_pwm_mcu_export(void)
{
    /* PWM devices on MCU. */
#if 0
    static elab_pwm_driver_t pwm_mcu_led1_c08;
    static elab_pwm_driver_t pwm_mcu_led2_c09;
    elab_driver_pwm_init(&pwm_mcu_led1_c08, "pwm_led1", "C.08");
    elab_driver_pwm_init(&pwm_mcu_led2_c09, "pwm_led2", "C.09");
#endif

    static elab_pwm_driver_t pwm_mcu_dac_c01;
    elab_driver_pwm_init(&pwm_mcu_dac_c01, "pwm_dac", "C.01");
}
INIT_EXPORT(driver_pwm_mcu_export, EXPORT_LEVEL_PWM_MCU);

/* ----------------------------- end of file -------------------------------- */
