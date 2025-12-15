
/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include <string.h>
#include <stdlib.h>
#include "drv_pin.h"
#include "drv_util.h"
#include "elab/edf/normal/elab_i2c.h"
#include "elab/common/elab_assert.h"
#include "elab/common/elab_def.h"
#include "elab/common/elab_export.h"
#include "stm32g0xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

ELAB_TAG("DriverPinI2C");

/* private function prototype ----------------------------------------------- */
static bool _check_pin_name_valid(const char *name);
static elab_device_t *_get_i2c_dev_from_name(const char *name);
static uint8_t _get_pin_id_from_name(const char *name);

static elab_err_t _init(elab_pin_t * const me);
static elab_err_t _set_mode(elab_pin_t * const me, uint8_t mode);
static elab_err_t _get_status(elab_pin_t * const me, bool *status);
static elab_err_t _set_status(elab_pin_t * const me, bool status);

/* private variables -------------------------------------------------------- */
static const elab_pin_ops_t pin_driver_ops =
{
    .init = _init,
    .set_mode = _set_mode,
    .get_status = _get_status,
    .set_status = _set_status,
};

/* public functions --------------------------------------------------------- */
void elab_driver_pin_i2c_init(elab_pin_driver_t *me,
                                const char *name, const char *pin_name)
{
    elab_assert(me != NULL);
    elab_assert(name != NULL);
    assert_name(_check_pin_name_valid(pin_name), pin_name);

    me->pin_name = pin_name;
    elab_pin_register(&me->device, name, &pin_driver_ops, me);
}

/* private functions -------------------------------------------------------- */
/**
  * @brief  The PIN driver initialization function.
  * @param  me  PIN device handle.
  * @retval None.
  */
static elab_err_t _init(elab_pin_t * const me)
{
    elab_assert(me != NULL);
    elab_pin_driver_t *driver = (elab_pin_driver_t *)me->super.user_data;

    uint8_t id = _get_pin_id_from_name(driver->pin_name);
    _set_mode(me, (id == 0 ? PIN_MODE_OUTPUT_OD : PIN_MODE_OUTPUT_PP));
    
    return ELAB_OK;
}

/**
  * @brief  The PIN driver set_mode function.
  * @param  me  PIN device handle.
  * @retval None.
  */
static elab_err_t _set_mode(elab_pin_t * const me, uint8_t mode)
{
    elab_assert(me != NULL);
    elab_assert(mode < PIN_MODE_MAX);

    elab_pin_driver_t *driver = (elab_pin_driver_t *)me->super.user_data;
    elab_device_t *i2c = _get_i2c_dev_from_name(driver->pin_name);
    uint8_t id = _get_pin_id_from_name(driver->pin_name);
    elab_assert(id < 8);
    elab_err_t ret = ELAB_OK;

    uint8_t config = 0;
    ret = elab_i2c_read_memory(i2c, 3, &config, 1, 100);
    elab_assert(ret >= 0);

    if (mode == PIN_MODE_INPUT)
    {
        config |= (1 << id);
    }
    else if (mode == PIN_MODE_INPUT_PULLUP)
    {
        /* The function is not supported by IC PCA9557. */
        elab_assert(false);
    }
    else if (mode == PIN_MODE_INPUT_PULLDOWN)
    {
        /* The function is not supported by IC PCA9557. */
        elab_assert(false);
    }
    else if (mode == PIN_MODE_OUTPUT_PP)
    {
        if (id == 0)
        {
            /* For only IO0, The function is not supported by IC PCA9557. */
            elab_assert(false);
        }
        config &=~ (1 << id);
    }
    else if (mode == PIN_MODE_OUTPUT_OD)
    {
        if (id != 0)
        {
            /* For only IO1-7, The function is not supported by IC PCA9557. */
            elab_assert(false);
        }
        config &=~ (1 << id);
    }
    
    ret = elab_i2c_write_memory(i2c, 3, &config, 1, 100);
    return ((ret >= 0) ? ELAB_OK : ret);
}

/**
  * @brief  The PIN driver set_mode function.
  * @param  me  PIN device handle.
  * @retval GPIO status.
  */
static elab_err_t _get_status(elab_pin_t * const me, bool *status)
{
    elab_assert(me != NULL);

    elab_pin_driver_t *driver = (elab_pin_driver_t *)me->super.user_data;
    elab_device_t *i2c = _get_i2c_dev_from_name(driver->pin_name);
    uint8_t id = _get_pin_id_from_name(driver->pin_name);

    uint8_t in_status = 0;
    elab_i2c_read_memory(i2c, 0, &in_status, 1, 100);

    *status = (in_status & (1 << id)) ? true : false;
    return ELAB_OK;
}

/**
  * @brief  The PIN driver set_status function.
  * @param  me      PIN device handle.
  * @param  status  GPIO status.
  * @retval None.
  */
static elab_err_t _set_status(elab_pin_t * const me, bool status)
{
    elab_assert(me != NULL);

    elab_pin_driver_t *driver = (elab_pin_driver_t *)me->super.user_data;
    elab_device_t *i2c = _get_i2c_dev_from_name(driver->pin_name);
    uint8_t id = _get_pin_id_from_name(driver->pin_name);
    
    uint8_t out_status;
    elab_i2c_read_memory(i2c, 1, &out_status, 1, 100);
    if (status)
    {
        out_status |= (1 << id);
    }
    else
    {
        out_status &=~ (1 << id);
    }
    elab_err_t ret = elab_i2c_write_memory(i2c, 1, &out_status, 1, 100);
    ret = ret >= ELAB_OK ? ELAB_OK : ret;
    
    return ret;
}

static bool _check_pin_name_valid(const char *name)
{
    bool valid = true;
    char i2c_name[32] = { 0 };
    strcpy(i2c_name, name);
    char *str_id = strstr(i2c_name, ".");
    str_id[0] = 0;
    str_id ++;
    int32_t id = 0;

    if (!elab_device_valid(i2c_name))
    {
        valid = false;
        goto exit;
    }

    id = atoi(str_id);
    if (id < 0 || id >= 8)
    {
        valid = false;
        goto exit;
    }
    
exit:
    return valid;
}

static elab_device_t *_get_i2c_dev_from_name(const char *name)
{
    char i2c_name[32] = { 0 };
    strcpy(i2c_name, name);
    char *_str = strstr(i2c_name, ".");
    _str[0] = 0;

    return elab_device_find(i2c_name);
}

static uint8_t _get_pin_id_from_name(const char *name)
{
    char i2c_name[32] = { 0 };
    strcpy(i2c_name, name);
    char *_str = strstr(i2c_name, ".");

    return (uint8_t)atoi(&_str[1]);
}

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
