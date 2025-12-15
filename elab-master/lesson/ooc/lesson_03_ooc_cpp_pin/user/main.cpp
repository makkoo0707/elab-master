/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* Includes ------------------------------------------------------------------*/
#include "bsp.h"
#include "eio_pin.h"
#include "elab_common.h"

/* private variables -------------------------------------------------------- */
static eio_pin_t pin_led;

/* public functions --------------------------------------------------------- */
/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    bsp_init();

    pin_led.init("C.08", PIN_MODE_OUTPUT);
    
    while (1)
    {
        if ((elab_time_ms() % 1000) < 500)
        {
            pin_led.set_status(true);
        }
        else
        {
            pin_led.set_status(false);
        }
    }
}

/* ----------------------------- end of file -------------------------------- */
