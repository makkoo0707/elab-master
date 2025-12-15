
/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include <stdlib.h>
#include "eio_pin.h"
#include "elab.h"

#ifdef __cplusplus
extern "C" {
#endif

ELAB_TAG("APP_LED");

/* private config ----------------------------------------------------------- */
#define LED_POLL_PERIOD_MS                  (100)

/* private variables -------------------------------------------------------- */
eio_object_t *led = NULL;

/* includes ----------------------------------------------------------------- */
/**
  * @brief  LED initialization.
  * @retval None
  */
void led_init(void)
{
    led = eio_find("LED1");
    elab_assert(led != NULL);
    eio_pin_set_mode(led, PIN_MODE_OUTPUT_PP);
}
INIT_EXPORT(led_init, 2);

/**
  * @brief  LED polling function.
  * @retval None
  */
void led_poll(void)
{
    if (led == NULL)
    {
        return;
    }
    if ((elab_time_ms() % 1000) < 500)
    {
        eio_pin_set_status(led, true);
    }
    else
    {
        eio_pin_set_status(led, false);
    }
}
POLL_EXPORT(led_poll, LED_POLL_PERIOD_MS);

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
