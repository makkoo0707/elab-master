
/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include "../driver/drv_adc.h"
#include "elab/common/elab_export.h"
#include "export.h"

/* private variables -------------------------------------------------------- */
static elab_adc_driver_t adc_mcu_pwm_a00;
static elab_adc_driver_t adc_mcu_button_a01;

/* public functions --------------------------------------------------------- */
static void driver_adc_export(void)
{
    /* PIN devices on MCU. */
    elab_driver_adc_init(&adc_mcu_pwm_a00, "adc_pwm", "A.00");
    elab_driver_adc_init(&adc_mcu_button_a01, "adc_button", "A.01");
}
INIT_EXPORT(driver_adc_export, EXPORT_LEVEL_ADC);

/* ----------------------------- end of file -------------------------------- */
