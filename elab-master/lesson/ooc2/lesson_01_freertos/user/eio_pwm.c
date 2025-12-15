
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
static eio_ops_t _obj_ops =
{
    .open = NULL,
    .close = NULL,
    .read = NULL,
    .write = NULL,
};

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

    eio_obj_attr_t attr =
    {
        .user_data = user_data,
        .standlone = true,
        .type = EIO_OBJ_PWM,
    };

    eio_register(&me->super, name, &_obj_ops, &attr);

    me->ops = ops;
    me->duty_ratio = 0;
    me->ops->init(me);
}

/**
  * @brief  EIO pwm's status turning on function.
  * @param  me      this pointer
  * @param  duty    the input pwm status.
  * @retval None.
  */
void eio_pwm_set_duty(eio_object_t * const me, uint8_t duty_ratio)
{
    elab_assert(me != NULL);
    elab_assert(duty_ratio <= 100);
    elab_assert(me->attr.type == EIO_OBJ_PWM);
    
    eio_pwm_t *pwm = (eio_pwm_t *)me;
    if (duty_ratio != pwm->duty_ratio)
    {
        pwm->ops->set_duty(pwm, duty_ratio);
        pwm->duty_ratio = duty_ratio;
    }
}

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
