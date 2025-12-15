
/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "elab/3rd/Shell/shell.h"
#include "elab/edf/normal/elab_pwm.h"
#include "../common/elab_log.h"

ELAB_TAG("PwmTest");

#ifdef __cplusplus
extern "C" {
#endif

/* private functions -------------------------------------------------------- */
/**
  * @brief Testing function for LED.
  * @retval None
  */
static int32_t test_pwm(int32_t argc, char *argv[])
{
    int32_t ret = 0;
    elab_device_t *dev = NULL;
    int32_t duty = 0;

    if (argc != 3)
    {
        elog_error("Not right argument number: %u. It should be 3.", argc);
        ret = -1;
        goto exit;
    }

    if (!elab_device_valid(argv[1]))
    {
        elog_error("Not right device name: %s.", argv[1]);
        ret = -2;
        goto exit;
    }

    duty = atoi(argv[2]);
    if (duty < 0 || duty > 100)
    {
        elog_error("Not right device duty: %d. It should be [0, 100]", duty);
        ret = -3;
        goto exit;
    }
    
    dev = elab_device_find(argv[1]);
    elab_pwm_set_duty(dev, duty);

exit:
    if (ret != 0)
    {
        elog_debug("The command example:\n    test_pwm dev_name 100\n");
    }
    return ret;
}

/**
  * @brief  Test shell command export
  */
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                    test_pwm,
                    test_pwm,
                    PWM testing function);

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
