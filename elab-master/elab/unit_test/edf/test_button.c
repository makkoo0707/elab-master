/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "event_def.h"
#include "../../edf/elab_device.h"
#include "../../edf/user/elab_button.h"
#include "../../3rd/Unity/unity.h"
#include "../../3rd/Unity/unity_fixture.h"
#include "../../common/elab_common.h"
#include "../../common/elab_assert.h"
#include "../../midware/esig_captor/esig_captor.h"

#define TAG                         "ut_edf_button"
#include "../../common/elab_log.h"

/* Private config ------------------------------------------------------------*/
#define UT_BUTTON_TEST_TIMES                        (5)

/* Private typedef -----------------------------------------------------------*/
typedef struct elab_button_test
{
    uint32_t time;
    uint8_t event;
    uint32_t event_signal;
    osSemaphoreId_t sem_wait_e_sig;
} elab_button_test_t;

/* Exported function prototypes ----------------------------------------------*/


/* Private function prototypes -----------------------------------------------*/
static bool _is_pressed(elab_button_t *const me);
static void _cb(elab_button_t *const me, uint8_t event_id);
static void _set_button_status(bool status);
static uint8_t _get_event_id(void);

/* Private variables ---------------------------------------------------------*/
static elab_button_t *button = NULL;
static elab_button_test_t test_data;
static bool drv_button_trig = false;
static uint8_t e_signal = ELAB_BUTTON_EVT_NONE;
static esig_captor_t esig_cap;
static bool esig_cap_started = false;

static elab_button_ops_t button_ops =
{
    .is_pressed = _is_pressed,
};

/* Exported functions --------------------------------------------------------*/
/**
  * @brief  Define test group of device core
  */
TEST_GROUP(button);

/**
  * @brief  Define test fixture setup function of device core
  */
TEST_SETUP(button)
{
    elab_device_t *dev = NULL;

    button = elab_malloc(sizeof(elab_button_t));
    TEST_ASSERT_NOT_NULL(button);

    elab_button_register(button, "button", &button_ops, NULL);
    dev = elab_device_find("button");
    TEST_ASSERT_NOT_NULL(dev);

    _set_button_status(false);

    if (!esig_cap_started)
    {
        esig_cap_started = true;
        esig_captor_init(&esig_cap, 32, EVT_BUTTON_UT_PRESS, EVT_BUTTON_UT_LONG_PRESS);

        for (uint32_t sig = EVT_BUTTON_UT_PRESS; sig <= EVT_BUTTON_UT_LONG_PRESS; sig ++)
        {
            esig_captor_sub(&esig_cap, sig);
        }
    }
}

/**
  * @brief  Define test fixture tear down function of device core
  */
TEST_TEAR_DOWN(button)
{
    elab_device_t *dev = NULL;

    elab_button_unregister(button);
    dev = elab_device_find("button");
    TEST_ASSERT_NULL(dev);
    elab_free(button);
    button = NULL;
}

/**
  * @brief  Register and unregister functions in device framework.
  */
TEST(button, is_pressed)
{
    uint32_t time_start = 0;
    elab_device_t *dev = elab_device_find("button");
    TEST_ASSERT_NOT_NULL(dev);

    /* Pressed */
    TEST_ASSERT_FALSE(elab_button_is_pressed(dev));
    _set_button_status(true);
    time_start = osKernelGetTickCount();
    while ((osKernelGetTickCount() - time_start) < 15)
    {
        TEST_ASSERT_FALSE(elab_button_is_pressed(dev));
        osDelay(1);
    }
    while ((osKernelGetTickCount() - time_start) < 25)
    {
        osDelay(1);
    }
    TEST_ASSERT_TRUE(elab_button_is_pressed(dev));

    /* Holding for 100ms. */
    for (uint32_t i = 0; i < 10; i ++)
    {
        TEST_ASSERT_TRUE(elab_button_is_pressed(dev));
        osDelay(10);
    }

    /* Release. */
    _set_button_status(false);
    time_start = osKernelGetTickCount();
    while ((osKernelGetTickCount() - time_start) < 15)
    {
        TEST_ASSERT_TRUE(elab_button_is_pressed(dev));
        osDelay(1);
    }
    while ((osKernelGetTickCount() - time_start) < 25)
    {
        osDelay(1);
    }
    TEST_ASSERT_FALSE(elab_button_is_pressed(dev));

    /* Pressed with one release for 10ms*/
    TEST_ASSERT_FALSE(elab_button_is_pressed(dev));
    _set_button_status(true);
    time_start = osKernelGetTickCount();
    while ((osKernelGetTickCount() - time_start) < 10)
    {
        TEST_ASSERT_FALSE(elab_button_is_pressed(dev));
        osDelay(1);
    }
    _set_button_status(false);
    while ((osKernelGetTickCount() - time_start) < 20)
    {
        TEST_ASSERT_FALSE(elab_button_is_pressed(dev));
        osDelay(1);
    }
    _set_button_status(true);
    while ((osKernelGetTickCount() - time_start) < 25)
    {
        osDelay(1);
    }
    TEST_ASSERT_FALSE(elab_button_is_pressed(dev));

    osDelay(25);

    /* Holding for 100ms. */
    for (uint32_t i = 0; i < 10; i ++)
    {
        TEST_ASSERT_TRUE(elab_button_is_pressed(dev));
        osDelay(10);
    }

    /* Release with one press for 10ms. */
    _set_button_status(false);
    time_start = osKernelGetTickCount();
    while ((osKernelGetTickCount() - time_start) < 10)
    {
        TEST_ASSERT_TRUE(elab_button_is_pressed(dev));
        osDelay(1);
    }
    _set_button_status(true);
    while ((osKernelGetTickCount() - time_start) < 20)
    {
        TEST_ASSERT_TRUE(elab_button_is_pressed(dev));
        osDelay(1);
    }
    _set_button_status(false);
    while ((osKernelGetTickCount() - time_start) < 25)
    {
        osDelay(1);
    }
    TEST_ASSERT_TRUE(elab_button_is_pressed(dev));
}

/**
  * @brief  Register and unregister functions in device framework.
  */
TEST(button, callback_click)
{
    uint32_t time_start = 0;
    elab_device_t *dev = elab_device_find("button");
    TEST_ASSERT_NOT_NULL(dev);
    elab_button_set_event_callback(dev, _cb);

    TEST_ASSERT_FALSE(elab_button_is_pressed(dev));
    _set_button_status(true);
    osDelay(25);
    TEST_ASSERT_TRUE(elab_button_is_pressed(dev));
    TEST_ASSERT_EQUAL_UINT8(ELAB_BUTTON_EVT_PRESSED, _get_event_id());
    e_signal = ELAB_BUTTON_EVT_NONE;

    /* Delay for click testing. */
    osDelay(ELAB_BUTTON_CLICK_TIME_MIN);

    /* Release. */
    _set_button_status(false);
    osDelay(25);
    TEST_ASSERT_FALSE(elab_button_is_pressed(dev));
    TEST_ASSERT_EQUAL_UINT8(ELAB_BUTTON_EVT_RELEASE, _get_event_id());
    e_signal = ELAB_BUTTON_EVT_NONE;

    /* Delay for double-click timeout. */
    osDelay(ELAB_BUTTON_DOUBLE_CLICK_IDLE_TIME_MAX + 20);
    TEST_ASSERT_EQUAL_UINT8(ELAB_BUTTON_EVT_CLICK, _get_event_id());
    e_signal = ELAB_BUTTON_EVT_NONE;
}

/**
  * @brief  Register and unregister functions in device framework.
  */
TEST(button, callback_long_press)
{
    uint32_t time_start = 0;
    elab_device_t *dev = elab_device_find("button");
    TEST_ASSERT_NOT_NULL(dev);
    elab_button_set_event_callback(dev, _cb);

    TEST_ASSERT_FALSE(elab_button_is_pressed(dev));
    _set_button_status(true);
    osDelay(25);
    TEST_ASSERT_TRUE(elab_button_is_pressed(dev));
    TEST_ASSERT_EQUAL_UINT8(ELAB_BUTTON_EVT_PRESSED, _get_event_id());
    e_signal = ELAB_BUTTON_EVT_NONE;

    /* Delay for long-press testing. */
    osDelay(ELAB_BUTTON_LONGPRESS_TIME_MIN);

    /* Release. */
    _set_button_status(false);
    osDelay(25);
    TEST_ASSERT_FALSE(elab_button_is_pressed(dev));
    TEST_ASSERT_EQUAL_UINT8(ELAB_BUTTON_EVT_LONGPRESS, _get_event_id());
    e_signal = ELAB_BUTTON_EVT_NONE;
}

/**
  * @brief  Register and unregister functions in device framework.
  */
TEST(button, callback_double_click)
{
    uint32_t time_start = 0;
    elab_device_t *dev = elab_device_find("button");
    TEST_ASSERT_NOT_NULL(dev);
    elab_button_set_event_callback(dev, _cb);

    TEST_ASSERT_FALSE(elab_button_is_pressed(dev));
    _set_button_status(true);
    osDelay(25);
    TEST_ASSERT_TRUE(elab_button_is_pressed(dev));
    TEST_ASSERT_EQUAL_UINT8(ELAB_BUTTON_EVT_PRESSED, _get_event_id());
    e_signal = ELAB_BUTTON_EVT_NONE;

    /* Delay for long-press testing. */
    osDelay(ELAB_BUTTON_CLICK_TIME_MIN);
    _set_button_status(false);
    osDelay(ELAB_BUTTON_DOUBLE_CLICK_IDLE_TIME_MAX - 50);
    _set_button_status(true);
    osDelay(ELAB_BUTTON_CLICK_TIME_MIN);

    /* Release. */
    _set_button_status(false);
    osDelay(25);
    TEST_ASSERT_FALSE(elab_button_is_pressed(dev));
    TEST_ASSERT_EQUAL_UINT8(ELAB_BUTTON_EVT_DOUBLE_CLICK, _get_event_id());
    e_signal = ELAB_BUTTON_EVT_NONE;
}

/**
  * @brief  Register and unregister functions in device framework.
  */
TEST(button, signal_click)
{
    uint32_t time_start = 0;
    elab_device_t *dev = elab_device_find("button");
    TEST_ASSERT_NOT_NULL(dev);
    for (uint32_t id = 0; id < ELAB_BUTTON_EVT_MAX; id ++)
    {
        elab_button_set_event_signal(dev, id, (EVT_BUTTON_UT_PRESS + id));
    }

    TEST_ASSERT_FALSE(elab_button_is_pressed(dev));
    _set_button_status(true);
    osDelay(25);
    TEST_ASSERT_TRUE(elab_button_is_pressed(dev));

    /* Delay for click testing. */
    osDelay(ELAB_BUTTON_CLICK_TIME_MIN);

    /* Release. */
    _set_button_status(false);
    osDelay(25);
    TEST_ASSERT_FALSE(elab_button_is_pressed(dev));

    /* Delay for double-click timeout. */
    osDelay(ELAB_BUTTON_DOUBLE_CLICK_IDLE_TIME_MAX + 20);

    TEST_ASSERT_EQUAL_UINT32(EVT_BUTTON_UT_PRESS, esig_captor_pop(&esig_cap));
    TEST_ASSERT_EQUAL_UINT32(EVT_BUTTON_UT_RELEASE, esig_captor_pop(&esig_cap));
    TEST_ASSERT_EQUAL_UINT32(EVT_BUTTON_UT_CLICK, esig_captor_pop(&esig_cap));
    TEST_ASSERT_EQUAL_UINT32(Q_NULL_SIG, esig_captor_pop(&esig_cap));
}

/**
  * @brief  Register and unregister functions in device framework.
  */
TEST(button, signal_long_press)
{
    uint32_t time_start = 0;
    elab_device_t *dev = elab_device_find("button");
    TEST_ASSERT_NOT_NULL(dev);
    for (uint32_t id = 0; id < ELAB_BUTTON_EVT_MAX; id ++)
    {
        elab_button_set_event_signal(dev, id, (EVT_BUTTON_UT_PRESS + id));
    }

    TEST_ASSERT_FALSE(elab_button_is_pressed(dev));
    _set_button_status(true);
    osDelay(25);
    TEST_ASSERT_TRUE(elab_button_is_pressed(dev));

    /* Delay for long-press testing. */
    osDelay(ELAB_BUTTON_LONGPRESS_TIME_MIN);

    /* Release. */
    _set_button_status(false);
    osDelay(25);
    TEST_ASSERT_FALSE(elab_button_is_pressed(dev));

    TEST_ASSERT_EQUAL_UINT32(EVT_BUTTON_UT_PRESS, esig_captor_pop(&esig_cap));
    TEST_ASSERT_EQUAL_UINT32(EVT_BUTTON_UT_RELEASE, esig_captor_pop(&esig_cap));
    TEST_ASSERT_EQUAL_UINT32(EVT_BUTTON_UT_LONG_PRESS, esig_captor_pop(&esig_cap));
    TEST_ASSERT_EQUAL_UINT32(Q_NULL_SIG, esig_captor_pop(&esig_cap));
}

/**
  * @brief  Register and unregister functions in device framework.
  */
TEST(button, signal_double_click)
{
    uint32_t time_start = 0;
    elab_device_t *dev = elab_device_find("button");
    TEST_ASSERT_NOT_NULL(dev);
    for (uint32_t id = 0; id < ELAB_BUTTON_EVT_MAX; id ++)
    {
        elab_button_set_event_signal(dev, id, (EVT_BUTTON_UT_PRESS + id));
    }

    TEST_ASSERT_FALSE(elab_button_is_pressed(dev));
    _set_button_status(true);
    osDelay(25);
    TEST_ASSERT_TRUE(elab_button_is_pressed(dev));

    /* Delay for long-press testing. */
    osDelay(ELAB_BUTTON_CLICK_TIME_MIN);
    _set_button_status(false);
    osDelay(ELAB_BUTTON_DOUBLE_CLICK_IDLE_TIME_MAX - 50);
    _set_button_status(true);
    osDelay(ELAB_BUTTON_CLICK_TIME_MIN);

    /* Release. */
    _set_button_status(false);
    osDelay(25);
    TEST_ASSERT_FALSE(elab_button_is_pressed(dev));

    TEST_ASSERT_EQUAL_UINT32(EVT_BUTTON_UT_PRESS, esig_captor_pop(&esig_cap));
    TEST_ASSERT_EQUAL_UINT32(EVT_BUTTON_UT_RELEASE, esig_captor_pop(&esig_cap));
    TEST_ASSERT_EQUAL_UINT32(EVT_BUTTON_UT_PRESS, esig_captor_pop(&esig_cap));
    TEST_ASSERT_EQUAL_UINT32(EVT_BUTTON_UT_RELEASE, esig_captor_pop(&esig_cap));
    TEST_ASSERT_EQUAL_UINT32(EVT_BUTTON_UT_DOUBLE_CLICK, esig_captor_pop(&esig_cap));
    TEST_ASSERT_EQUAL_UINT32(Q_NULL_SIG, esig_captor_pop(&esig_cap));
}

/**
  * @brief  Define run test cases of device core
  */
TEST_GROUP_RUNNER(button)
{
    RUN_TEST_CASE(button, is_pressed);
    RUN_TEST_CASE(button, callback_click);
    RUN_TEST_CASE(button, callback_long_press);
    RUN_TEST_CASE(button, callback_double_click);
    RUN_TEST_CASE(button, signal_click);
    RUN_TEST_CASE(button, signal_long_press);
    RUN_TEST_CASE(button, signal_double_click);
}

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Simulated driver opening function for device testing.
  */
static bool _is_pressed(elab_button_t *const me)
{
    (void)me;

    return drv_button_trig;
}

static void _cb(elab_button_t *const me, uint8_t event_id)
{
    e_signal = event_id;
}

static uint8_t _get_event_id(void)
{
    elab_device_t *dev = elab_device_find("button");
    elab_device_lock(dev);
    uint8_t sig = e_signal;
    elab_device_unlock(dev);

    return sig;
}

static void _set_button_status(bool status)
{
    elab_device_t *dev = elab_device_find("button");
    elab_device_lock(dev);
    drv_button_trig = status;
    elab_device_unlock(dev);
}

/* ----------------------------- end of file -------------------------------- */
