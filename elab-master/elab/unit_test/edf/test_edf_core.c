/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../edf/elab_device.h"
#include "../../3rd/Unity/unity.h"
#include "../../3rd/Unity/unity_fixture.h"
#include "../../common/elab_common.h"
#include "../../common/elab_assert.h"

#define TAG                         "ut_elab_device"
#include "../../common/elab_log.h"

/* Private config ------------------------------------------------------------*/
#define UT_edf_CORE_MAX                          (1)
#define UT_DEVICE_CMD_TEST                          (1)
#define UT_DEVICE_BUFF_SIZE                         (256)
#define UT_HEAP_SIZE                                (1024 * 10)
#define UT_OPEN_TIMES_MAX                           (250)

#define UT_STR_READ                                 "dev_read_data"
#define UT_STR_WRITE                                "dev_write_data"

/* Exported function prototypes ----------------------------------------------*/
elab_err_t elab_device_unregister(elab_device_t *dev);
uint32_t elab_device_get_number(void);

/* Private function prototypes -----------------------------------------------*/
static elab_err_t ops_enable(elab_device_t *dev, bool status);
static elab_err_t ops_close(elab_device_t *dev);
static int32_t ops_read(elab_device_t *dev, uint32_t pos, void *buffer, uint32_t size);
static int32_t ops_write(elab_device_t *dev,
                            uint32_t pos, const void *buffer, uint32_t size);
static elab_err_t ops_control(elab_device_t *dev, int32_t cmd, void *args);
static void thread_func_read(void *paras);
static void thread_func_write(void *paras);

/* Private variables ---------------------------------------------------------*/
static const elab_dev_ops_t serial_ops =
{
    .enable = ops_enable,
    .read = ops_read,
    .write = ops_write,
};

static const osThreadAttr_t thread_attr_dev_test = 
{
    .name = "ThreadTestDev",
    .attr_bits = osThreadDetached, 
    .priority = osPriorityLow,
    .stack_size = 2048,
};

static elab_dev_ops_t ops_reg;

static uint8_t buff_rd[UT_DEVICE_BUFF_SIZE];
static uint8_t buff_wr[UT_DEVICE_BUFF_SIZE];
static uint32_t count_rd = 0;
static uint32_t count_wr = 0;
static int32_t count_open = 0;
static bool dev_is_open = false;
static int32_t cmd_id = 0;
static osSemaphoreId_t sem_test_mode_enter = NULL;
static osSemaphoreId_t sem_test_mode_exit = NULL;
static int32_t ret_read_test = 0;
static int32_t ret_write_test = 0;

/* Exported functions --------------------------------------------------------*/
/**
  * @brief  Define test group of device core
  */
TEST_GROUP(edf_core);

/**
  * @brief  Define test fixture setup function of device core
  */
TEST_SETUP(edf_core)
{

}

/**
  * @brief  Define test fixture tear down function of device core
  */
TEST_TEAR_DOWN(edf_core)
{

}

/**
  * @brief  Register and unregister functions in device framework.
  */
TEST(edf_core, register_unregister)
{
    char dev_name[16];
    elab_device_t *dev = NULL;
    elab_device_attr_t attr =
    {
        .sole = true,
        .type = ELAB_DEVICE_UART,
        .name = NULL,
    };

    uint32_t count_num_init = elab_device_get_number();

    /* Register */
    for (uint32_t i = 0; i < UT_edf_CORE_MAX; i ++)
    {
        TEST_ASSERT_EQUAL_UINT32((i + count_num_init), elab_device_get_number());

        memset(dev_name, 0, sizeof(dev_name));
        sprintf(dev_name, "dev_name_%u", i);
        dev = elab_malloc(sizeof(elab_device_t));
        TEST_ASSERT_NOT_NULL(dev);
        attr.name = dev_name;
        elab_device_register(dev, &attr);

        TEST_ASSERT_EQUAL_UINT32((i + 1 + count_num_init), elab_device_get_number());
    }

    /* Unregister */
    for (uint32_t i = 0; i < UT_edf_CORE_MAX; i ++)
    {
        TEST_ASSERT_EQUAL_UINT32((UT_edf_CORE_MAX - i + count_num_init),
                                    elab_device_get_number());

        memset(dev_name, 0, sizeof(dev_name));
        sprintf(dev_name, "dev_name_%u", i);
        dev = elab_device_find(dev_name);
        TEST_ASSERT_NOT_NULL(dev);
        elab_device_unregister(dev);

        TEST_ASSERT_EQUAL_UINT32((UT_edf_CORE_MAX - i - 1 + count_num_init),
                                    elab_device_get_number());

        elab_free(dev);
    }
}

/**
  * @brief  Open & close functions for standalone device.
  */
TEST(edf_core, open_close_standalone)
{
    elab_device_t *dev = elab_malloc(sizeof(elab_device_t));
    TEST_ASSERT_NOT_NULL(dev);
    elab_device_attr_t attr =
    {
        .sole = true,
        .type = ELAB_DEVICE_UART,
        .name = "dev_name",
    };
    dev->ops = &serial_ops;
    elab_device_register(dev, &attr);
    TEST_ASSERT_TRUE(elab_device_is_sole(dev));
    TEST_ASSERT_EQUAL_UINT8(0, dev->enable_count);
    elab_device_open(dev);
    TEST_ASSERT_TRUE(dev_is_open);
    TEST_ASSERT_EQUAL_INT32(1, count_open);
    TEST_ASSERT_TRUE(elab_device_is_enabled(dev));
    TEST_ASSERT_EQUAL_UINT8(1, dev->enable_count);
    TEST_ASSERT_EQUAL_UINT8(1, dev->enable_count);
    elab_device_close(dev);
    TEST_ASSERT_FALSE(elab_device_is_enabled(dev));
    TEST_ASSERT_EQUAL_INT32(0, count_open);
    TEST_ASSERT_EQUAL_UINT8(0, dev->enable_count);
    TEST_ASSERT_FALSE(dev_is_open);
    elab_device_unregister(dev);
    elab_free(dev);
}

/**
  * @brief  Open & close functions for non-standalone device.
  */
TEST(edf_core, open_close_non_standalone)
{
    elab_device_t *dev = elab_malloc(sizeof(elab_device_t));
    TEST_ASSERT_NOT_NULL(dev);
    elab_device_attr_t attr =
    {
        .sole = false,
        .type = ELAB_DEVICE_UART,
        .name = "dev_name",
    };
    dev->ops = &serial_ops;
    elab_device_register(dev, &attr);
    TEST_ASSERT_EQUAL_UINT8(0, dev->enable_count);
    TEST_ASSERT_FALSE(dev_is_open);
    TEST_ASSERT_FALSE(elab_device_is_sole(dev));
    TEST_ASSERT_FALSE(elab_device_is_enabled(dev));
    TEST_ASSERT_EQUAL_INT32(0, count_open);
    for (uint32_t i = 0; i < UT_OPEN_TIMES_MAX; i ++)
    {
        TEST_ASSERT_EQUAL_UINT8(i, dev->enable_count);
        elab_device_open(dev);
        TEST_ASSERT_EQUAL_INT32(1, count_open);
        TEST_ASSERT_TRUE(dev_is_open);
        TEST_ASSERT_TRUE(elab_device_is_enabled(dev));
        TEST_ASSERT_EQUAL_UINT8((1 + i), dev->enable_count);
    }
    for (uint32_t i = 0; i < UT_OPEN_TIMES_MAX; i ++)
    {
        TEST_ASSERT_EQUAL_UINT8((UT_OPEN_TIMES_MAX - i), dev->enable_count);
        TEST_ASSERT_TRUE(elab_device_is_enabled(dev));
        TEST_ASSERT_EQUAL_INT32(1, count_open);
        elab_device_close(dev);
        TEST_ASSERT_EQUAL_UINT8((UT_OPEN_TIMES_MAX - i - 1), dev->enable_count);
        if (i == (UT_OPEN_TIMES_MAX - 1))
        {
            TEST_ASSERT_FALSE(elab_device_is_enabled(dev));
        }
        else
        {
            TEST_ASSERT_TRUE(elab_device_is_enabled(dev));
        }
    }
    TEST_ASSERT_EQUAL_INT32(0, count_open);
    TEST_ASSERT_FALSE(dev_is_open);
    TEST_ASSERT_FALSE(elab_device_is_enabled(dev));

    elab_device_unregister(dev);
    elab_free(dev);
}

/**
  * @brief  Read function in non-test mode.
  */
TEST(edf_core, read_non_test_mode)
{
    int32_t ret = 0;
    elab_device_t *dev = elab_malloc(sizeof(elab_device_t));
    TEST_ASSERT_NOT_NULL(dev);
    elab_device_attr_t attr =
    {
        .sole = true,
        .type = ELAB_DEVICE_UART,
        .name = "dev_name",
    };
    dev->ops = &serial_ops;
    elab_device_register(dev, &attr);

    memset(buff_rd, 0, UT_DEVICE_BUFF_SIZE);

    /* Read after opening. */
    elab_device_open(dev);
    ret = elab_device_read(dev, 0, buff_rd, UT_DEVICE_BUFF_SIZE);
    TEST_ASSERT_EQUAL_INT32(strlen(UT_STR_READ), ret);
    TEST_ASSERT_EQUAL_MEMORY(UT_STR_READ, buff_rd, ret);
    elab_device_close(dev);

    elab_device_unregister(dev);
    elab_free(dev);
}

/**
  * @brief  Write function in non-test mode.
  */
TEST(edf_core, write_non_test_mode)
{
    int32_t ret = 0;
    elab_device_t *dev = elab_malloc(sizeof(elab_device_t));
    TEST_ASSERT_NOT_NULL(dev);
    elab_device_attr_t attr =
    {
        .sole = true,
        .type = ELAB_DEVICE_UART,
        .name = "dev_name",
    };
    dev->ops = &serial_ops;
    elab_device_register(dev, &attr);

    /* Write after opening. */
    elab_device_open(dev);
    memset(buff_wr, 0, UT_DEVICE_BUFF_SIZE);
    ret = elab_device_write(dev, 0, UT_STR_WRITE, strlen(UT_STR_WRITE));
    TEST_ASSERT_EQUAL_INT32(strlen(UT_STR_WRITE), ret);
    TEST_ASSERT_EQUAL_MEMORY(UT_STR_WRITE, buff_wr, ret);
    elab_device_close(dev);

    elab_device_unregister(dev);
    elab_free(dev);
}

/**
  * @brief  Read function in test mode.
  */
TEST(edf_core, read_test_mode)
{
    elab_device_t *dev = elab_malloc(sizeof(elab_device_t));
    TEST_ASSERT_NOT_NULL(dev);
    osStatus_t ret_os = osOK;
    osThreadId_t thread = NULL;
    int32_t ret = 0;

    sem_test_mode_enter = osSemaphoreNew(1, 0, NULL);
    TEST_ASSERT_NOT_NULL(sem_test_mode_enter);
    sem_test_mode_exit = osSemaphoreNew(1, 0, NULL);
    TEST_ASSERT_NOT_NULL(sem_test_mode_exit);
    elab_device_attr_t attr =
    {
        .sole = true,
        .type = ELAB_DEVICE_UART,
        .name = "dev_name",
    };
    dev->ops = &serial_ops;
    elab_device_register(dev, &attr);

    /* Test mode setting and resetting test. */
    elab_device_set_test_mode(dev);
    TEST_ASSERT_TRUE(elab_device_is_test_mode(dev));
    elab_device_set_normal_mode(dev);
    TEST_ASSERT_FALSE(elab_device_is_test_mode(dev));

    /* Read the data in the current thread. */
    elab_device_open(dev);
    ret = elab_device_read(dev, 0, buff_rd, UT_DEVICE_BUFF_SIZE);
    TEST_ASSERT_EQUAL_INT32(strlen(UT_STR_READ), ret);
    TEST_ASSERT_EQUAL_MEMORY(UT_STR_READ, buff_rd, ret);

    /* Read the data in the specific newly-created thread. */
    for (uint32_t c = 0; c < 1000; c ++)
    {
        ret_read_test = 0;
        memset(buff_rd, 0, UT_DEVICE_BUFF_SIZE);
        osThreadNew(thread_func_read, dev, &thread_attr_dev_test);
        ret_os = osSemaphoreRelease(sem_test_mode_enter);
        TEST_ASSERT_EQUAL_INT32(osOK, ret_os);
        ret_os = osSemaphoreAcquire(sem_test_mode_exit, osWaitForever);
        TEST_ASSERT_EQUAL_INT32(osOK, ret_os);
        TEST_ASSERT_EQUAL_INT32(0, ret_read_test);
        for (uint32_t i = 0; i < UT_DEVICE_BUFF_SIZE; i ++)
        {
            TEST_ASSERT_EQUAL_UINT32(0, buff_rd[i]);
        }
    }

    elab_device_close(dev);
    elab_device_unregister(dev);
    elab_free(dev);
    ret_os = osSemaphoreDelete(sem_test_mode_enter);
    TEST_ASSERT_EQUAL_INT32(osOK, ret_os);
    ret_os = osSemaphoreDelete(sem_test_mode_exit);
    TEST_ASSERT_EQUAL_INT32(osOK, ret_os);
}

/**
  * @brief  Write function in test mode.
  */
TEST(edf_core, write_test_mode)
{
    elab_device_t *dev = elab_malloc(sizeof(elab_device_t));
    TEST_ASSERT_NOT_NULL(dev);
    osStatus_t ret_os = osOK;
    osThreadId_t thread = NULL;
    int32_t ret = 0;

    sem_test_mode_enter = osSemaphoreNew(1, 0, NULL);
    TEST_ASSERT_NOT_NULL(sem_test_mode_enter);
    sem_test_mode_exit = osSemaphoreNew(1, 0, NULL);
    TEST_ASSERT_NOT_NULL(sem_test_mode_exit);
    elab_device_attr_t attr =
    {
        .sole = true,
        .type = ELAB_DEVICE_UART,
        .name = "dev_name",
    };
    dev->ops = &serial_ops;
    elab_device_register(dev, &attr);

    /* Write after opening. */
    elab_device_open(dev);
    memset(buff_wr, 0, UT_DEVICE_BUFF_SIZE);
    ret = elab_device_write(dev, 0, UT_STR_WRITE, strlen(UT_STR_WRITE));
    TEST_ASSERT_EQUAL_INT32(strlen(UT_STR_WRITE), ret);
    TEST_ASSERT_EQUAL_MEMORY(UT_STR_WRITE, buff_wr, ret);

    /* Write the data in the specific thread. */
    for (uint32_t c = 0; c < 1000; c ++)
    {
        memset(buff_wr, 0, UT_DEVICE_BUFF_SIZE);
        osThreadNew(thread_func_write, dev, &thread_attr_dev_test);
        ret_os = osSemaphoreRelease(sem_test_mode_enter);
        TEST_ASSERT_EQUAL_INT32(osOK, ret_os);
        ret_os = osSemaphoreAcquire(sem_test_mode_exit, osWaitForever);
        TEST_ASSERT_EQUAL_INT32(osOK, ret_os);
        TEST_ASSERT_EQUAL_INT32(0, ret_write_test);
        for (uint32_t i = 0; i < UT_DEVICE_BUFF_SIZE; i ++)
        {
            TEST_ASSERT_EQUAL_UINT32(0, buff_wr[i]);
        }
    }

    elab_device_close(dev);
    elab_device_unregister(dev);
    elab_free(dev);
    ret_os = osSemaphoreDelete(sem_test_mode_enter);
    TEST_ASSERT_EQUAL_INT32(osOK, ret_os);
    ret_os = osSemaphoreDelete(sem_test_mode_exit);
    TEST_ASSERT_EQUAL_INT32(osOK, ret_os);
}

/**
  * @brief  Define run test cases of device core
  */
TEST_GROUP_RUNNER(edf_core)
{
    RUN_TEST_CASE(edf_core, register_unregister);
    RUN_TEST_CASE(edf_core, open_close_standalone);
    RUN_TEST_CASE(edf_core, open_close_non_standalone);
    RUN_TEST_CASE(edf_core, read_non_test_mode);
    RUN_TEST_CASE(edf_core, write_non_test_mode);
    RUN_TEST_CASE(edf_core, read_test_mode);
    RUN_TEST_CASE(edf_core, write_test_mode);
}

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Simulated driver opening function for device testing.
  */
static elab_err_t ops_enable(elab_device_t *dev, bool status)
{
    (void)dev;
    dev_is_open = status;
    if (status)
    {
        count_open ++;
    }
    else
    {
        count_open --;
    }

    return ELAB_OK;
}

/**
  * @brief  Simulated driver reading function for device testing.
  */
static int32_t ops_read(elab_device_t *dev, uint32_t pos, void *buffer, uint32_t size)
{
    (void)dev;
    (void)pos;

    memcpy(buffer, UT_STR_READ, strlen(UT_STR_READ));
    return strlen(UT_STR_READ);
}

/**
  * @brief  Simulated driver writting function for device testing.
  */
static int32_t ops_write(elab_device_t *dev,
                            uint32_t pos, const void *buffer, uint32_t size)
{
    (void)dev;
    (void)pos;

    memcpy(buff_wr, buffer, size);
    return size;
}

/**
  * @brief  Simulated driver control function for device testing.
  */
static elab_err_t ops_control(elab_device_t *dev, int32_t cmd, void *args)
{
    (void)dev;
    (void)args;

    cmd_id = cmd;
    return ELAB_OK;
}

/**
  * @brief  Thread function for reading data in testing mode.
  * @param  Thread parameter.
  */
static void thread_func_read(void *paras)
{
    elab_device_t *dev = (elab_device_t *)paras;
    osStatus_t ret_os = osOK;

    /* Wait for testing mode entry. */
    ret_os = osSemaphoreAcquire(sem_test_mode_enter, osWaitForever);
    TEST_ASSERT_EQUAL_INT32(osOK, ret_os);
    elab_device_set_test_mode(dev);

    ret_read_test = elab_device_read(dev, 0, buff_rd, UT_DEVICE_BUFF_SIZE);

    /* Exit the test mode. */
    elab_device_set_normal_mode(dev);
    ret_os = osSemaphoreRelease(sem_test_mode_exit);
    TEST_ASSERT_EQUAL_INT32(osOK, ret_os);

    osThreadExit();
}

/**
  * @brief  Thread function for writing data in testing mode.
  * @param  Thread parameter.
  */
static void thread_func_write(void *paras)
{
    elab_device_t *dev = (elab_device_t *)paras;
    osStatus_t ret_os = osOK;

    /* Wait for testing mode entry. */
    ret_os = osSemaphoreAcquire(sem_test_mode_enter, osWaitForever);
    TEST_ASSERT_EQUAL_INT32(osOK, ret_os);
    elab_device_set_test_mode(dev);

    /* Write data into the device. */
    ret_write_test = elab_device_write(dev, 0, UT_STR_WRITE, strlen(UT_STR_WRITE));

    /* Exit the test mode. */
    elab_device_set_normal_mode(dev);
    ret_os = osSemaphoreRelease(sem_test_mode_exit);
    TEST_ASSERT_EQUAL_INT32(osOK, ret_os);
    
    osThreadExit();
}

/* ----------------------------- end of file -------------------------------- */
