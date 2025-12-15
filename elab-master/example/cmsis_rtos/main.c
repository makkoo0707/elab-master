/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdio.h>
#include "elab/os/cmsis_os.h"
#include "test.h"

/* private variables ---------------------------------------------------------*/
/**
 * @brief  The thread attribute for testing.
 */
static const osThreadAttr_t thread_attr_test = 
{
    .name = "ThreadTest",
    .attr_bits = osThreadDetached,
    .priority = osPriorityNormal,
    .stack_size = 2048,
};

/* private function prototypes -----------------------------------------------*/
static void task_function_test(void *parameter);
static void timer_function(void *parameter);

/* public functions ----------------------------------------------------------*/
/**
  * @brief  The main function.
  */
int main(int32_t argc, char **argv)
{
    osKernelInitialize();
    osThreadNew(task_function_test, NULL, &thread_attr_test);

    /* Start the scheduler. */
    osKernelStart();

    while (1);

    return 0;
}

/* private functions ---------------------------------------------------------*/
static void task_function_test(void *parameter)
{
    static osTimerId_t timer = NULL;

    static const osTimerAttr_t timer_attr_test =
    {
        .name = "timer_test",
        .attr_bits = 0,
        .cb_mem = NULL,
        .cb_size = 0,
    };
    timer = osTimerNew(timer_function, osTimerPeriodic, "Timer", &timer_attr_test);
    osTimerStart(timer, 1000);

    test_event();
    test_malloc();

    while (1)
    {
        osDelay(1000);
    }
}

static void timer_function(void *parameter)
{
    printf("Timer test.\n");
}

/* ----------------------------- end of file -------------------------------- */
