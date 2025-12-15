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

#define TAG                         "ut_sem"
#include "../../common/elab_log.h"

/* Private config ------------------------------------------------------------*/
#define UT_SEM_TEST_TIMES                         (10000)
#define UT_SEM_TEST_NUMBER                        (64)

/* Private typedef -----------------------------------------------------------*/
typedef struct ut_semaphore
{
    struct ut_semaphore *next;
    osThreadId_t thread;
    osSemaphoreId_t sem;
    uint32_t value;
    uint32_t id;
} ut_semaphore_t;

/* Private function prototypes -----------------------------------------------*/
static void entry_semaphore_test(void *paras);

/* Private variables ---------------------------------------------------------*/
static ut_semaphore_t *ut_sem[UT_SEM_TEST_NUMBER];
static uint64_t flag_sem_test = 0;
static osSemaphoreId_t sem = NULL;

/* Exported functions --------------------------------------------------------*/
/**
  * @brief  Define test group of semaphores.
  */
TEST_GROUP(semaphore);

/**
  * @brief  Define test fixture setup function of semaphores.
  */
TEST_SETUP(semaphore)
{
    srand(time(0));

    for (uint32_t i = 0; i < UT_SEM_TEST_NUMBER; i ++)
    {
        ut_sem[i] = elab_malloc(sizeof(ut_semaphore_t));
        TEST_ASSERT_NOT_NULL(ut_sem[i]);
    }
    for (uint32_t i = 0; i < UT_SEM_TEST_NUMBER; i ++)
    {
        ut_sem[i]->next = ut_sem[(i + 1) % UT_SEM_TEST_NUMBER];
        TEST_ASSERT_NOT_NULL(ut_sem[i]->next);
        ut_sem[i]->sem = osSemaphoreNew(1, 0, NULL);
        TEST_ASSERT_NOT_NULL(ut_sem[i]->sem);
        ut_sem[i]->thread = osThreadNew(entry_semaphore_test, ut_sem[i], NULL);
        TEST_ASSERT_NOT_NULL(ut_sem[i]->thread);
        ut_sem[i]->value = 0;
        ut_sem[i]->id = i;
        flag_sem_test |= (uint64_t)((uint64_t)1 << i);
    }

    sem = osSemaphoreNew(UT_SEM_TEST_TIMES, 0, NULL);
    TEST_ASSERT_NOT_NULL(sem);
}

/**
  * @brief  Define test fixture tear down function of semaphores.
  */
TEST_TEAR_DOWN(semaphore)
{
    for (uint32_t i = 0; i < UT_SEM_TEST_NUMBER; i ++)
    {
        osThreadTerminate(ut_sem[i]->thread);
        ut_sem[i]->thread = NULL;
        osSemaphoreDelete(ut_sem[i]->sem);
        ut_sem[i]->sem = NULL;
        elab_free(ut_sem[i]);
        TEST_ASSERT_NOT_NULL(ut_sem[i]);
    }

    osStatus_t ret_os = osSemaphoreDelete(sem);
    TEST_ASSERT(ret_os == osOK);
    sem = NULL;
}

/**
  * @brief  Semaphores acquiring, releasing and count getting testing.
  */
TEST(semaphore, acquire_release_get_count)
{
    TEST_PASS_MESSAGE("PASSED.");

    osStatus_t ret_os = osOK;
    uint32_t sem_value = 0;

    for (uint32_t i = 0; i < UT_SEM_TEST_TIMES; i ++)
    {
        sem_value = osSemaphoreGetCount(sem);
        TEST_ASSERT_EQUAL_UINT32(i, sem_value);
        ret_os = osSemaphoreRelease(sem);
        TEST_ASSERT(ret_os == osOK);
        sem_value = osSemaphoreGetCount(sem);
        TEST_ASSERT_EQUAL_UINT32((i + 1), sem_value);
    }

    for (uint32_t i = 0; i < UT_SEM_TEST_TIMES; i ++)
    {
        sem_value = osSemaphoreGetCount(sem);
        TEST_ASSERT_EQUAL_UINT32((UT_SEM_TEST_TIMES - i), sem_value);
        ret_os = osSemaphoreAcquire(sem, osWaitForever);
        TEST_ASSERT(ret_os == osOK);
        sem_value = osSemaphoreGetCount(sem);
        TEST_ASSERT_EQUAL_UINT32((UT_SEM_TEST_TIMES - i - 1), sem_value);
    }
}

/**
  * @brief  Semaphores acquiring timeout testing.
  */
TEST(semaphore, acquire_timeout)
{
    uint32_t time_current = 0;
    osStatus_t ret_os = osOK;

    uint32_t time_delay = 0;
    uint32_t times = UT_SEM_TEST_TIMES > 100 ? 100 : UT_SEM_TEST_TIMES;
    for (uint32_t i = 0; i < 100; i ++)
    {
        time_delay = 1 + (rand() % 9);          /* 1 - 10 */
        time_current = osKernelGetTickCount();
        ret_os = osSemaphoreAcquire(sem, time_delay);
        TEST_ASSERT(ret_os == osErrorTimeout);
        TEST_ASSERT_GREATER_OR_EQUAL_UINT32((time_current + time_delay),
                                            osKernelGetTickCount());
    }
}

/**
  * @brief  Semaphores acquiring and releasing multi-thread testing.
  */
TEST(semaphore, acquire_release_multi_thread)
{
    osSemaphoreRelease(ut_sem[0]->sem);

    while (1)
    {
        if (flag_sem_test != 0)
        {
            osDelay(1);
        }
        else
        {
            break;
        }
    }

    for (uint32_t i = 0; i < UT_SEM_TEST_NUMBER; i ++)
    {
        TEST_ASSERT_EQUAL_UINT32(UT_SEM_TEST_TIMES, ut_sem[i]->value);
    }
}

/**
  * @brief  Define run test cases of semaphores.
  */
TEST_GROUP_RUNNER(semaphore)
{
    RUN_TEST_CASE(semaphore, acquire_release_get_count);
    RUN_TEST_CASE(semaphore, acquire_timeout);
    RUN_TEST_CASE(semaphore, acquire_release_multi_thread);
}

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Entry function for semaphores multi-thread test.
  */
static void entry_semaphore_test(void *paras)
{
    ut_semaphore_t *ut_sem = (ut_semaphore_t *)paras;
    osStatus_t ret_os = osOK;
    uint8_t byte = 0;

    while (1)
    {
        osSemaphoreAcquire(ut_sem->sem, osWaitForever);
        ut_sem->value ++;
        if (ut_sem->value >= UT_SEM_TEST_TIMES)
        {
            elog_debug("Sem unit test %u end.", ut_sem->id);
            flag_sem_test &=~ (uint64_t)((uint64_t)1 << ut_sem->id);
            break;
        }
        osSemaphoreRelease(ut_sem->next->sem);
    }

    osSemaphoreRelease(ut_sem->next->sem);
    osThreadExit();
}

#endif

/* ----------------------------- end of file -------------------------------- */
 