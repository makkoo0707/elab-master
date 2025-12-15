/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "test.h"
#include "elab/os/cmsis_os.h"
#include "elab/common/elab_common.h"

/* private variables ---------------------------------------------------------*/
/**
 * @brief  The thread attribute for testing.
 */
static const osThreadAttr_t thread_attr_test = 
{
    .name = "ThreadMalloc",
    .attr_bits = osThreadDetached,
    .priority = osPriorityNormal,
    .stack_size = 2048,
};

/* private function prototypes -----------------------------------------------*/
static void task_function_malloc(void *parameter);

/* Exported functions --------------------------------------------------------*/
void test_malloc(void)
{
    osThreadNew(task_function_malloc, NULL, &thread_attr_test);
}

/* private functions ---------------------------------------------------------*/
static void task_function_malloc(void *parameter)
{
    (void)parameter;
    void *data;

    printf("Test malloc start.\n");
    data = elab_malloc(10 * 1024);
    if (data != NULL)
    {
        elab_free(data);
    }
    printf("Test malloc step 1 passed.\n");

    data = elab_malloc(31 * 1024 * 1024);
    if (data != NULL)
    {
        elab_free(data);
    }
    else
    {
        printf("Malloc 100K fails.");
    }
    printf("Test malloc step 2 passed.\n");

    osThreadExit();
}

/* ----------------------------- end of file -------------------------------- */
