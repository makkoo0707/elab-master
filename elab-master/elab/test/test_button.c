/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include <string.h>
#include <stdio.h>
#include "../edf/user/elab_button.h"
#include "../3rd/Shell/shell.h"
#include "../common/elab_log.h"

ELAB_TAG("PinTest");

/* Exported functions --------------------------------------------------------*/
/**
  * @brief  Button test read
  * @param  argc - argument count
  * @param  argv - argument variant
  * @retval execute result
  */
static int test_button_get(int argc, char *argv[])
{
    int ret = 0;
    bool status = true;
    elab_device_t *dev = NULL;

    if (argc != 2)
    {
        elog_error("Not right argument number: %u. It should be 2.", argc);
        ret = -1;
        goto exit;
    }

    if (!elab_device_valid(argv[1]))
    {
        elog_error("Not right device name: %s.", argv[1]);
        ret = -2;
        goto exit;
    }

    dev = elab_device_find(argv[1]);
    status = elab_button_is_pressed(dev);
    printf("Button device %s status is %u.\n", argv[1], status);

exit:
    if (ret != 0)
    {
        elog_debug("The command example:\n    test_button_get pin_name\n");
    }
    return ret;
}

/**
  * @brief  Export the shell test command
  */
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                    test_button_get,
                    test_button_get,
                    button status get testing function);

/**************** (C) COPYRIGHT Philips Healthcare Suzhou ******END OF FILE****/
