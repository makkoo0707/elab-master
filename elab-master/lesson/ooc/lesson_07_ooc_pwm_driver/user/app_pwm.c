
/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include <stdlib.h>
#include "eio_pwm.h"
#include "elab.h"

#ifdef __cplusplus
extern "C" {
#endif

ELAB_TAG("APP_PWM");

/* private config ----------------------------------------------------------- */
#define LED_POLL_PERIOD_MS                  (5)

/* private variables -------------------------------------------------------- */
eio_pwm_t *led3 = NULL;
eio_pwm_t *led4 = NULL;

/* includes ----------------------------------------------------------------- */
/**
  * @brief  LED initialization.
  * @retval None
  */
void led_init(void)
{
    led3 = eio_pwm_find("pwmled3");
    elab_assert(led3 != NULL);
    
    led4 = eio_pwm_find("pwmled4");
    elab_assert(led4 != NULL);
}
INIT_EXPORT(led_init, 2);

/**
  * @brief  LED polling function.
  * @retval None
  */
void led_poll(void)
{
    uint16_t ms = elab_time_ms() % 1000;
    uint8_t duty_ratio;
    if (ms <= 500)
    {
        duty_ratio = ms / 5;
    }
    else
    {
        duty_ratio = (1000 - ms) / 5;
    }

    eio_pwm_set_duty(led3, duty_ratio);
    eio_pwm_set_duty(led4, duty_ratio);
}
POLL_EXPORT(led_poll, LED_POLL_PERIOD_MS);

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
