
/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include "../driver/drv_button.h"
#include "elab/common/elab_export.h"
#include "export.h"

/* private variables -------------------------------------------------------- */
static elab_button_driver_t button_pin_start;
static elab_button_driver_t button_pin_stop;
static elab_button_driver_t button_adc_right;
static elab_button_driver_t button_adc_left;
static elab_button_driver_t button_adc_up;
static elab_button_driver_t button_adc_down;

/* public functions --------------------------------------------------------- */
static void driver_button_export(void)
{
    /* Buttons based on PIN devices. */
    elab_driver_button_pin_init(&button_pin_start,
                                "button_start", "pin_button_start", true);
    elab_driver_button_pin_init(&button_pin_stop,
                                "button_stop", "pin_button_stop", false);

    /* Buttons based on ADC devices. */
    elab_driver_button_adc_init(&button_adc_right,
                                "button_right", "adc_button",
                                1.500, 1.65);
    elab_driver_button_adc_init(&button_adc_left,
                                "button_left", "adc_button",
                                1.00, 1.495);
    elab_driver_button_adc_init(&button_adc_up,
                                "button_up", "adc_button",
                                0.0, 0.2);
    elab_driver_button_adc_init(&button_adc_down,
                                "button_down", "adc_button",
                                1.66, 1.88);
}
INIT_EXPORT(driver_button_export, EXPORT_LEVEL_BUTTON);

/* ----------------------------- end of file -------------------------------- */
