/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#if defined(__linux__)

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "../../os/cmsis_os.h"
#include "../../3rd/Unity/unity.h"
#include "../../3rd/Unity/unity_fixture.h"
#include "../../common/elab_common.h"
#include "../../common/elab_assert.h"

#define TAG                         "ut_thread"
#include "../../common/elab_log.h"

/* Private config ------------------------------------------------------------*/
#define UT_THREAD_TEST_TIMES                      (1000)
#define UT_THREAD_TEST_NUMBER                     (64)

/* Private typedef -----------------------------------------------------------*/


/* Private function prototypes -----------------------------------------------*/
static void entry_thread_test(void *paras);

/* Private variables ---------------------------------------------------------*/
static const osThreadAttr_t thread_attr_test = 
{
    .name = "ThreadUnitTest",
    .attr_bits = osThreadDetached, 
    .priority = osPriorityRealtime,
    .stack_size = 1024,
};

/* Exported functions --------------------------------------------------------*/
/**
  * @brief  Define test group of semaphores.
  */
TEST_GROUP(thread);

/**
  * @brief  Define test fixture setup function of semaphores.
  */
TEST_SETUP(thread)
{
    srand(osKernelGetTickCount());

}

/**
  * @brief  Define test fixture tear down function of semaphores.
  */
TEST_TEAR_DOWN(thread)
{

}

/**
  * @brief  Semaphores acquiring, releasing and count getting testing.
  */
TEST(thread, new_and_terminate)
{
    osThreadId_t thread = NULL;
    osStatus_t ret_os = osOK;

    for (uint32_t i = 0; i < UT_THREAD_TEST_TIMES; i ++)
    {
        thread = osThreadNew(entry_thread_test, thread, &thread_attr_test);
        TEST_ASSERT_NOT_NULL(thread);
        ret_os = osThreadTerminate(thread);
        TEST_ASSERT_EQUAL_INT32(osOK, ret_os);
    }
}

/**
  * @brief  Define run test cases of semaphores.
  */
TEST_GROUP_RUNNER(thread)
{
    RUN_TEST_CASE(thread, new_and_terminate);
}

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Entry function for semaphores multi-thread test.
  */
static void entry_thread_test(void *paras)
{
    while (1)
    {
        osDelay(1);
    }
    osThreadExit();
}

#endif

/* ----------------------------- end of file -------------------------------- */
 