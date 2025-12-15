
/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include <string.h>
#include "eio_pwm.h"
#include "elab_assert.h"

#ifdef __cplusplus
extern "C" {
#endif

ELAB_TAG("EIO_PWM");

/* private variables -------------------------------------------------------- */
static eio_pwm_t *pwm_list = NULL;

/* public functions --------------------------------------------------------- */
/**
  * @brief  EIO pwm initialization.
  * @param  me      this pointer
  * @param  name    pwm's name.
  * @param  mode    pwm's mode.
  * @retval None
  */
void eio_pwm_register(eio_pwm_t * const me,
                        const char *name,
                        const eio_pwm_ops_t *ops,
                        void *user_data)
{
    elab_assert(me != NULL);
    elab_assert(name != NULL);
    elab_assert(ops != NULL);

    me->name = name;
    me->ops = ops;
    me->user_data = user_data;
    me->next = pwm_list;
    pwm_list = me;

    me->duty_ratio = 0;
    me->ops->init(me);
}

/**
  * @brief  Find the device by its name.
  * @param  name    pwm's name.
  * @retval PWM device handle
  */
eio_pwm_t *eio_pwm_find(const char *name)
{
    elab_assert(name != NULL);
    elab_assert(pwm_list != NULL);
    
    eio_pwm_t *pwm = pwm_list;
    while (pwm != NULL)
    {
        if (strcmp(pwm->name, name) == 0)
        {
            break;
        }
        pwm = pwm->next;
    }

    return pwm;
}

/**
  * @brief  EIO pwm's status turning on function.
  * @param  me      this pointer
  * @param  duty    the input pwm status.
  * @retval None.
  */
void eio_pwm_set_duty(eio_pwm_t * const me, uint8_t duty_ratio)
{
    elab_assert(me != NULL);
    elab_assert(duty_ratio <= 100);
    
    if (duty_ratio != me->duty_ratio)
    {
        me->ops->set_duty(me, duty_ratio);
        me->duty_ratio = duty_ratio;
    }
}

/* ----------------------------- end of file -------------------------------- */
