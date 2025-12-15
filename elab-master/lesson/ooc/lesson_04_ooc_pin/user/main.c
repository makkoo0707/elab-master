/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* Includes ------------------------------------------------------------------*/
#include "bsp.h"
#include "eio_pin.h"
#include "elab_common.h"

/* private variables -------------------------------------------------------- */
eio_pin_t pin_led;

/* public functions --------------------------------------------------------- */
/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    bsp_init();

    eio_pin_init(&pin_led, "C.08", PIN_MODE_OUTPUT);
    
    while (1)
    {
        if ((elab_time_ms() % 1000) < 500)
        {
            eio_pin_set_status(&pin_led, true);
        }
        else
        {
            eio_pin_set_status(&pin_led, false);
        }
    }
}

/* ----------------------------- end of file -------------------------------- */
