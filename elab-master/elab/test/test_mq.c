
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
#include "elab/common/elab_assert.h"
#include "elab/edf/elab_device.h"

ELAB_TAG("MqTest");

#ifdef __cplusplus
extern "C" {
#endif

#define TEST_MQ_SIZE                        (1024)

static osMessageQueueId_t mq_test = NULL;
static osThreadId_t mq_test_thread = NULL;
static bool recv_start = false;

/**
 * @brief  The thread attribute for testing.
 */
static const osThreadAttr_t thread_attr_mq_test = 
{
    .name = "ThreadStartPoll",
    .attr_bits = osThreadDetached,
    .priority = osPriorityNormal,
    .stack_size = 2048,
};

/* private functions -------------------------------------------------------- */
static void _thread_func(void *para)
{
    uint8_t data;
    uint8_t data_bkp;

    while (1)
    {
        osMessageQueueGet(mq_test, &data, NULL, osWaitForever);
        printf("0x%02x ", data);
        if (((data + 1) % 16) == 0)
        {
            printf("\n");
        }
        if (recv_start == false)
        {
            recv_start = true;
            data_bkp = data;
        }
        else
        {
            if (((uint8_t)(data_bkp + 1)) != data)
            {
                elog_error("Error data %u %u");
            }
        }
        data_bkp = data;
        if (data_bkp == 0xff)
        {
            elog_debug("testing passed.");
        }
    }
}

/**
  * @brief Testing function for motor device.
  * @retval None
  */
static int32_t test_mq(int32_t argc, char *argv[])
{
    if (mq_test == NULL)
    {
        mq_test = osMessageQueueNew(TEST_MQ_SIZE, 1, NULL);
        elab_assert(mq_test != NULL);

        mq_test_thread = osThreadNew(_thread_func, NULL, &thread_attr_mq_test);
        elab_assert(mq_test_thread != NULL);
    }

    uint8_t data;
    for (uint32_t i = 0; i < 256; i ++)
    {
        data = (uint8_t)i;
        osMessageQueuePut(mq_test, &data, 0, osWaitForever);
    }
    
    return 0;
}

/**
  * @brief  Test shell command export
  */
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                    test_mq,
                    test_mq,
                    message queue testing function);

SHELL_EXPORT_KEY(SHELL_CMD_PERMISSION(0), ESH_KEY_F5, test_mq, f5);

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
