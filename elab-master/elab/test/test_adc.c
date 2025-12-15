
/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "../edf/normal/elab_adc.h"
#include "../3rd/Shell/shell.h"
#include "../common/elab_log.h"

ELAB_TAG("AdcTest");

#ifdef __cplusplus
extern "C" {
#endif

/* private functions -------------------------------------------------------- */
/**
  * @brief Testing function for eLog module.
  * @retval None
  */
static int32_t test_adc_read(int32_t argc, char *argv[])
{
    (void)argc;
    (void)argv;
    
    int32_t ret = 0;
    elab_device_t *dev = NULL;
    float value = 0.0;

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
    value = elab_adc_get_value(dev);
    printf("ADC device %s value is %.3f.\n", argv[1], value);

exit:
    return ret;
}

/**
  * @brief  Test shell command export
  */
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                    test_adc_read,
                    test_adc_read,
                    ADC testing function);

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
