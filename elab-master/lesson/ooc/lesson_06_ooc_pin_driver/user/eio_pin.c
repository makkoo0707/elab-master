
/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include <string.h>
#include "eio_pin.h"
#include "elab_assert.h"

#ifdef __cplusplus
extern "C" {
#endif

ELAB_TAG("EIO_PIN");

/* private variables -------------------------------------------------------- */
static eio_pin_t *pin_list = NULL;

/* public functions --------------------------------------------------------- */
/**
  * @brief  EIO pin initialization.
  * @param  me      this pointer
  * @param  name    pin's name.
  * @param  mode    pin's mode.
  * @retval None
  */
void eio_pin_register(eio_pin_t * const me,
                        const char *name,
                        const eio_pin_ops_t *ops,
                        void *user_data)
{
    elab_assert(me != NULL);
    elab_assert(name != NULL);
    elab_assert(ops != NULL);

    me->name = name;
    me->ops = ops;
    me->user_data = user_data;
    me->next = pin_list;
    pin_list = me;

    me->ops->init(me);
    me->status = me->ops->get_status(me);
}

/**
  * @brief  Find the device by its name.
  * @param  name    pin's name.
  * @retval PIN device handle
  */
eio_pin_t *eio_pin_find(const char *name)
{
    elab_assert(name != NULL);
    elab_assert(pin_list != NULL);
    
    eio_pin_t *pin = pin_list;
    while (pin != NULL)
    {
        if (strcmp(pin->name, name) == 0)
        {
            break;
        }
        pin = pin->next;
    }

    return pin;
}

/**
  * @brief  EIO pin's status getting function.
  * @param  me      this pointer
  * @retval The pin's status.
  */
bool eio_pin_get_status(eio_pin_t * const me)
{
    elab_assert(me != NULL);

    me->status = me->ops->get_status(me);

    return me->status;
}

/**
  * @brief  EIO pin's status turning on function.
  * @param  me      this pointer
  * @param  status  the input pin status.
  * @retval None.
  */
void eio_pin_set_status(eio_pin_t * const me, bool status)
{
    elab_assert(me != NULL);
    elab_assert(me->mode == PIN_MODE_OUTPUT_PP ||
                me->mode == PIN_MODE_OUTPUT_OD);
    
    if (status != me->status)
    {
        me->ops->set_status(me, status);
        eio_pin_get_status(me);
        elab_assert(me->status == status);
    }
}


void eio_pin_set_mode(eio_pin_t * const me, uint8_t mode)
{
    elab_assert(me != NULL);
    
    me->mode = mode;
    me->ops->set_mode(me, mode);
}

/* ----------------------------- end of file -------------------------------- */
