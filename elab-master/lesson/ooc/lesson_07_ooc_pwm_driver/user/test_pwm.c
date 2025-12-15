
/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
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
static int32_t test_func_pwm(int32_t argc, char *argv[])
{
    int32_t ret = 0;
    uint32_t duty_ratio;

    if (argc != 2)
    {
        ret = -1;
        goto exit;
    }

    duty_ratio = atoi(argv[1]);
    if (duty_ratio > 100)
    {
        ret = -2;
        goto exit;
    }
    
    eio_pwm_set_duty(led3, duty_ratio);

exit:
    return ret;
}

/**
  * @brief  Test shell command export
  */
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                    test_pwm,
                    test_func_pwm,
                    LED pwm testing function);

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
