
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
#include "elab/edf/elab_device.h"
#include "elab/edf/normal/elab_serial.h"
#include "elab/os/cmsis_os.h"
#include "elab/common/elab_assert.h"

ELAB_TAG("Rs232Test");

#ifdef __cplusplus
extern "C" {
#endif

static const osThreadAttr_t _rs232_attr =
{
    .name = "test_rs232",
    .attr_bits = osThreadDetached,
    .priority = osPriorityRealtime,
    .stack_size = 2048,
};

static elab_device_t *rs232[2] = { NULL, NULL };

/* private functions -------------------------------------------------------- */
static void _entry_test_232(void *para)
{
    char *str_test1 = "rs232_1_tx";
    char *str_test2 = "rs232_2_tx";
    char buffer[32];
    uint32_t length = 0;

    rs232[0] = elab_device_find("RS232_1");
    rs232[1] = elab_device_find("RS232_2");
    elab_assert(rs232[0] != NULL);
    elab_assert(rs232[1] != NULL);

    elab_device_open(rs232[0]);
    elab_device_open(rs232[1]);

    while (1)
    {
        memset(buffer, 0, 32);
        elab_device_write(rs232[0], 0, str_test1, strlen(str_test1));

        length = elab_device_read(rs232[1], 0, buffer, 32);
        printf("RS232 1 tx %ld bytes. RS232_2 rx %u bytes: %s.\n",
                strlen(str_test1), length, buffer);

        memset(buffer, 0, 32);
        elab_device_write(rs232[1], 0, str_test2, strlen(str_test2));

        length = elab_device_read(rs232[0], 0, buffer, 32);
        printf("RS232 2 tx %ld bytes. RS232_1 rx %u bytes: %s.\n",
                strlen(str_test2), length, buffer);

        osDelay(1000);
    }
}

/**
  * @brief Testing function for motor device.
  * @retval None
  */
static int32_t test_rs232(int32_t argc, char *argv[])
{
    osThreadNew(_entry_test_232, NULL, &_rs232_attr);

    return 0;
}

/**
  * @brief  Test shell command export
  */
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                    test_rs232,
                    test_rs232,
                    rs232 testing function);

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
