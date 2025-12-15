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
#include "../../3rd/qpc/include/qpc.h"
#include "../../common/elab_common.h"
#include "../../common/elab_assert.h"

Q_DEFINE_THIS_FILE

/* Private config ------------------------------------------------------------*/
#define UT_ACTOR_TIMES                          (10000)
#define UT_ACTOR_E_QUEUE_SIZE                   (32)

/* private state function prototypes -----------------------------------------*/
static QState _state_initial(QActive * const me, void const * const par);
static QState _state_work(QActive * const me, QEvt const * const e);

/* Private variables ---------------------------------------------------------*/
static QActive *sm = NULL;

/* Exported functions --------------------------------------------------------*/
/**
  * @brief  Define test group of device core
  */
TEST_GROUP(actor);

/**
  * @brief  Define test fixture setup function of device core
  */
TEST_SETUP(actor)
{

}

/**
  * @brief  Define test fixture tear down function of device core
  */
TEST_TEAR_DOWN(actor)
{
}

/**
  * @brief  Register and unregister functions in device framework.
  */
TEST(actor, funcs)
{
    for (uint32_t i = 0; i < UT_ACTOR_TIMES; i ++)
    {
        sm = elab_malloc(sizeof(QActive));
        TEST_ASSERT_NOT_NULL(sm);
        QEvt **e_queue = elab_malloc(sizeof(QEvt *) * UT_ACTOR_E_QUEUE_SIZE);
        TEST_ASSERT_NOT_NULL(e_queue);
        void *stack = elab_malloc(1024);
        TEST_ASSERT_NOT_NULL(stack);

        QActive_ctor(sm, Q_STATE_CAST(&_state_initial));
        QACTIVE_START(sm,
                        osPriorityNormal,               /* QP priority */
                        (QEvt const **)e_queue,
                        UT_ACTOR_E_QUEUE_SIZE,          /* evt queue */
                        (void *)stack, 1024U,           /* no per-thread stack */
                        (QEvt *)0);                     /* no initialization event */
        QActive_stop(sm);

        elab_free(sm);
        elab_free(e_queue);
        elab_free(stack);
    }
}

/**
  * @brief  Define run test cases of device core
  */
TEST_GROUP_RUNNER(actor)
{
    RUN_TEST_CASE(actor, funcs);
}

/* private state function ----------------------------------------------------*/
static QState _state_initial(QActive * const me, void const * const par)
{
    (void)par;
    (void)me;

    return Q_TRAN(&_state_work);
}

static QState _state_work(QActive * const me, QEvt const * const e)
{
    (void)me;
    (void)e;

    return Q_SUPER(&QHsm_top);
}

/* ----------------------------- end of file -------------------------------- */
