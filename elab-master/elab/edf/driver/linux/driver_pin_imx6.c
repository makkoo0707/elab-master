
/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* include ------------------------------------------------------------------ */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <string.h>
#include "driver_pin_imx6.h"
#include "elab/edf/normal/elab_pin.h"
#include "elab/common/elab_assert.h"

ELAB_TAG("PinDriverImx6");

/* private define ----------------------------------------------------------- */
#define EXPORT_PATH                         "/sys/class/gpio/export"

/* private function prototype ----------------------------------------------- */
static elab_err_t _init(elab_pin_t * const me);
static elab_err_t _set_mode(elab_pin_t * const me, uint8_t mode);
static elab_err_t _set_status(elab_pin_t * const me, bool status);
static elab_err_t _get_status(elab_pin_t * const me, bool *status);

static int32_t _pin_get(const char *name);
static uint32_t _pin_get_type(const char *name);

/* private variables -------------------------------------------------------- */
static const elab_pin_ops_t _pin_ops =
{
    .init = _init,
    .set_mode = _set_mode,
    .get_status = _get_status,
    .set_status = _set_status,
};

static int32_t fd_export = (int32_t)ELAB_ERR_INVALID;

/* public function ---------------------------------------------------------- */
/**
  * @brief  Pin device init exporting fucntion
  * @param  None
  * @retval None
  */
void drv_pin_init(driver_pin_imx6_t *driver,
                    const char *dev_name, const char *drv_name)
{
    if (fd_export == ELAB_ERR_INVALID)
    {
        fd_export = open(EXPORT_PATH, O_WRONLY);
        elab_assert(fd_export >= 0);
    }

    int32_t gpio_id = -1;
    int32_t ret;

    driver->id = (int32_t)ELAB_ERR_INVALID;
    driver->fd_device = (int32_t)ELAB_ERR_INVALID;
    driver->fd_direct = (int32_t)ELAB_ERR_INVALID;
    driver->status = false;
    driver->dev_name = dev_name;
    driver->drv_name = drv_name;

    /* Clear all temporary buffer. */
    memset(driver->device_path, 0, DRV_PIN_PATH_MAX);
    memset(driver->direct_path, 0, DRV_PIN_PATH_MAX);
    memset(driver->gpio_name, 0, 16);

    /* Get the ID of GPIO. */
    driver->id = _pin_get(driver->drv_name);
    gpio_id = driver->id;
    sprintf(driver->gpio_name, "%d", gpio_id);
    sprintf(driver->device_path, "/sys/class/gpio/gpio%d/value", gpio_id);
    sprintf(driver->direct_path, "/sys/class/gpio/gpio%d/direction", gpio_id);

    /* Export the GPIO. */
    /* Open GPIO exporting device. */
    write(fd_export, driver->gpio_name, strlen(driver->gpio_name));

    /* Write GPIO direction. */
    uint32_t type = _pin_get_type(driver->drv_name);
    elab_assert(type != PIN_MODE_MAX);
    char *str_direct = (type == PIN_MODE_OUTPUT_PP || type == PIN_MODE_OUTPUT_OD) ?
                        "out" : "in";
    driver->is_out = (type == PIN_MODE_OUTPUT_PP || type == PIN_MODE_OUTPUT_OD);
                        
    driver->fd_direct = open(driver->direct_path, O_RDWR);
    elab_assert(driver->fd_direct >= 0);
    ret = write(driver->fd_direct, str_direct, strlen(str_direct));
    elab_assert(ret >= 0);
    close(driver->fd_direct);

    /* Read GPIO direction. */
    char direction[4] = { 0 };
    driver->fd_direct = open(driver->direct_path, O_RDONLY);
    elab_assert(driver->fd_direct >= 0);
    ret = read(driver->fd_direct, direction, sizeof(direction));
    elab_assert(ret >= 0);
    close(driver->fd_direct);

    if (strncmp(direction, "in", 2) == 0)
    {
        elab_assert(!driver->is_out);

        char buffer[4];
        memset(buffer, 0, 4);
        driver->fd_device = open(driver->device_path, O_RDONLY);
        elab_assert(driver->fd_device >= 0);
        ret = read(driver->fd_device, buffer, sizeof(buffer));
        elab_assert(ret >= 0);
        elab_assert(strlen(buffer) == 2);
        elab_assert((buffer[0] == '0' || buffer[0] == '1') && buffer[1] == '\n');
        close(driver->fd_device);

        driver->status = buffer[0] == '0' ? false : true;
    }
    else if (strncmp(direction, "out", 3) == 0)
    {
        elab_assert(driver->is_out);
        driver->fd_device = open(driver->device_path, O_RDWR);
        elab_assert(driver->fd_device >= 0);
    }
    else
    {
        elab_assert(false);
    }

    elab_pin_register(&driver->device,
                        driver->dev_name,
                        &_pin_ops, (void *)driver);
    driver->device.mode =
        driver->is_out ? PIN_MODE_OUTPUT_OD : PIN_MODE_INPUT;

    printf("GPIO %s (id %d) register success.\n",
                                    driver->dev_name,
                                    driver->id);

    close(fd_export);
}

/* private function --------------------------------------------------------- */
static elab_err_t _init(elab_pin_t * const me)
{
    (void)me;

    return ELAB_OK;
}

static elab_err_t _set_mode(elab_pin_t * const me, uint8_t mode)
{
    (void)me;
    (void)mode;

    return ELAB_OK;
}

static elab_err_t _set_status(elab_pin_t * const me, bool status)
{
    driver_pin_imx6_t *data = me->super.user_data;

    const char *str_value = status ? "1" : "0";
    data->fd_device = open(data->device_path, O_RDWR);
    elab_assert(data->fd_device >= 0);
    int32_t ret = write(data->fd_device, str_value, strlen(str_value));
    elab_assert(ret >= 0);
    close(data->fd_device);

    data->status = status;

    return ELAB_OK;
}

static elab_err_t _get_status(elab_pin_t * const me, bool *status_out)
{
    driver_pin_imx6_t *data = me->super.user_data;
    bool ret = false;

    if (me->mode == PIN_MODE_OUTPUT_PP || me->mode == PIN_MODE_OUTPUT_OD)
    {
        ret = data->status;
    }
    else
    {
        char buffer[10];
        memset(buffer, 0, 10);
        data->fd_device = open(data->device_path, O_RDONLY);
        elab_assert(data->fd_device >= 0);
        int32_t ret_read = read(data->fd_device, buffer, sizeof(buffer));
        elab_assert(ret_read >= 0);
        close(data->fd_device);

        *status_out = (buffer[0] == '0' ? false : true);
    }

    return ELAB_OK;
}

static uint32_t _pin_get_type(const char *name)
{
    uint32_t ret_type = PIN_MODE_MAX;

    static const char *str_pin_type[] =
    {
        "INP", "IPU", "IPD", "OPP", "OOD"
    };
    for (uint32_t i = 0; i < sizeof(str_pin_type) / sizeof(char *); i ++)
    {
        if (strncmp(str_pin_type[i], name, 3) == 0)
        {
            ret_type = i;
            break;
        }
    }

    return ret_type;
}

static bool _pin_name_valid(const char *name)
{
    bool ret = true;
    uint8_t len = strlen(name);

    if (len != strlen("INP.5.12"))
    {
        ret = false;
        goto exit;
    }

    if (strncmp(name, "INP", 3) == 0 ||
        strncmp(name, "IPU", 3) == 0 ||
        strncmp(name, "IPD", 3) == 0 ||
        strncmp(name, "OPP", 3) == 0 ||
        strncmp(name, "OOD", 3) == 0)
    {
        ret = false;
        goto exit;
    }

    if (name[3] != '.' || name[5] != '.' ||
        !(name[4] >= '0' && name[4] <= '9') ||
        !(name[len - 1] >= '0' && name[len - 1] <= '9') ||
        !(name[len - 2] >= '0' && name[len - 2] <= '9'))
    {
        ret = false;
        goto exit;
    }

exit:
    if (!ret)
    {
        elog_error("PIN name %s is invalid.");
    }
    return ret;
}

static int32_t _pin_get(const char *name)
{
    /* For example, P3.4 P2.18 */
    elab_assert(_pin_name_valid(name));

    int32_t ret = (int32_t)ELAB_ERR_INVALID;
    uint8_t len = strlen(name);
    ret = (name[1] - '0' - 1) * 32 + (name[3] - '0') * 10 + (name[4] - '0');

    return ret;
}

/* ----------------------------- end of file -------------------------------- */
