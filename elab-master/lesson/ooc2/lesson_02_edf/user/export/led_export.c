
/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include "elab/common/elab_export.h"
#include "elab/edf/user/elab_led.h"
#include "export.h"

/* private variables -------------------------------------------------------- */
static elab_led_t led1;
static elab_led_t led2;
static elab_led_t led3;
static elab_led_t led4;

/* public functions --------------------------------------------------------- */
static void led_export(void)
{
    /* PIN devices on MCU. */
    elab_led_register(&led1, "led1", "pin_led1", true);
    elab_led_register(&led2, "led2", "pin_led2", true);
    elab_led_register(&led3, "led3", "pin_led3", true);
    elab_led_register(&led4, "led4", "pin_led4", true);
}
INIT_EXPORT(led_export, EXPORT_LEVEL_LED);

/* ----------------------------- end of file -------------------------------- */
