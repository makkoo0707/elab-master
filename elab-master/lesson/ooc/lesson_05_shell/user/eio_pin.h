/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef EIO_PIN_H
#define EIO_PIN_H

/* includes ----------------------------------------------------------------- */
#include <stdbool.h>
#include "stm32g0xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/* public define ------------------------------------------------------------ */
enum pin_mode
{
    PIN_MODE_INPUT = 0,
    PIN_MODE_INPUT_PULLUP,
    PIN_MODE_INPUT_PULLDOWN,
    PIN_MODE_OUTPUT_PP,
    PIN_MODE_OUTPUT_OD,

    PIN_MODE_MAX
};

/* public typedef ----------------------------------------------------------- */
typedef struct eio_pin_data
{
    GPIO_TypeDef *gpio_x;
    uint16_t pin;
} eio_pin_data_t;

typedef struct eio_pin
{
    eio_pin_data_t data;
    enum pin_mode mode;
    bool status;
} eio_pin_t;

/* public functions --------------------------------------------------------- */
void eio_pin_init(eio_pin_t * const me, const char *name, enum pin_mode mode);
bool eio_pin_get_status(eio_pin_t * const me);
void eio_pin_set_status(eio_pin_t * const me, bool status);

#ifdef __cplusplus
}
#endif

#endif

/* ----------------------------- end of file -------------------------------- */
