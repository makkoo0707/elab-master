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

#define TAG                         "ut_mutex"
#include "../../common/elab_log.h"

/* Private config ------------------------------------------------------------*/
#define UT_MUTEX_TEST_TIMES                         (1000)
#define UT_MUTEX_TEST_DATA_SIZE                     (256)
#define UT_MUTEX_TEST_NUMBER                        (64)

/* Private typedef -----------------------------------------------------------*/
typedef struct ut_mutex
{
    osThreadId_t thread;
    osSemaphoreId_t sem;
    uint8_t value;
    uint8_t id;
} ut_mutex_t;

/* Private function prototypes -----------------------------------------------*/
static void entry_mutex_test(void *paras);

/* Private variables ---------------------------------------------------------*/
static uint8_t *shared_data = NULL;
static osMutexId_t mutex = NULL;
static ut_mutex_t *ut_mutex[UT_MUTEX_TEST_NUMBER];
static osSemaphoreId_t sem_one_time = NULL;
static uint64_t flag_mutex_tested = 0;

static const osMutexAttr_t mutex_attr =
{
    "ut_mutex", 
    osMutexRecursive | osMutexPrioInherit, 
    NULL, 
    0U 
};

/* Exported functions --------------------------------------------------------*/
/**
  * @brief  Define test group of mutex.
  */
TEST_GROUP(mutex);

/**
  * @brief  Define test fixture setup function of mutex.
  */
TEST_SETUP(mutex)
{
    shared_data = elab_malloc(UT_MUTEX_TEST_DATA_SIZE);
    TEST_ASSERT_NOT_NULL(shared_data);
    memset(shared_data, 0, UT_MUTEX_TEST_DATA_SIZE);

    mutex = osMutexNew(&mutex_attr);
    TEST_ASSERT_NOT_NULL(mutex);

    sem_one_time = osSemaphoreNew(1, 0, NULL);
    TEST_ASSERT_NOT_NULL(sem_one_time);

    for (uint32_t i = 0; i < UT_MUTEX_TEST_NUMBER; i ++)
    {
        ut_mutex[i] = elab_malloc(sizeof(ut_mutex_t));
        TEST_ASSERT_NOT_NULL(ut_mutex[i]);
        ut_mutex[i]->value = (uint8_t)i;
        ut_mutex[i]->id = (uint8_t)i;
        ut_mutex[i]->sem = osSemaphoreNew(1, 0, NULL);
        TEST_ASSERT_NOT_NULL(ut_mutex[i]->sem);
        ut_mutex[i]->thread = osThreadNew(entry_mutex_test, ut_mutex[i], NULL);
        TEST_ASSERT_NOT_NULL(ut_mutex[i]->thread);
    }
}

/**
  * @brief  Define test fixture tear down function of mutex.
  */
TEST_TEAR_DOWN(mutex)
{
    uint8_t byte = shared_data[0];
    for (uint32_t i = 0; i < UT_MUTEX_TEST_DATA_SIZE; i ++)
    {
        if (byte != shared_data[i])
        {
            elog_error("Tear down error. mutex test error. %d: 0x%02x, 0x%02x.",
                            i, byte, shared_data[i]);
        }
    }

    for (uint32_t i = 0; i < UT_MUTEX_TEST_NUMBER; i ++)
    {
        osThreadTerminate(ut_mutex[i]->thread);
        ut_mutex[i]->thread = NULL;
        osSemaphoreDelete(ut_mutex[i]->sem);
        ut_mutex[i]->sem = NULL;
        elab_free(ut_mutex[i]);
        ut_mutex[i] = NULL;
    }
    elab_free(shared_data);
    osMutexDelete(mutex);
    osSemaphoreDelete(sem_one_time);
    shared_data = NULL;
    mutex = NULL;
    sem_one_time = NULL;
}

/**
  * @brief  Mutex general testing.
  */
TEST(mutex, test)
{
    for (uint32_t i = 0; i < UT_MUTEX_TEST_TIMES; i ++)
    {
        for (uint64_t j = 0; j < UT_MUTEX_TEST_NUMBER; j ++)
        {
            flag_mutex_tested |= (uint64_t)((uint64_t)1 << j);
        }
        for (uint32_t m = 0; m < UT_MUTEX_TEST_NUMBER; m ++)
        {
            osSemaphoreRelease(ut_mutex[m]->sem);
        }
        osSemaphoreAcquire(sem_one_time, osWaitForever);
    }
}

/**
  * @brief  Define run test cases of mutex
  */
TEST_GROUP_RUNNER(mutex)
{
    RUN_TEST_CASE(mutex, test);
}

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Entry function for mutex multi-thread test.
  */
static void entry_mutex_test(void *paras)
{
    ut_mutex_t *ut_mutex = (ut_mutex_t *)paras;
    osStatus_t ret_os = osOK;
    uint8_t byte = 0;

    while (1)
    {
        osSemaphoreAcquire(ut_mutex->sem, osWaitForever);

        ret_os = osMutexAcquire(mutex, osWaitForever);
        elab_assert(ret_os == osOK);
        byte = shared_data[0];
        for (uint32_t i = 0; i < UT_MUTEX_TEST_DATA_SIZE; i ++)
        {
            if (byte != shared_data[i])
            {
                elog_error("Error. mutex %d test error. %d: 0x%02x, 0x%02x.",
                                ut_mutex->id, i, byte, shared_data[i]);
            }
            shared_data[i] = ut_mutex->value;
        }
        flag_mutex_tested &=~ (uint64_t)(1 << ut_mutex->id);
        ret_os = osMutexRelease(mutex);
        elab_assert(ret_os == osOK);
        if (flag_mutex_tested == 0)
        {
            osSemaphoreRelease(sem_one_time);
        }
    }
}

#endif

/* ----------------------------- end of file -------------------------------- */
 