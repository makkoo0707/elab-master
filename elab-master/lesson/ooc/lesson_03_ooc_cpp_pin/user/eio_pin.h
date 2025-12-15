/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef EIO_PIN_H
#define EIO_PIN_H

/* includes ----------------------------------------------------------------- */
#include "stm32g0xx_hal.h"

/* public define ------------------------------------------------------------ */
enum pin_mode
{
    PIN_MODE_INPUT = 0,
    PIN_MODE_INPUT_PULLUP,
    PIN_MODE_INPUT_PULLDOWN,
    PIN_MODE_OUTPUT,
    PIN_MODE_OUTPUT_OD,

    PIN_MODE_MAX
};

/* public typedef ----------------------------------------------------------- */
typedef struct eio_pin_data
{
    GPIO_TypeDef *gpio_x;
    uint16_t pin;
} eio_pin_data_t;

/* public class ------------------------------------------------------------- */
class eio_pin_t
{
public:
    void init(const char *name, enum pin_mode mode);
    bool get_status(void);
    void set_status(bool status);

private:
    bool status_;
    enum pin_mode mode_;
    eio_pin_data_t data_;
};

#endif

/* ----------------------------- end of file -------------------------------- */
