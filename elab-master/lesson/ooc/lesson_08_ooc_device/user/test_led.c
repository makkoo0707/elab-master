
/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include <stdint.h>
#include <string.h>
#include "shell.h"
#include "dev_list.h"

#ifdef __cplusplus
extern "C" {
#endif

/* private functions -------------------------------------------------------- */
/**
  * @brief Testing function for LED.
  * @retval None
  */
static int32_t test_func_led(int32_t argc, char *argv[])
{
    int32_t ret = 0;

    if (argc != 2)
    {
        ret = -1;
        goto exit;
    }

    if (strcmp(argv[1], "0") == 0)
    {
        eio_pin_set_status(led, false);
    }
    else if (strcmp(argv[1], "1") == 0)
    {
        eio_pin_set_status(led, true);
    }
    else
    {
        ret = -2;
        goto exit;
    }

exit:
    return ret;
}

/**
  * @brief  Test shell command export
  */
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                    test_led,
                    test_func_led,
                    LED testing function);

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
