
/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "elab/3rd/Shell/shell.h"
#include "elab/common/elab_log.h"
#include "elab/edf/normal/elab_pin.h"
#include "elab/edf/driver/simulator/simu_pin.h"

ELAB_TAG("GpioTest");

#ifdef __cplusplus
extern "C" {
#endif

/* private functions -------------------------------------------------------- */
/**
  * @brief Testing function for base vw function.
  * @retval None
  */
static int32_t test_gpio_read(int32_t argc, char *argv[])
{
    (void)argc;
    (void)argv;

    bool status;
    int32_t ret = 0;
    elab_device_t *pin = NULL;

    if (argc != 2)
    {
        elog_error("test_gpio_read.");
        ret = -1;
        goto exit;
    }

    pin = elab_device_find(argv[1]);
    if (pin == NULL)
    {
        elog_error("spi device not found.");
        ret = -2;
        goto exit;
    }

    status = elab_pin_get_status(pin);
    printf("GPIO %s status %d.\n", argv[1], status);

exit:
    return ret;
}

/**
  * @brief Testing function for base vw function.
  * @retval None
  */
static int32_t test_gpio_write(int32_t argc, char *argv[])
{
    (void)argc;
    (void)argv;

    bool status;
    int32_t ret = 0;
    elab_device_t *pin = NULL;

    if (argc != 3)
    {
        elog_error("test_gpio_read.");
        ret = -1;
        goto exit;
    }

    pin = elab_device_find(argv[1]);
    if (pin == NULL)
    {
        elog_error("spi device not found.");
        ret = -2;
        goto exit;
    }

    status = argv[2][0] == '0' ? false : true;

    elab_pin_set_status(pin, status);
    printf("GPIO %s status %d.\n", argv[1], status);

exit:
    return ret;
}

/**
  * @brief Testing function for base vw function.
  * @retval None
  */
static int32_t test_simu_gpio_write(int32_t argc, char *argv[])
{
    (void)argc;
    (void)argv;

    bool status;
    int32_t ret = 0;

    if (argc != 3)
    {
        elog_error("test_gpio_read.");
        ret = -1;
        goto exit;
    }

    status = argv[2][0] == '0' ? false : true;

    simu_in_set_status(argv[1], status);

exit:
    return ret;
}

/**
  * @brief  Test shell command export
  */
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                    test_gpio_read,
                    test_gpio_read,
                    GPIO reading testing function);

/**
  * @brief  Test shell command export
  */
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                    test_gpio_write,
                    test_gpio_write,
                    GPIO writing testing function);

/**
  * @brief  Test shell command export
  */
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                    test_simu_gpio_write,
                    test_simu_gpio_write,
                    GPIO writing testing function);

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
