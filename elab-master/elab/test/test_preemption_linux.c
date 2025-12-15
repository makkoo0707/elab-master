
/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#if defined(__linux__)

/* includes ----------------------------------------------------------------- */
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "../3rd/Shell/shell.h"
#include "../common/elab_log.h"
#include "../os/cmsis_os.h"

ELAB_TAG("eLogTest");

#ifdef __cplusplus
extern "C" {
#endif

/* private variables -------------------------------------------------------- */
static const osThreadAttr_t thread_low_attr =
{
    .name = "test_preemption_low",
    .stack_size = 2048,
    .priority = (osPriority_t)osPriorityBelowNormal,
};

static const osThreadAttr_t thread_high_attr =
{
    .name = "test_preemption_high",
    .stack_size = 2048,
    .priority = (osPriority_t)osPriorityNormal,
};

/* private function prototype ----------------------------------------------- */
static void _entry_thread_low_prio(void *para);
static void _entry_thread_high_prio(void *para);

/* private functions -------------------------------------------------------- */
/**
  * @brief Testing function for eLog module.
  * @retval None
  */
static int32_t test_preemption_linux(int32_t argc, char *argv[])
{
    (void)argc;
    (void)argv;
    
    osThreadNew(_entry_thread_low_prio, NULL, &thread_low_attr);
    osThreadNew(_entry_thread_high_prio, NULL, &thread_high_attr);

    return 0;
}

static void _entry_thread_low_prio(void *para)
{
    float a = 0.001;

    while (1)
    {
        printf("test a: %f, time: %u. ----------------------------------\n",
                a, osKernelGetTickCount());
        for (uint32_t i = 0; i < 10000000; i ++)
        {
            a += 0.001;
        }
        printf("test a: %f, time: %u.\n", a, osKernelGetTickCount());
    }
}

static void _entry_thread_high_prio(void *para)
{
    while (1)
    {
        osDelay(100);
        printf(".\n");
    }
}

/**
  * @brief  Test shell command export
  */
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                    test_preemption_linux,
                    test_preemption_linux,
                    Thread preemption testing on Linux);

#ifdef __cplusplus
}
#endif

#endif

/* ----------------------------- end of file -------------------------------- */
