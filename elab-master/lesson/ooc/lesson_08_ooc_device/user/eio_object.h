/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef EIO_OBJECT_H
#define EIO_OBJECT_H

/* includes ----------------------------------------------------------------- */
#include <stdbool.h>
#include <stdint.h>
#include "elab_def.h"

#ifdef __cplusplus
extern "C" {
#endif

/* public define ------------------------------------------------------------ */
enum eio_obect_type
{
    EIO_OBJ_PIN = 0,
    EIO_OBJ_PWM,
    EIO_OBJ_ADC,
    EIO_OBJ_DAC,
    EIO_OBJ_UART,
    EIO_OBJ_SPI,
    EIO_OBJ_I2C,
    EIO_OBJ_CAN,

    EIO_OBJ_MAX
};

/* public typedef ----------------------------------------------------------- */
typedef struct eio_obj_attr
{
    void *user_data;
    uint8_t type;
    bool standlone;
} eio_obj_attr_t;

typedef struct eio_object
{
    struct eio_object *next;
    const char *name;
    const struct eio_ops *ops;
    uint16_t count_open;
    eio_obj_attr_t attr;
} eio_object_t;

typedef struct eio_ops
{
    elab_err_t (* open)(eio_object_t * const me);
    elab_err_t (* close)(eio_object_t * const me);
    int32_t (* read)(eio_object_t * const me, void *buffer, uint32_t size);
    int32_t (* write)(eio_object_t * const me, const void *buffer, uint32_t size);
} eio_ops_t;

/* public functions --------------------------------------------------------- */
/* For io-level driver. */
void eio_register(eio_object_t * const me, const char *name,
                    const eio_ops_t *ops,
                    eio_obj_attr_t *attr);

/* For high-level code. */
eio_object_t *eio_find(const char *name);
elab_err_t eio_open(eio_object_t * const me);
elab_err_t eio_close(eio_object_t * const me);
int32_t eio_read(eio_object_t * const me, void *buffer, uint32_t size);
int32_t eio_write(eio_object_t * const me, const void *buffer, uint32_t size);

#ifdef __cplusplus
}
#endif

#endif  /* EIO_OBJECT_H */

/* ----------------------------- end of file -------------------------------- */
