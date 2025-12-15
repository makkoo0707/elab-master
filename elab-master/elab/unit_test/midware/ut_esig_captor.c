/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "event_def.h"
#include "../../3rd/Unity/unity.h"
#include "../../3rd/Unity/unity_fixture.h"
#include "../../midware/esig_captor/esig_captor.h"
#include "../../common/elab_common.h"
#include "../../common/elab_assert.h"

/* Private config ------------------------------------------------------------*/
#define UT_ESIG_CAP_CAPACITY                        (128)
#define UT_ESIG_CAP_TIMES                           (1000)
#define UT_ESIG_PUB_DELAY_TIME                      (2)

/* Private variables ---------------------------------------------------------*/
static esig_captor_t esig_capture;
static esig_captor_t *esig_cap = NULL;
static bool esig_capture_init = false;
static elib_queue_t e_queue_temp;

/* Exported functions --------------------------------------------------------*/
/**
  * @brief  Define test group of device core
  */
TEST_GROUP(esig_captor);

/**
  * @brief  Define test fixture setup function of device core
  */
TEST_SETUP(esig_captor)
{
    srand(time(0));

    if (!esig_capture_init)
    {
        esig_capture_init = true;

        esig_cap = &esig_capture;
        TEST_ASSERT_NOT_NULL(esig_cap);

        esig_captor_init(esig_cap, UT_ESIG_CAP_CAPACITY, ECAP_TEST_MIN, ECAP_TEST_MAX);
        TEST_ASSERT_EQUAL_UINT32(0, esig_captor_get_event_count(esig_cap));
        TEST_ASSERT_EQUAL_UINT32(Q_NULL_SIG, esig_captor_pop(esig_cap));

        void *mem_queue = elab_malloc(sizeof(uint32_t) * UT_ESIG_CAP_CAPACITY);
        elib_queue_init(&e_queue_temp, mem_queue, 
                        sizeof(uint32_t) * UT_ESIG_CAP_CAPACITY);
    }
}

/**
  * @brief  Define test fixture tear down function of device core
  */
TEST_TEAR_DOWN(esig_captor)
{
}

/**
  * @brief  Register and unregister functions in device framework.
  */
TEST(esig_captor, sub_unsub)
{
    for (uint32_t i = ECAP_TEST_MIN; i <= ECAP_TEST_MAX; i ++)
    {
        QF_publish_(Q_NEW(QEvt, i));
        osDelay(UT_ESIG_PUB_DELAY_TIME);
        TEST_ASSERT_EQUAL_UINT32(0, esig_captor_get_event_count(esig_cap));

        esig_captor_sub(esig_cap, i);
        QF_publish_(Q_NEW(QEvt, i));
        osDelay(UT_ESIG_PUB_DELAY_TIME);
        TEST_ASSERT_EQUAL_UINT32(1, esig_captor_get_event_count(esig_cap));
        TEST_ASSERT_EQUAL_UINT32(i, esig_captor_pop(esig_cap));
        TEST_ASSERT_EQUAL_UINT32(0, esig_captor_get_event_count(esig_cap));

        esig_captor_unsub(esig_cap, i);
        QF_publish_(Q_NEW(QEvt, i));
        osDelay(UT_ESIG_PUB_DELAY_TIME);
        TEST_ASSERT_EQUAL_UINT32(0, esig_captor_get_event_count(esig_cap));
        TEST_ASSERT_EQUAL_UINT32(Q_NULL_SIG, esig_captor_pop(esig_cap));
    }
}

/**
  * @brief  Register and unregister functions in device framework.
  */
TEST(esig_captor, functions)
{
    uint32_t e_sig = ECAP_TEST_MIN;

    for (uint32_t sig = ECAP_TEST_MIN; sig <= ECAP_TEST_MAX; sig ++)
    {
        esig_captor_sub(esig_cap, sig);
    }

    for (uint32_t i = 0; i < UT_ESIG_CAP_TIMES; i ++)
    {
        /* Generate one random event. */
        do
        {
            e_sig = rand() % (ECAP_TEST_MAX + 1);
        } while (e_sig < ECAP_TEST_MIN);
        QEvt *e = Q_NEW(QEvt, e_sig);

        if (i >= UT_ESIG_CAP_CAPACITY)
        {
            TEST_ASSERT_EQUAL_UINT32(
                UT_ESIG_CAP_CAPACITY, esig_captor_get_event_count(esig_cap));
            QF_publish_(e);
            osDelay(UT_ESIG_PUB_DELAY_TIME);
            TEST_ASSERT_EQUAL_UINT32(
                UT_ESIG_CAP_CAPACITY, esig_captor_get_event_count(esig_cap));
        }
        else
        {
            TEST_ASSERT_EQUAL_UINT32(i, esig_captor_get_event_count(esig_cap));
            QF_publish_(e);
            osDelay(UT_ESIG_PUB_DELAY_TIME);
            TEST_ASSERT_EQUAL_UINT32((i + 1), esig_captor_get_event_count(esig_cap));
        }
    }

    for (uint32_t i = 0; i < UT_ESIG_CAP_TIMES; i ++)
    {
        if (i >= UT_ESIG_CAP_CAPACITY)
        {
            TEST_ASSERT_EQUAL_UINT32(0, esig_captor_get_event_count(esig_cap));
            TEST_ASSERT_EQUAL_UINT32(Q_NULL_SIG, esig_captor_pop(esig_cap));
            TEST_ASSERT_EQUAL_UINT32(0, esig_captor_get_event_count(esig_cap));
        }
        else
        {
            TEST_ASSERT_EQUAL_UINT32((UT_ESIG_CAP_CAPACITY - i),
                                        esig_captor_get_event_count(esig_cap));
            uint32_t e_sig_pop = esig_captor_pop(esig_cap);
            TEST_ASSERT_GREATER_OR_EQUAL_UINT32(ECAP_TEST_MIN, e_sig_pop);
            TEST_ASSERT_LESS_OR_EQUAL_UINT32(ECAP_TEST_MAX, e_sig_pop);
            TEST_ASSERT_EQUAL_UINT32((UT_ESIG_CAP_CAPACITY - i - 1),
                                        esig_captor_get_event_count(esig_cap));
            
        }
    }
}

/**
  * @brief  Define run test cases of device core
  */
TEST_GROUP_RUNNER(esig_captor)
{
    RUN_TEST_CASE(esig_captor, sub_unsub);
    RUN_TEST_CASE(esig_captor, functions);
}

/* ----------------------------- end of file -------------------------------- */
