/* includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "test.h"
#include "elab/os/cmsis_os.h"

/* private defines -----------------------------------------------------------*/
#define EVENT_TEST_THREAH_ONE             (1 << 1)
#define EVENT_TEST_THREAH_TWO             (1 << 2)

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

static const osEventFlagsAttr_t event_attr_test =
{
    .name = "EventTest",
    .attr_bits = osThreadDetached,
    .cb_mem = NULL,
    .cb_size = 0,
};

static osEventFlagsId_t event;

/* private function prototypes -----------------------------------------------*/
static void task_function_one(void *parameter);
static void task_function_two(void *parameter);
static void task_function_target(void *parameter);

/* Exported functions --------------------------------------------------------*/
void test_event(void)
{
    printf("test_event start. =============================================\n");
    event = osEventFlagsNew(&event_attr_test);

    osThreadNew(task_function_one, NULL, &thread_attr_test);
    osThreadNew(task_function_two, NULL, &thread_attr_test);
    osThreadNew(task_function_target, NULL, &thread_attr_test);
}

/* private functions ---------------------------------------------------------*/
static void task_function_one(void *parameter)
{
    while (1)
    {
        osDelay(1000);
        printf("test_event task_function_one set event.\n");
        osEventFlagsSet(event, EVENT_TEST_THREAH_ONE);
        osDelay(3000);
    }
}

static void task_function_two(void *parameter)
{
    while (1)
    {
        osDelay(3000);
        printf("test_event task_function_two set event.\n");
        osEventFlagsSet(event, EVENT_TEST_THREAH_TWO);
        osDelay(1000);
    }
}

static void task_function_target(void *parameter)
{
    while (1)
    {
        osEventFlagsWait(event,
                            (EVENT_TEST_THREAH_ONE | EVENT_TEST_THREAH_TWO),
                            osFlagsWaitAll, 10000);
        printf("test_event task_function_target pass. --------------------------\n");
    }
}

/* ----------------------------- end of file -------------------------------- */
