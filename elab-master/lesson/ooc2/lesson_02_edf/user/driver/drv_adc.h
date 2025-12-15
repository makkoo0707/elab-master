/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef DRV_ADC_H
#define DRV_ADC_H

/* includes ----------------------------------------------------------------- */
#include "elab/edf/normal/elab_adc.h"
#include "stm32g0xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/* public typedef ----------------------------------------------------------- */
typedef struct elab_adc_driver
{
    elab_adc_t device;
    const char *pin_name;
} elab_adc_driver_t;

/* public functions --------------------------------------------------------- */
/* For example, the pin name should be like "A.02". */
void elab_driver_adc_init(elab_adc_driver_t *me,
                            const char *name, const char *pin_name);

#ifdef __cplusplus
}
#endif

#endif  /* DRV_ADC_H */

/* ----------------------------- end of file -------------------------------- */
