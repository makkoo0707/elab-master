/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include <string.h>
#include "eio_object.h"
#include "elab_assert.h"

ELAB_TAG("EIO_OBJECT");

/* private variables -------------------------------------------------------- */
static eio_object_t *eio_list = NULL;

/* public functions --------------------------------------------------------- */
/**
  * @brief  EIO registering function.
  * @param  me      this pointer
  * @param  name    EIO object's name.
  * @param  ops     EIO object's level operation interface.
  * @param  ops     EIO object attribute.
  * @retval None
  */
void eio_register(eio_object_t * const me, const char *name,
                    const eio_ops_t *ops,
                    eio_obj_attr_t *attr)
{
    elab_assert(me != NULL);
    elab_assert(name != NULL);
    elab_assert(ops != NULL);
    elab_assert(attr != NULL);

    me->name = name;
    me->ops = ops;
    
    me->next = eio_list;
    eio_list = me;

    memcpy(&me->attr, attr, sizeof(eio_obj_attr_t));
}

/**
  * @brief  EIO object handle getting function.
  * @param  name    EIO object's name.
  * @retval EIO object handle.
  */
eio_object_t *eio_find(const char *name)
{
    elab_assert(name != NULL);
    
    eio_object_t *obj = eio_list;
    while (obj != NULL)
    {
        if (strcmp(obj->name, name) == 0)
        {
            break;
        }
        obj = obj->next;
    }

    return obj;
}

/**
  * @brief  EIO object handle opening function.
  * @param  me      EIO object handle.
  * @retval See elab_err_t.
  */
elab_err_t eio_open(eio_object_t * const me)
{
    elab_assert(me->ops->open != NULL);

    elab_err_t ret = ELAB_OK;

    if (me->attr.standlone)
    {
        if (me->count_open != 0)
        {
            ret = ELAB_ERROR;
            goto exit;
        }
    }

    ret = me->ops->open(me);
    if (ret == ELAB_OK)
    {
        me->count_open ++;
    }

exit:
    return ret;
}

/**
  * @brief  EIO object handle closing function.
  * @param  me      EIO object handle.
  * @retval See elab_err_t.
  */
elab_err_t eio_close(eio_object_t * const me)
{
    elab_assert(me->ops->close != NULL);

    elab_err_t ret = ELAB_OK;

    if (me->count_open > 0)
    {
        ret = me->ops->close(me);
        if (ret == ELAB_OK)
        {
            me->count_open --;
            goto exit;
        }
    }
    else
    {
        ret = ELAB_ERROR;
    }

exit:
    return ret;
}

/**
  * @brief  EIO object handle reading function.
  * @param  me      EIO object handle.
  * @param  buffer  Reading buffer.
  * @param  size    The buffer size.
  * @retval If >= 0, the actual data bytes number; if < 0, see elab_err_t.
  */
int32_t eio_read(eio_object_t * const me, void *buffer, uint32_t size)
{
    elab_assert(me != NULL);
    elab_assert(buffer != NULL);
    elab_assert(size != 0);
    elab_assert(me->ops->read != NULL);

    int32_t ret = 0;

    if (me->count_open != 0)
    {
        ret = me->ops->read(me, buffer, size);
    }
    else
    {
        ret = (int32_t)ELAB_ERROR;
    }
    
    return ret;
}

/**
  * @brief  EIO object handle writting function.
  * @param  me      EIO object handle.
  * @param  buffer  Writting buffer.
  * @param  size    The buffer size.
  * @retval If >= 0, the actual data bytes number; if < 0, see elab_err_t.
  */
int32_t eio_write(eio_object_t * const me, const void *buffer, uint32_t size)
{
    elab_assert(me != NULL);
    elab_assert(buffer != NULL);
    elab_assert(size != 0);
    elab_assert(me->ops->write != NULL);

    int32_t ret = 0;

    if (me->count_open != 0)
    {
        ret = me->ops->write(me, buffer, size);
    }
    else
    {
        ret = (int32_t)ELAB_ERROR;
    }
    
    return ret;
}

/* ----------------------------- end of file -------------------------------- */
