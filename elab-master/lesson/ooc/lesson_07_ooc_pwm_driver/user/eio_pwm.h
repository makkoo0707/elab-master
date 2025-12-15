/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef EIO_PWM_H
#define EIO_PWM_H

/* includes ----------------------------------------------------------------- */
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* public typedef ----------------------------------------------------------- */
typedef struct eio_pwm
{
    struct eio_pwm *next;
    const struct eio_pwm_ops *ops;
    void *user_data;

    const char *name;
    uint8_t duty_ratio;
} eio_pwm_t;

typedef struct eio_pwm_ops
{
    void (* init)(eio_pwm_t * const me);
    void (* set_duty)(eio_pwm_t * const me, uint8_t duty_ratio);
} eio_pwm_ops_t;

/* public functions --------------------------------------------------------- */
/* For low-level driver. */
void eio_pwm_register(eio_pwm_t * const me,
                        const char *name,
                        const eio_pwm_ops_t *ops,
                        void *user_data);

/* For high-level code. */
eio_pwm_t *eio_pwm_find(const char *name);
void eio_pwm_set_duty(eio_pwm_t * const me, uint8_t duty_ratio);

#ifdef __cplusplus
}
#endif

#endif  /* EIO_PWM_H */

/* ----------------------------- end of file -------------------------------- */
