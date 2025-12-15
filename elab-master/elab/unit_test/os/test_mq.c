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

#define TAG                         "ut_mq"
#include "../../common/elab_log.h"

/* Private config ------------------------------------------------------------*/
#define UT_SIMU_MQ_BUFF_SIZE                        (256)
#define UT_SIMU_MQ_TIMES                            (1000)

#define UT_STR_READ                                 "dev_read_data"
#define UT_STR_WRITE                                "dev_write_data"

/* Exported function prototypes ----------------------------------------------*/


/* Private function prototypes -----------------------------------------------*/
static void entry_mq_read(void *paras);

/* Private variables ---------------------------------------------------------*/
static osMessageQueueId_t mq = NULL;
static uint8_t *buff_tx = NULL;
static osSemaphoreId_t sem = NULL;
static uint32_t count_rx = 0;

static const osThreadAttr_t attr_serial_read = 
{
    .name = "ThreadMqTest",
    .attr_bits = osThreadDetached, 
    .priority = osPriorityRealtime,
    .stack_size = 2048,
};

/* Exported functions --------------------------------------------------------*/
/**
  * @brief  Define test group of device core
  */
TEST_GROUP(mq);

/**
  * @brief  Define test fixture setup function of device core
  */
TEST_SETUP(mq)
{
    srand(time(0));

    buff_tx = elab_malloc(UT_SIMU_MQ_BUFF_SIZE);
    TEST_ASSERT_NOT_NULL(buff_tx);

    /* Create one semaphore and a message queue. */
    sem = osSemaphoreNew(1, 0, NULL);
    TEST_ASSERT_NOT_NULL(sem);
    mq = osMessageQueueNew(2048, 1, NULL);
    TEST_ASSERT_NOT_NULL(mq);
}

/**
  * @brief  Define test fixture tear down function of device core
  */
TEST_TEAR_DOWN(mq)
{
    osStatus_t ret_os = osOK;

    elab_free(buff_tx);

    ret_os = osSemaphoreDelete(sem);
    TEST_ASSERT(ret_os == osOK);
    ret_os = osMessageQueueDelete(mq);
    TEST_ASSERT(ret_os == osOK);
}


/**
  * @brief  Assert failures in device framework functions.
  */
TEST(mq, rx_tx_cross_thread)
{
    int32_t ret = 0;
    uint32_t time_start = 0;
    osStatus_t ret_os = osOK;

    /* Start one thread for serial port reading. */
    osThreadId_t thread =
        osThreadNew(entry_mq_read, mq, &attr_serial_read);
    TEST_ASSERT_NOT_NULL(thread);

    /* Generate random data in the tx buffer. */
    for (uint32_t i = 0; i < UT_SIMU_MQ_BUFF_SIZE; i ++)
    {
        buff_tx[i] = rand() % UINT8_MAX;
    }

    for (uint32_t i = 0; i < UT_SIMU_MQ_TIMES; i ++)
    {
        count_rx = 0;

        for (uint32_t m = 0; m < UT_SIMU_MQ_BUFF_SIZE; m ++)
        {
            osMessageQueuePut(mq, &buff_tx[m], 0, osWaitForever);
        }
        ret_os = osSemaphoreAcquire(sem, osWaitForever);
        TEST_ASSERT(ret_os == osOK);

        TEST_ASSERT_EQUAL_UINT32(UT_SIMU_MQ_BUFF_SIZE, count_rx);
    }

    ret_os = osThreadTerminate(thread);
    TEST_ASSERT(ret_os == osOK);
}

/**
  * @brief  Define run test cases of device core
  */
TEST_GROUP_RUNNER(mq)
{
    RUN_TEST_CASE(mq, rx_tx_cross_thread);
}

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Simulated driver opening function for serial device testing.
  */
static void entry_mq_read(void *paras)
{
    (void)paras;

    osStatus_t ret = osOK;
    osStatus_t ret_bkp = osErrorTimeout;
    char ch;

    while (1)
    {
        ret = osMessageQueueGet(mq, &ch, NULL, 5);
        if (ret == osOK)
        {
            count_rx ++;
        }
        else if (ret == osErrorTimeout && ret_bkp == osOK)
        {
            osSemaphoreRelease(sem);
        }

        ret_bkp = ret;
    }
}

#endif

/* ----------------------------- end of file -------------------------------- */
