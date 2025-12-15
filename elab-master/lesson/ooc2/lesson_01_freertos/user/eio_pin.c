
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
static eio_ops_t _obj_ops =
{
    .open = NULL,
    .close = NULL,
    .read = NULL,
    .write = NULL,
};

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

    eio_obj_attr_t attr =
    {
        .user_data = user_data,
        .standlone = true,
        .type = EIO_OBJ_PIN,
    };

    eio_register(&me->super, name, &_obj_ops, &attr);

    me->ops = ops;
    me->ops->init(me);
    me->status = me->ops->get_status(me);
}

/**
  * @brief  EIO pin's status getting function.
  * @param  me      this pointer
  * @retval The pin's status.
  */
void eio_pin_set_mode(eio_object_t * const me, uint8_t mode)
{
    elab_assert(me != NULL);

    eio_pin_t *pin = (eio_pin_t *)me;
    if (pin->mode != mode)
    {
        pin->ops->set_mode(pin, mode);
        pin->mode = mode;
    }
}


/**
  * @brief  EIO pin's status getting function.
  * @param  me      this pointer
  * @retval The pin's status.
  */
bool eio_pin_get_status(eio_object_t * const me)
{
    elab_assert(me != NULL);
    eio_pin_t *pin = (eio_pin_t *)me;

    pin->status = pin->ops->get_status(pin);

    return pin->status;
}

/**
  * @brief  EIO pin's status turning on function.
  * @param  me      this pointer
  * @param  status  the input pin status.
  * @retval None.
  */
void eio_pin_set_status(eio_object_t * const me, bool status)
{
    elab_assert(me != NULL);

    eio_pin_t *pin = (eio_pin_t *)me;
    elab_assert(pin->mode == PIN_MODE_OUTPUT_PP ||
                pin->mode == PIN_MODE_OUTPUT_OD);
    
    if (status != pin->status)
    {
        pin->ops->set_status(pin, status);
        eio_pin_get_status(me);
        elab_assert(pin->status == status);
    }
}

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
