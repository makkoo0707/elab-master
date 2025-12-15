/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef DRV_PWM_H
#define DRV_PWM_H

/* includes ----------------------------------------------------------------- */
#include "elab/edf/normal/elab_pwm.h"

#ifdef __cplusplus
extern "C" {
#endif

/* public typedef ----------------------------------------------------------- */
typedef struct elab_pwn_driver
{
    elab_pwm_t device;
    const char *pin_name;
} elab_pwm_driver_t;

/* public functions --------------------------------------------------------- */
/* For example, the pin name should be like "A.02". */
void elab_driver_pwm_init(elab_pwm_driver_t *me,
                            const char *name, const char *pin_name);

#ifdef __cplusplus
}
#endif

#endif  /* DRV_PWM_H */

/* ----------------------------- end of file -------------------------------- */
