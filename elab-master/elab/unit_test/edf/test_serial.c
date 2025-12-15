/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "../../edf/normal/elab_serial.h"
#include "../../3rd/Unity/unity.h"
#include "../../3rd/Unity/unity_fixture.h"
#include "../../common/elab_common.h"
#include "../../common/elab_assert.h"
#include "../../common/elab_log.h"

ELAB_TAG("UtSerial");

/* Private config ------------------------------------------------------------*/
#define UT_DEVICE_CORE_MAX                          (1)
#define UT_DEVICE_CMD_TEST                          (1)
#define UT_DEVICE_BUFF_SIZE                         (256)
#define UT_OPEN_TIMES_MAX                           (250)

#define UT_STR_READ                                 "dev_read_data"
#define UT_STR_WRITE                                "dev_write_data"
#define UT_SERIAL_NAME                              "serial_name"

/* Exported function prototypes ----------------------------------------------*/
elab_err_t device_unregister(elab_device_t *dev);
uint32_t elab_device_get_number(void);

/* Private function prototypes -----------------------------------------------*/
static elab_err_t ops_enable(elab_serial_t *serial, bool status);
#if defined(__linux__) || defined(_WIN32)
static int32_t ops_read(elab_serial_t *serial, void *pbuf, uint32_t size);
#endif
static int32_t ops_write(elab_serial_t *serial,
                            const void *pbuf, uint32_t size);
static int32_t ops_write_to_self(elab_serial_t *serial,
                                    const void *pbuf, uint32_t size);
static elab_err_t ops_config(elab_serial_t *serial, elab_serial_config_t *pcfg);
static void ops_set_tx(elab_serial_t *serial, bool status);
static void thread_func_read(void *paras);
static void thread_func_write(void *paras);
static void thread_func_config(void *paras);
static void make_data_for_reading(void);
static void inform_serial_tx_end_delay(elab_serial_t *serial);
static void entry_send_self(void *paras);

/* Private variables ---------------------------------------------------------*/
static elab_serial_ops_t serial_ops =
{
    .enable = ops_enable,
#if defined(__linux__) || defined(_WIN32)
    .read = ops_read,
#endif
    .write = ops_write,
    .config = ops_config,
    .set_tx = ops_set_tx,
};

static elab_serial_ops_t serial_ops_send_self =
{
    .enable = ops_enable,
#if defined(__linux__) || defined(_WIN32)
    .read = ops_read,
#endif
    .write = ops_write_to_self,
    .config = ops_config,
    .set_tx = ops_set_tx,
};

static const osThreadAttr_t thread_attr_dev_test = 
{
    .name = "ThreadTestSerial",
    .attr_bits = osThreadDetached, 
    .priority = osPriorityLow,
    .stack_size = 2048,
};

static const osThreadAttr_t attr_serial_read = 
{
    .name = "ThreadMqTest",
    .attr_bits = osThreadDetached, 
    .priority = osPriorityRealtime,
    .stack_size = 2048,
};

static elab_serial_ops_t ops_reg;

static uint8_t buff_rd[UT_DEVICE_BUFF_SIZE];
static uint8_t buff_wr[UT_DEVICE_BUFF_SIZE];
static uint32_t count_rd = 0;
static uint32_t count_wr = 0;
static uint32_t count_set_tx = 0;
static int32_t count_open = 0;
static int32_t ret_read_test = 0;
static int32_t ret_write_test = 0;
static bool serial_tx_mode = true;
static bool serial_is_open = false;
static bool serial_set_tx = false;
static osSemaphoreId_t sem_test_mode_enter = NULL;
static osSemaphoreId_t sem_test_mode_exit = NULL;
static osSemaphoreId_t sem_send_self = NULL;
static elab_serial_attr_t config_set;
static osMessageQueueId_t mq_read = NULL;
static osThreadId_t thread_send_self = NULL;

/* Exported functions --------------------------------------------------------*/
/**
  * @brief  Define test group of device core
  */
TEST_GROUP(dev_serial);

/**
  * @brief  Define test fixture setup function of device core
  */
TEST_SETUP(dev_serial)
{
    srand(time(0));

    mq_read = osMessageQueueNew((UT_DEVICE_BUFF_SIZE * 2), 1, NULL);
    TEST_ASSERT_NOT_NULL(mq_read);
}

/**
  * @brief  Define test fixture tear down function of device core
  */
TEST_TEAR_DOWN(dev_serial)
{
    osMessageQueueDelete(mq_read);
}

/**
  * @brief  Register and unregister functions in device framework.
  */
TEST(dev_serial, register_unregister)
{
    char dev_name[16];
    elab_serial_t *serial = NULL;
    elab_device_t *dev_find = NULL;
    elab_serial_attr_t config = (elab_serial_attr_t)ELAB_SERIAL_ATTR_DEFAULT;
    uint32_t count_num_init = elab_device_get_number();
    
    /* Register */
    for (uint32_t i = 0; i < UT_DEVICE_CORE_MAX; i ++)
    {
        TEST_ASSERT_EQUAL_UINT32((i + count_num_init), elab_device_get_number());

        memset(dev_name, 0, sizeof(dev_name));
        sprintf(dev_name, "serial_name_%u", i);
        serial = elab_malloc(sizeof(elab_serial_t));
        TEST_ASSERT_NOT_NULL(serial);
        elab_serial_register(serial, dev_name, &serial_ops, &config, NULL);
        dev_find = elab_device_find(dev_name);
        TEST_ASSERT_NOT_NULL(dev_find);
        TEST_ASSERT_EQUAL_PTR(serial, dev_find);

        TEST_ASSERT_EQUAL_UINT32((i + 1 + count_num_init), elab_device_get_number());
    }

    /* Unregister */
    elab_device_t *dev_free = NULL;
    for (uint32_t i = 0; i < UT_DEVICE_CORE_MAX; i ++)
    {
        TEST_ASSERT_EQUAL_UINT32((UT_DEVICE_CORE_MAX - i + count_num_init),
                                    elab_device_get_number());

        memset(dev_name, 0, sizeof(dev_name));
        sprintf(dev_name, "serial_name_%u", i);
        dev_find = elab_device_find(dev_name);
        TEST_ASSERT_NOT_NULL(dev_find);
        dev_free = dev_find;
        elab_serial_unregister(ELAB_SERIAL_CAST(dev_find));
        dev_find = elab_device_find(dev_name);
        TEST_ASSERT_NULL(dev_find);

        TEST_ASSERT_EQUAL_UINT32((UT_DEVICE_CORE_MAX - i - 1 + count_num_init),
                                    elab_device_get_number());

        elab_free(dev_free);
    }

    TEST_ASSERT_EQUAL_UINT32(count_num_init, elab_device_get_number());

    serial = elab_malloc(sizeof(elab_serial_t));
    TEST_ASSERT_NOT_NULL(serial);

    /* Register RS485 mode serial device. */
    config.mode = ELAB_SERIAL_MODE_HALF_DUPLEX;
    elab_serial_register(serial, dev_name, &serial_ops, &config, NULL);
    TEST_ASSERT_FALSE(elab_device_is_sole(ELAB_DEVICE_CAST(serial)));
    dev_find = elab_device_find(dev_name);
    TEST_ASSERT_NOT_NULL(dev_find);
    TEST_ASSERT_EQUAL_PTR(serial, dev_find);
    elab_serial_unregister(ELAB_SERIAL_CAST(dev_find));
    dev_find = elab_device_find(dev_name);
    TEST_ASSERT_NULL(dev_find);

    /* Register non-RS485 mode serial device. */
    config.mode = ELAB_SERIAL_MODE_FULL_DUPLEX;
    elab_serial_register(serial, dev_name, &serial_ops, &config, NULL);
    TEST_ASSERT_TRUE(elab_device_is_sole(ELAB_DEVICE_CAST(serial)));
    dev_find = elab_device_find(dev_name);
    TEST_ASSERT_NOT_NULL(dev_find);
    TEST_ASSERT_EQUAL_PTR(serial, dev_find);
    elab_serial_unregister(ELAB_SERIAL_CAST(dev_find));
    dev_find = elab_device_find(dev_name);
    TEST_ASSERT_NULL(dev_find);

    elab_free(serial);
    serial = NULL;

    TEST_ASSERT_EQUAL_UINT32(count_num_init, elab_device_get_number());
}

/**
  * @brief  Open & close functions for RS485 mode device.
  */
TEST(dev_serial, open_close_rs485)
{
    elab_serial_t *serial = NULL;
    elab_device_t *dev_find = NULL;
    elab_serial_attr_t config = (elab_serial_attr_t)ELAB_SERIAL_ATTR_DEFAULT;
    uint32_t count_num_init = elab_device_get_number();

    serial = elab_malloc(sizeof(elab_serial_t));
    TEST_ASSERT_NOT_NULL(serial);

    /* Register RS485 mode serial device. */
    count_open = 0;
    config.mode = ELAB_SERIAL_MODE_HALF_DUPLEX;
    elab_serial_register(serial, UT_SERIAL_NAME, &serial_ops, &config, NULL);
    TEST_ASSERT_FALSE(elab_device_is_sole(ELAB_DEVICE_CAST(serial)));
    dev_find = elab_device_find(UT_SERIAL_NAME);
    TEST_ASSERT_NOT_NULL(dev_find);
    TEST_ASSERT_EQUAL_PTR(serial, dev_find);
    for (uint32_t i = 0; i < UT_OPEN_TIMES_MAX; i ++)
    {
        TEST_ASSERT_EQUAL_INT32(0, count_open);
        elab_device_open(dev_find);
        TEST_ASSERT_EQUAL_INT32(1, count_open);
        TEST_ASSERT_TRUE(serial_is_open);
        elab_device_close(dev_find);
        TEST_ASSERT_FALSE(serial_is_open);
        TEST_ASSERT_EQUAL_INT32(0, count_open);
    }

    TEST_ASSERT_FALSE(serial_is_open);
    TEST_ASSERT_EQUAL_INT32(0, count_open);
    for (uint32_t i = 0; i < UT_OPEN_TIMES_MAX; i ++)
    {
        elab_device_open(dev_find);
        TEST_ASSERT_EQUAL_INT32(1, count_open);
        TEST_ASSERT_TRUE(serial_is_open);
    }
    for (uint32_t i = 0; i < UT_OPEN_TIMES_MAX; i ++)
    {
        TEST_ASSERT_TRUE(serial_is_open);
        TEST_ASSERT_EQUAL_INT32(1, count_open);
        elab_device_close(dev_find);
    }
    TEST_ASSERT_EQUAL_INT32(0, count_open);
    TEST_ASSERT_FALSE(serial_is_open);

    elab_serial_unregister(ELAB_SERIAL_CAST(dev_find));
    dev_find = elab_device_find(UT_SERIAL_NAME);
    TEST_ASSERT_NULL(dev_find);

    elab_free(serial);
    serial = NULL;

    TEST_ASSERT_EQUAL_UINT32(count_num_init, elab_device_get_number());
}

/**
  * @brief  Open & close functions for non-RS485-mode device.
  */
TEST(dev_serial, open_close_non_rs485)
{
    elab_serial_t *serial = NULL;
    elab_device_t *dev_find = NULL;
    elab_serial_attr_t config = (elab_serial_attr_t)ELAB_SERIAL_ATTR_DEFAULT;
    uint32_t count_num_init = elab_device_get_number();

    serial = elab_malloc(sizeof(elab_serial_t));
    TEST_ASSERT_NOT_NULL(serial);

    /* Register RS485 mode serial device. */
    config.mode = ELAB_SERIAL_MODE_FULL_DUPLEX;
    elab_serial_register(serial, UT_SERIAL_NAME, &serial_ops, &config, NULL);
    TEST_ASSERT_TRUE(elab_device_is_sole(ELAB_DEVICE_CAST(serial)));
    dev_find = elab_device_find(UT_SERIAL_NAME);
    TEST_ASSERT_NOT_NULL(dev_find);
    TEST_ASSERT_EQUAL_PTR(serial, dev_find);
    
    for (uint32_t i = 0; i < UT_OPEN_TIMES_MAX; i ++)
    {
        TEST_ASSERT_EQUAL_INT32(0, count_open);
        elab_device_open(dev_find);
        TEST_ASSERT_EQUAL_INT32(1, count_open);
        TEST_ASSERT_TRUE(serial_is_open);
        elab_device_close(dev_find);
        TEST_ASSERT_EQUAL_INT32(0, count_open);
        TEST_ASSERT_FALSE(serial_is_open);
    }

    elab_serial_unregister(ELAB_SERIAL_CAST(dev_find));
    dev_find = elab_device_find(UT_SERIAL_NAME);
    TEST_ASSERT_NULL(dev_find);

    elab_free(serial);
    serial = NULL;

    TEST_ASSERT_EQUAL_UINT32(count_num_init, elab_device_get_number());
}

/**
  * @brief  Read function in non-test mode.
  */
TEST(dev_serial, read_non_test_mode)
{
    int32_t ret = 0;
    elab_serial_t *serial = NULL;
    elab_device_t *dev = NULL;
    elab_serial_attr_t config = (elab_serial_attr_t)ELAB_SERIAL_ATTR_DEFAULT;
    uint32_t count_num_init = elab_device_get_number();
    uint32_t time_start = 0;

    serial = elab_malloc(sizeof(elab_serial_t));
    TEST_ASSERT_NOT_NULL(serial);

    /* Register non-RS485 mode serial device. */
    config.mode = ELAB_SERIAL_MODE_FULL_DUPLEX;
    elab_serial_register(serial, UT_SERIAL_NAME, &serial_ops, &config, NULL);
    TEST_ASSERT_TRUE(elab_device_is_sole(ELAB_DEVICE_CAST(serial)));
    dev = elab_device_find(UT_SERIAL_NAME);
    TEST_ASSERT_NOT_NULL(dev);
    TEST_ASSERT_EQUAL_PTR(serial, dev);

    /* Read after opening, using platform device read interface. */
    elab_device_open(dev);
    for (uint32_t i = 0; i < 100; i ++)
    {
        memset(buff_rd, 0, UT_DEVICE_BUFF_SIZE);
#if !defined(__linux__) && !defined(_WIN32)
        dev_serial_isr_rx(ELAB_SERIAL_CAST(dev), UT_STR_READ, strlen(UT_STR_READ));
#else
        make_data_for_reading();
#endif
        ret = elab_device_read(dev, 0, buff_rd, strlen(UT_STR_READ));
        TEST_ASSERT_EQUAL_INT32(strlen(UT_STR_READ), ret);
        TEST_ASSERT_EQUAL_MEMORY(UT_STR_READ, buff_rd, ret);
    }
    elab_device_close(dev);

    /* Read after opening, using serial device read interface. */
    elab_device_open(dev);
    for (uint32_t i = 0; i < 100; i ++)
    {
        memset(buff_rd, 0, UT_DEVICE_BUFF_SIZE);
#if !defined(__linux__) && !defined(_WIN32)
        dev_serial_isr_rx(ELAB_SERIAL_CAST(dev), UT_STR_READ, strlen(UT_STR_READ));
#else
        make_data_for_reading();
#endif
        time_start = osKernelGetTickCount();
        ret = elab_serial_read(dev, buff_rd, UT_DEVICE_BUFF_SIZE, 10);
        TEST_ASSERT_EQUAL_INT32(strlen(UT_STR_READ), ret);
        TEST_ASSERT_EQUAL_MEMORY(UT_STR_READ, buff_rd, ret);
        TEST_ASSERT_GREATER_OR_EQUAL_UINT32((time_start + 10),
                                                osKernelGetTickCount());
    }
    for (uint32_t i = 0; i < 100; i ++)
    {
        memset(buff_rd, 0, UT_DEVICE_BUFF_SIZE);
#if !defined(__linux__) && !defined(_WIN32)
        dev_serial_isr_rx(ELAB_SERIAL_CAST(dev), UT_STR_READ, strlen(UT_STR_READ));
#else
        make_data_for_reading();
#endif
        time_start = osKernelGetTickCount();
        ret = elab_serial_read(dev, buff_rd, strlen(UT_STR_READ), 10);
        TEST_ASSERT_EQUAL_INT32(strlen(UT_STR_READ), ret);
        TEST_ASSERT_EQUAL_MEMORY(UT_STR_READ, buff_rd, ret);
        TEST_ASSERT_LESS_THAN_UINT32((time_start + 10), osKernelGetTickCount());
    }
    for (uint32_t i = 0; i < 100; i ++)
    {
        memset(buff_rd, 0, UT_DEVICE_BUFF_SIZE);
        time_start = osKernelGetTickCount();
        ret = elab_serial_read(dev, buff_rd, UT_DEVICE_BUFF_SIZE, 10);
        TEST_ASSERT_EQUAL_INT32(ELAB_ERR_TIMEOUT, ret);
        for (uint32_t j = 0; j < UT_DEVICE_BUFF_SIZE; j ++)
        {
            TEST_ASSERT_EQUAL_UINT8(0, buff_rd[j]);
        }
        TEST_ASSERT_GREATER_OR_EQUAL_UINT32((time_start + 10),
                                                osKernelGetTickCount());
    }
    elab_device_close(dev);

    /* Read after opening, using serial device read interface. */
#if !defined(__linux__) && !defined(_WIN32)
    elab_device_open(dev);
    for (uint32_t i = 0; i < 100; i ++)
    {
        memset(buff_rd, 0, UT_DEVICE_BUFF_SIZE);
        time_start = osKernelGetTickCount();
        ret = elab_serial_read(dev, buff_rd, UT_DEVICE_BUFF_SIZE, 10);
        TEST_ASSERT_EQUAL_INT32(0, ret);
        TEST_ASSERT_GREATER_OR_EQUAL_UINT32((time_start + 10),
                                                osKernelGetTickCount());
    }
    for (uint32_t i = 0; i < 100; i ++)
    {
        memset(buff_rd, 0, UT_DEVICE_BUFF_SIZE);
        time_start = osKernelGetTickCount();
        ret = elab_serial_read(dev, buff_rd, strlen(UT_STR_READ), 10);
        TEST_ASSERT_EQUAL_INT32(0, ret);
        TEST_ASSERT_LESS_THAN_UINT32((time_start + 10), osKernelGetTickCount());
    }
    elab_device_close(dev);
#endif

    elab_serial_unregister(ELAB_SERIAL_CAST(dev));
    dev = elab_device_find(UT_SERIAL_NAME);
    TEST_ASSERT_NULL(dev);
    elab_free(serial);

    TEST_ASSERT_EQUAL_UINT32(count_num_init, elab_device_get_number());
}

/**
  * @brief  Write function in non-test mode.
  */
TEST(dev_serial, write_non_test_mode_non_rs485)
{
    int32_t ret = 0;
    elab_serial_t *serial = NULL;
    elab_device_t *dev = NULL;
    elab_serial_attr_t config = (elab_serial_attr_t)ELAB_SERIAL_ATTR_DEFAULT;
    uint32_t count_num_init = elab_device_get_number();

    serial = elab_malloc(sizeof(elab_serial_t));
    TEST_ASSERT_NOT_NULL(serial);

    /* Register RS485 mode serial device. */
    config.mode = ELAB_SERIAL_MODE_FULL_DUPLEX;
    elab_serial_register(serial, UT_SERIAL_NAME, &serial_ops, &config, NULL);
    TEST_ASSERT_TRUE(elab_device_is_sole(ELAB_DEVICE_CAST(serial)));
    dev = elab_device_find(UT_SERIAL_NAME);
    TEST_ASSERT_NOT_NULL(dev);
    TEST_ASSERT_EQUAL_PTR(serial, dev);

    /* Write after opening. */
    elab_device_open(dev);
    TEST_ASSERT_TRUE(elab_device_is_enabled(ELAB_DEVICE_CAST(serial)));
    memset(buff_wr, 0, UT_DEVICE_BUFF_SIZE);
    inform_serial_tx_end_delay(serial);
    ret = elab_device_write(dev, 0, UT_STR_WRITE, strlen(UT_STR_WRITE));
    TEST_ASSERT_EQUAL_INT32(strlen(UT_STR_WRITE), ret);
    TEST_ASSERT_EQUAL_MEMORY(UT_STR_WRITE, buff_wr, ret);
    elab_device_close(dev);

    /* Write after opening. */
    count_set_tx = 0;
    elab_device_open(dev);
    for (uint32_t i = 0; i < 100; i ++)
    {
        memset(buff_wr, 0, UT_DEVICE_BUFF_SIZE);
        TEST_ASSERT_EQUAL_UINT32(0, count_set_tx);
        inform_serial_tx_end_delay(serial);
        ret = elab_device_write(dev, 0, UT_STR_WRITE, strlen(UT_STR_WRITE));
        TEST_ASSERT_EQUAL_UINT32(0, count_set_tx);
        TEST_ASSERT_EQUAL_INT32(strlen(UT_STR_WRITE), ret);
        TEST_ASSERT_EQUAL_MEMORY(UT_STR_WRITE, buff_wr, ret);
    }
    elab_device_close(dev);

    elab_serial_unregister(ELAB_SERIAL_CAST(dev));
    dev = elab_device_find(UT_SERIAL_NAME);
    TEST_ASSERT_NULL(dev);
    elab_free(serial);

    TEST_ASSERT_EQUAL_UINT32(count_num_init, elab_device_get_number());
}

/**
  * @brief  Write function in non-test mode.
  */
TEST(dev_serial, write_non_test_mode_rs485)
{
    int32_t ret = 0;
    elab_serial_t *serial = NULL;
    elab_device_t *dev = NULL;
    elab_serial_attr_t config = (elab_serial_attr_t)ELAB_SERIAL_ATTR_DEFAULT;
    uint32_t count_num_init = elab_device_get_number();

    serial = elab_malloc(sizeof(elab_serial_t));
    TEST_ASSERT_NOT_NULL(serial);

    /* Register RS485 mode serial device. */
    config.mode = ELAB_SERIAL_MODE_HALF_DUPLEX;
    elab_serial_register(serial, UT_SERIAL_NAME, &serial_ops, &config, NULL);
    TEST_ASSERT_FALSE(elab_device_is_sole(ELAB_DEVICE_CAST(serial)));
    dev = elab_device_find(UT_SERIAL_NAME);
    TEST_ASSERT_NOT_NULL(dev);
    TEST_ASSERT_EQUAL_PTR(serial, dev);

    /* Write after opening. */
    elab_device_open(dev);
    TEST_ASSERT_TRUE(elab_device_is_enabled(ELAB_DEVICE_CAST(serial)));
    memset(buff_wr, 0, UT_DEVICE_BUFF_SIZE);
    inform_serial_tx_end_delay(serial);
    ret = elab_device_write(dev, 0, UT_STR_WRITE, strlen(UT_STR_WRITE));
    TEST_ASSERT_EQUAL_INT32(strlen(UT_STR_WRITE), ret);
    TEST_ASSERT_EQUAL_MEMORY(UT_STR_WRITE, buff_wr, ret);
    elab_device_close(dev);

    /* Write after opening. */
    elab_device_open(dev);
    for (uint32_t i = 0; i < 100; i ++)
    {
        memset(buff_wr, 0, UT_DEVICE_BUFF_SIZE);
        TEST_ASSERT_EQUAL_UINT32(i, count_set_tx);
        inform_serial_tx_end_delay(serial);
        ret = elab_device_write(dev, 0, UT_STR_WRITE, strlen(UT_STR_WRITE));
        TEST_ASSERT_EQUAL_UINT32((i + 1), count_set_tx);
        TEST_ASSERT_EQUAL_INT32(strlen(UT_STR_WRITE), ret);
        TEST_ASSERT_EQUAL_MEMORY(UT_STR_WRITE, buff_wr, ret);
    }
    elab_device_close(dev);

    elab_serial_unregister(ELAB_SERIAL_CAST(dev));
    dev = elab_device_find(UT_SERIAL_NAME);
    TEST_ASSERT_NULL(dev);
    elab_free(serial);

    TEST_ASSERT_EQUAL_UINT32(count_num_init, elab_device_get_number());
}

/**
  * @brief  Write function in non-test mode.
  */
TEST(dev_serial, xfer_non_test_mode)
{
    int32_t ret = 0;
    elab_serial_t *serial = NULL;
    elab_device_t *dev = NULL;
    elab_serial_attr_t config = (elab_serial_attr_t)ELAB_SERIAL_ATTR_DEFAULT;
    uint32_t count_num_init = elab_device_get_number();
    uint32_t time_start = 0;

    serial = elab_malloc(sizeof(elab_serial_t));
    TEST_ASSERT_NOT_NULL(serial);

    /* Register RS485 mode serial device. */
    config.mode = ELAB_SERIAL_MODE_HALF_DUPLEX;
    elab_serial_register(serial, UT_SERIAL_NAME, &serial_ops, &config, NULL);
    TEST_ASSERT_FALSE(elab_device_is_sole(ELAB_DEVICE_CAST(serial)));
    dev = elab_device_find(UT_SERIAL_NAME);
    TEST_ASSERT_NOT_NULL(dev);
    TEST_ASSERT_EQUAL_PTR(serial, dev);

    /* Xfer after opening. */
    elab_device_open(dev);
    for (uint32_t i = 0; i < 100; i ++)
    {
        memset(buff_wr, 0, UT_DEVICE_BUFF_SIZE);
        memset(buff_rd, 0, UT_DEVICE_BUFF_SIZE);
#if !defined(__linux__) && !defined(_WIN32)
        dev_serial_isr_rx(ELAB_SERIAL_CAST(dev), UT_STR_READ, strlen(UT_STR_READ));
#else
        make_data_for_reading();
#endif
        inform_serial_tx_end_delay(serial);
        time_start = osKernelGetTickCount();
        ret = elab_serial_xfer(dev, UT_STR_WRITE, strlen(UT_STR_WRITE),
                                    buff_rd, UT_DEVICE_BUFF_SIZE, 20);
        TEST_ASSERT_GREATER_OR_EQUAL_UINT32(
                                (time_start + 20), osKernelGetTickCount());
        TEST_ASSERT_EQUAL_INT32(strlen(UT_STR_READ), ret);
        TEST_ASSERT_EQUAL_MEMORY(UT_STR_READ, buff_rd, ret);
    }
    elab_device_close(dev);

#if !defined(__linux__) && !defined(_WIN32)
    /* Xfer after opening. */
    elab_device_open(dev);
    for (uint32_t i = 0; i < 100; i ++)
    {
        memset(buff_wr, 0, UT_DEVICE_BUFF_SIZE);
        memset(buff_rd, 0, UT_DEVICE_BUFF_SIZE);
        time_start = osKernelGetTickCount();
        ret = dev_serial_xfer(dev, UT_STR_WRITE, strlen(UT_STR_WRITE),
                                    buff_rd, UT_DEVICE_BUFF_SIZE, 10);
        TEST_ASSERT_EQUAL_INT32(strlen(UT_STR_WRITE), ret);
        TEST_ASSERT_EQUAL_MEMORY(UT_STR_WRITE, buff_wr, ret);
        TEST_ASSERT_EQUAL_INT32(0, ret);
        TEST_ASSERT_GREATER_OR_EQUAL_UINT32((time_start + 10),
                                                osKernelGetTickCount());
    }
    elab_device_close(dev);
#endif

    elab_serial_unregister(ELAB_SERIAL_CAST(dev));
    dev = elab_device_find(UT_SERIAL_NAME);
    TEST_ASSERT_NULL(dev);
    elab_free(serial);

    TEST_ASSERT_EQUAL_UINT32(count_num_init, elab_device_get_number());
}


/**
  * @brief  Control function in non-test mode.
  */
TEST(dev_serial, send_to_self_non_test_mode)
{
    int32_t ret = 0;
    osStatus_t ret_os = osOK;
    elab_serial_t *serial = NULL;
    elab_device_t *dev = NULL;
    elab_serial_attr_t config = (elab_serial_attr_t)ELAB_SERIAL_ATTR_DEFAULT;
    uint32_t count_num_init = elab_device_get_number();
    uint32_t time_start = 0;
    uint8_t ch = 'A';

    serial = elab_malloc(sizeof(elab_serial_t));
    TEST_ASSERT_NOT_NULL(serial);
    sem_send_self = osSemaphoreNew(1, 0, NULL);
    TEST_ASSERT_NOT_NULL(sem_send_self);

    /* Register RS485 mode serial device. */
    config.mode = ELAB_SERIAL_MODE_FULL_DUPLEX;
    elab_serial_register(serial, UT_SERIAL_NAME, &serial_ops_send_self, &config, NULL);
    TEST_ASSERT_TRUE(elab_device_is_sole(ELAB_DEVICE_CAST(serial)));
    dev = elab_device_find(UT_SERIAL_NAME);
    TEST_ASSERT_NOT_NULL(dev);
    TEST_ASSERT_EQUAL_PTR(serial, dev);

    /* Generate the random tx buffer data. */
    for (uint32_t i = 0; i < UT_DEVICE_BUFF_SIZE; i ++)
    {
        buff_wr[i] = (uint8_t)i;
    }

    /* Xfer after opening. */
    elab_device_open(dev);
    thread_send_self = osThreadNew(entry_send_self, serial, &attr_serial_read);
    TEST_ASSERT_NOT_NULL(thread_send_self);
    for (uint32_t i = 0; i < 1000; i ++)
    {
        count_rd = 0;
        ret = elab_serial_write(dev, buff_wr, UT_DEVICE_BUFF_SIZE);
        TEST_ASSERT_EQUAL_INT32(UT_DEVICE_BUFF_SIZE, ret);
        osSemaphoreAcquire(sem_send_self, osWaitForever);
        TEST_ASSERT_EQUAL_UINT32(UT_DEVICE_BUFF_SIZE, count_rd);
    }
    ret_os = osThreadTerminate(thread_send_self);
    TEST_ASSERT(ret_os == osOK);
    ret_os = osSemaphoreDelete(sem_send_self);
    TEST_ASSERT(ret_os == osOK);
    elab_device_close(dev);

    elab_serial_unregister(ELAB_SERIAL_CAST(dev));
    dev = elab_device_find(UT_SERIAL_NAME);
    TEST_ASSERT_NULL(dev);
    elab_free(serial);

    TEST_ASSERT_EQUAL_UINT32(count_num_init, elab_device_get_number());
}

/**
  * @brief  Control function in non-test mode.
  */
TEST(dev_serial, set_config_non_test_mode)
{
    int32_t ret = 0;
    elab_serial_t *serial = NULL;
    elab_device_t *dev = NULL;
    elab_serial_attr_t config_get;
    elab_serial_attr_t config = (elab_serial_attr_t)ELAB_SERIAL_ATTR_DEFAULT;
    uint32_t count_num_init = elab_device_get_number();

    serial = elab_malloc(sizeof(elab_serial_t));
    TEST_ASSERT_NOT_NULL(serial);

    /* Register RS485 mode serial device. */
    config.mode = ELAB_SERIAL_MODE_FULL_DUPLEX;
    elab_serial_register(serial, UT_SERIAL_NAME, &serial_ops, &config, NULL);
    TEST_ASSERT_TRUE(elab_device_is_sole(ELAB_DEVICE_CAST(serial)));
    dev = elab_device_find(UT_SERIAL_NAME);
    TEST_ASSERT_NOT_NULL(dev);
    TEST_ASSERT_EQUAL_PTR(serial, dev);

    /* Set config after opening. */
    elab_device_open(dev);
    elab_serial_set_attr(dev, &config);
    TEST_ASSERT_EQUAL_MEMORY(&config, &config_set, sizeof(elab_serial_attr_t));
    config_get = elab_serial_get_attr(dev);
    TEST_ASSERT_EQUAL_MEMORY(&config, &config_get, sizeof(elab_serial_attr_t));
    elab_device_close(dev);

    /* Set baudrate after opening. */
    elab_device_open(dev);
    config.baud_rate = 115200;
    elab_serial_set_baudrate(dev, 115200);
    TEST_ASSERT_EQUAL_MEMORY(&config, &config_set, sizeof(elab_serial_attr_t));
    config_get = elab_serial_get_attr(dev);
    TEST_ASSERT_EQUAL_MEMORY(&config, &config_get, sizeof(elab_serial_attr_t));
    elab_device_close(dev);

    elab_serial_unregister(ELAB_SERIAL_CAST(dev));
    dev = elab_device_find(UT_SERIAL_NAME);
    TEST_ASSERT_NULL(dev);
    elab_free(serial);

    TEST_ASSERT_EQUAL_UINT32(count_num_init, elab_device_get_number());
}

/**
  * @brief  Read function in test mode.
  */
TEST(dev_serial, read_test_mode)
{
    int32_t ret = 0;
    osStatus_t ret_os = osOK;
    elab_serial_t *serial = NULL;
    elab_device_t *dev = NULL;
    elab_serial_attr_t config = (elab_serial_attr_t)ELAB_SERIAL_ATTR_DEFAULT;
    uint32_t count_num_init = elab_device_get_number();
    uint32_t time_start = 0;

    serial = elab_malloc(sizeof(elab_serial_t));
    TEST_ASSERT_NOT_NULL(serial);
    sem_test_mode_enter = osSemaphoreNew(1, 0, NULL);
    TEST_ASSERT_NOT_NULL(sem_test_mode_enter);
    sem_test_mode_exit = osSemaphoreNew(1, 0, NULL);
    TEST_ASSERT_NOT_NULL(sem_test_mode_exit);

    /* Register non-RS485 mode serial device. */
    config.mode = ELAB_SERIAL_MODE_FULL_DUPLEX;
    elab_serial_register(serial, UT_SERIAL_NAME, &serial_ops, &config, NULL);
    TEST_ASSERT_TRUE(elab_device_is_sole(ELAB_DEVICE_CAST(serial)));
    dev = elab_device_find(UT_SERIAL_NAME);
    TEST_ASSERT_NOT_NULL(dev);
    TEST_ASSERT_EQUAL_PTR(serial, dev);

    memset(buff_rd, 0, UT_DEVICE_BUFF_SIZE);
    elab_device_open(dev);

    /* Read after opening, using platform device read interface. */
    for (uint32_t c = 0; c < 100; c ++)
    {
        memset(buff_rd, 0, UT_DEVICE_BUFF_SIZE);
#if !defined(__linux__) && !defined(_WIN32)
        dev_serial_isr_rx(ELAB_SERIAL_CAST(dev), UT_STR_READ, strlen(UT_STR_READ));
#else
        make_data_for_reading();
#endif
        ret = elab_serial_read(dev, buff_rd, strlen(UT_STR_READ), osWaitForever);
        TEST_ASSERT_EQUAL_INT32(strlen(UT_STR_READ), ret);
        TEST_ASSERT_EQUAL_MEMORY(UT_STR_READ, buff_rd, ret);
    }
    
    /* Read the data in the specific newly-created thread. */
    for (uint32_t c = 0; c < 100; c ++)
    {
        memset(buff_rd, 0, UT_DEVICE_BUFF_SIZE);
#if !defined(__linux__) && !defined(_WIN32)
        dev_serial_isr_rx(ELAB_SERIAL_CAST(dev), UT_STR_READ, strlen(UT_STR_READ));
#else
        make_data_for_reading();
#endif
        osThreadNew(thread_func_read, dev, &thread_attr_dev_test);
        ret_os = osSemaphoreRelease(sem_test_mode_enter);
        elab_assert(ret_os == osOK);
        ret_os = osSemaphoreAcquire(sem_test_mode_exit, osWaitForever);
        elab_assert(ret_os == osOK);
        TEST_ASSERT_EQUAL_INT32(ELAB_ERR_TIMEOUT, ret_read_test);
        for (uint32_t i = 0; i < UT_DEVICE_BUFF_SIZE; i ++)
        {
            TEST_ASSERT_EQUAL_UINT32(0, buff_rd[i]);
        }
        time_start = osKernelGetTickCount();
        ret = elab_serial_read(dev, buff_rd, UT_DEVICE_BUFF_SIZE, 10);
        TEST_ASSERT_GREATER_OR_EQUAL_UINT32(
                                (time_start + 10), osKernelGetTickCount());
        TEST_ASSERT_EQUAL_INT32(strlen(UT_STR_READ), ret);
        TEST_ASSERT_EQUAL_MEMORY(UT_STR_READ, buff_rd, ret);
    }

    /* Read after opening, using platform device read interface. */
    for (uint32_t c = 0; c < 100; c ++)
    {
        memset(buff_rd, 0, UT_DEVICE_BUFF_SIZE);
#if !defined(__linux__) && !defined(_WIN32)
        dev_serial_isr_rx(ELAB_SERIAL_CAST(dev), UT_STR_READ, strlen(UT_STR_READ));
#else
        make_data_for_reading();
#endif
        ret = elab_device_read(dev, 0, buff_rd, strlen(UT_STR_READ));
        TEST_ASSERT_EQUAL_INT32(strlen(UT_STR_READ), ret);
        TEST_ASSERT_EQUAL_MEMORY(UT_STR_READ, buff_rd, ret);
    }

    elab_device_close(dev);
    elab_serial_unregister(ELAB_SERIAL_CAST(dev));
    dev = elab_device_find(UT_SERIAL_NAME);
    TEST_ASSERT_NULL(dev);
    elab_free(serial);
    ret_os = osSemaphoreDelete(sem_test_mode_enter);
    elab_assert(ret_os == osOK);
    ret_os = osSemaphoreDelete(sem_test_mode_exit);
    elab_assert(ret_os == osOK);

    TEST_ASSERT_EQUAL_UINT32(count_num_init, elab_device_get_number());
}

/**
  * @brief  Write function in test mode.
  */
TEST(dev_serial, write_test_mode)
{
    int32_t ret = 0;
    osStatus_t ret_os = osOK;
    elab_serial_t *serial = NULL;
    elab_device_t *dev = NULL;
    elab_serial_attr_t config = (elab_serial_attr_t)ELAB_SERIAL_ATTR_DEFAULT;
    uint32_t count_num_init = elab_device_get_number();

    serial = elab_malloc(sizeof(elab_serial_t));
    TEST_ASSERT_NOT_NULL(serial);
    sem_test_mode_enter = osSemaphoreNew(1, 0, NULL);
    TEST_ASSERT_NOT_NULL(sem_test_mode_enter);
    sem_test_mode_exit = osSemaphoreNew(1, 0, NULL);
    TEST_ASSERT_NOT_NULL(sem_test_mode_exit);

    /* Register non-RS485 mode serial device. */
    config.mode = ELAB_SERIAL_MODE_FULL_DUPLEX;
    elab_serial_register(serial, UT_SERIAL_NAME, &serial_ops, &config, NULL);
    TEST_ASSERT_TRUE(elab_device_is_sole(ELAB_DEVICE_CAST(serial)));
    dev = elab_device_find(UT_SERIAL_NAME);
    TEST_ASSERT_NOT_NULL(dev);
    TEST_ASSERT_EQUAL_PTR(serial, dev);
    memset(buff_rd, 0, UT_DEVICE_BUFF_SIZE);
    elab_device_open(dev);

    /* Write after opening. */
    memset(buff_wr, 0, UT_DEVICE_BUFF_SIZE);
    inform_serial_tx_end_delay(serial);
    ret = elab_serial_write(dev, UT_STR_WRITE, strlen(UT_STR_WRITE));
    TEST_ASSERT_EQUAL_INT32(strlen(UT_STR_WRITE), ret);
    TEST_ASSERT_EQUAL_MEMORY(UT_STR_WRITE, buff_wr, ret);
    ret_os = osSemaphoreRelease(sem_test_mode_enter);
    elab_assert(ret_os == osOK);

    /* Write the data in the specific thread. */
    for (uint32_t c = 0; c < 100; c ++)
    {
        memset(buff_wr, 0, UT_DEVICE_BUFF_SIZE);
        osThreadNew(thread_func_write, dev, &thread_attr_dev_test);
        ret_os = osSemaphoreRelease(sem_test_mode_enter);
        elab_assert(ret_os == osOK);
        ret_os = osSemaphoreAcquire(sem_test_mode_exit, osWaitForever);
        elab_assert(ret_os == osOK);
        TEST_ASSERT_EQUAL_INT32(0, ret_write_test);
        for (uint32_t i = 0; i < UT_DEVICE_BUFF_SIZE; i ++)
        {
            TEST_ASSERT_EQUAL_UINT32(0, buff_wr[i]);
        }
    }

    elab_device_close(dev);
    elab_serial_unregister(ELAB_SERIAL_CAST(dev));
    dev = elab_device_find(UT_SERIAL_NAME);
    TEST_ASSERT_NULL(dev);
    elab_free(serial);
    ret_os = osSemaphoreDelete(sem_test_mode_enter);
    elab_assert(ret_os == osOK);
    ret_os = osSemaphoreDelete(sem_test_mode_exit);
    elab_assert(ret_os == osOK);

    TEST_ASSERT_EQUAL_UINT32(count_num_init, elab_device_get_number());
}

/**
  * @brief  Write function in test mode.
  */
TEST(dev_serial, config_test_mode)
{
    int32_t ret = 0;
    osStatus_t ret_os = osOK;
    elab_serial_t *serial = NULL;
    elab_device_t *dev = NULL;
    elab_serial_attr_t config = (elab_serial_attr_t)ELAB_SERIAL_ATTR_DEFAULT;
    elab_serial_attr_t config_get;
    uint32_t count_num_init = elab_device_get_number();

    serial = elab_malloc(sizeof(elab_serial_t));
    TEST_ASSERT_NOT_NULL(serial);
    sem_test_mode_enter = osSemaphoreNew(1, 0, NULL);
    TEST_ASSERT_NOT_NULL(sem_test_mode_enter);
    sem_test_mode_exit = osSemaphoreNew(1, 0, NULL);
    TEST_ASSERT_NOT_NULL(sem_test_mode_exit);

    /* Register non-RS485 mode serial device. */
    config.mode = ELAB_SERIAL_MODE_FULL_DUPLEX;
    elab_serial_register(serial, UT_SERIAL_NAME, &serial_ops, &config, NULL);
    TEST_ASSERT_TRUE(elab_device_is_sole(ELAB_DEVICE_CAST(serial)));
    dev = elab_device_find(UT_SERIAL_NAME);
    TEST_ASSERT_NOT_NULL(dev);
    TEST_ASSERT_EQUAL_PTR(serial, dev);

    memset(buff_rd, 0, UT_DEVICE_BUFF_SIZE);
    elab_device_open(dev);

    /* Set config after opening. */
    memset(buff_wr, 0, UT_DEVICE_BUFF_SIZE);
    config.baud_rate = 9600;
    elab_serial_set_attr(dev, &config);
    TEST_ASSERT_EQUAL_MEMORY(&config, &config_set, sizeof(elab_serial_attr_t));
    config_get = elab_serial_get_attr(dev);
    TEST_ASSERT_EQUAL_MEMORY(&config, &config_get, sizeof(elab_serial_attr_t));

    /* Set config the data in the specific thread. */
    memset(buff_wr, 0, UT_DEVICE_BUFF_SIZE);
    osThreadNew(thread_func_config, dev, &thread_attr_dev_test);
    ret_os = osSemaphoreRelease(sem_test_mode_enter);
    elab_assert(ret_os == osOK);
    ret_os = osSemaphoreAcquire(sem_test_mode_exit, osWaitForever);
    elab_assert(ret_os == osOK);
    config_get = elab_serial_get_attr(dev);
    TEST_ASSERT_EQUAL_UINT32(9600, config_get.baud_rate);
    config.baud_rate = 9600;
    TEST_ASSERT_EQUAL_MEMORY(&config, &config_get, sizeof(elab_serial_attr_t));

    elab_device_close(dev);
    elab_serial_unregister(ELAB_SERIAL_CAST(dev));
    dev = elab_device_find(UT_SERIAL_NAME);
    TEST_ASSERT_NULL(dev);
    elab_free(serial);
    ret_os = osSemaphoreDelete(sem_test_mode_enter);
    elab_assert(ret_os == osOK);
    ret_os = osSemaphoreDelete(sem_test_mode_exit);
    elab_assert(ret_os == osOK);

    TEST_ASSERT_EQUAL_UINT32(count_num_init, elab_device_get_number());
}

/**
  * @brief  Define run test cases of device core
  */
TEST_GROUP_RUNNER(dev_serial)
{
    RUN_TEST_CASE(dev_serial, register_unregister);
    RUN_TEST_CASE(dev_serial, open_close_rs485);
    RUN_TEST_CASE(dev_serial, open_close_non_rs485);
    RUN_TEST_CASE(dev_serial, read_non_test_mode);
    RUN_TEST_CASE(dev_serial, write_non_test_mode_non_rs485);
    RUN_TEST_CASE(dev_serial, write_non_test_mode_rs485);
    RUN_TEST_CASE(dev_serial, xfer_non_test_mode);
    RUN_TEST_CASE(dev_serial, send_to_self_non_test_mode);
    RUN_TEST_CASE(dev_serial, set_config_non_test_mode);
    RUN_TEST_CASE(dev_serial, read_test_mode);
    RUN_TEST_CASE(dev_serial, write_test_mode);
    RUN_TEST_CASE(dev_serial, config_test_mode);
}

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Simulated driver opening function for serial device testing.
  */
static elab_err_t ops_enable(elab_serial_t *serial, bool status)
{
    (void)serial;
    serial_is_open = status;
    if (status)
    {
        count_open ++;
    }
    else
    {
        count_open --;
    }
    count_set_tx = 0;
    return ELAB_OK;
}

#if defined(__linux__) || defined(_WIN32)
/**
  * @brief  Simulated driver reading function for serial device testing.
  */
static int32_t ops_read(elab_serial_t *serial, void *pbuf, uint32_t size)
{
    (void)serial;

    char *ch = (char *)pbuf;
    int32_t count = 0;
    for (uint32_t i = 0; i < size; i ++)
    {
        osStatus_t ret = osMessageQueueGet(mq_read, &ch[i], NULL, osWaitForever);
        elab_assert(ret == osOK);
        count ++;
    }

    return count;
}
#endif

/**
  * @brief  Simulated driver writting function for serial device testing.
  */
static int32_t ops_write(elab_serial_t *serial, const void *pbuf, uint32_t size)
{
    (void)serial;

    memcpy(buff_wr, pbuf, size);
    return size;
}

/**
  * @brief  Simulated driver writting function for serial device testing.
  */
static int32_t ops_write_to_self(elab_serial_t *serial,
                                    const void *pbuf, uint32_t size)
{
#if !defined(__linux__) && !defined(_WIN32)
    dev_serial_isr_rx(serial, pbuf, size);
#else
    for (uint32_t i = 0; i < size; i ++)
    {
        osMessageQueuePut(mq_read, &((uint8_t *)pbuf)[i], 0, osWaitForever);
    }
    elab_serial_tx_end(serial);
#endif

    return size;
}

/**
  * @brief  Simulated driver config function for serial device testing.
  */
static elab_err_t ops_config(elab_serial_t *serial, elab_serial_config_t *pcfg)
{
    (void)serial;

    memcpy(&config_set, pcfg, sizeof(elab_serial_config_t));
    return ELAB_OK;
}

/**
  * @brief  Simulated driver config function for serial device testing.
  */
static void ops_set_tx(elab_serial_t *serial, bool status)
{
    (void)serial;
    serial_set_tx = status;
    if (status)
    {
        count_set_tx ++;
    }
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
    elab_assert(ret_os == osOK);
    elab_device_set_test_mode(dev);
    ret_read_test =
        elab_serial_read(dev, buff_rd, strlen(UT_STR_READ), 5);
    /* Exit the test mode. */
    elab_device_set_normal_mode(dev);
    ret_os = osSemaphoreRelease(sem_test_mode_exit);
    elab_assert(ret_os == osOK);
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
    elab_assert(ret_os == osOK);
    elab_device_set_test_mode(dev);

    /* Write data into the device. */
    ret_write_test = elab_serial_write(dev, UT_STR_WRITE, strlen(UT_STR_WRITE));

    /* Exit the test mode. */
    elab_device_set_normal_mode(dev);
    ret_os = osSemaphoreRelease(sem_test_mode_exit);
    elab_assert(ret_os == osOK);
    
    osThreadExit();
}

/**
  * @brief  Thread function for writing data in testing mode.
  * @param  Thread parameter.
  */
static void thread_func_config(void *paras)
{
    elab_device_t *dev = (elab_device_t *)paras;
    elab_err_t ret = ELAB_OK;
    osStatus_t ret_os = osOK;

    /* Wait for testing mode entry. */
    ret_os = osSemaphoreAcquire(sem_test_mode_enter, osWaitForever);
    elab_assert(ret_os == osOK);
    elab_device_set_test_mode(dev);

    /* Serail device config. */
    elab_serial_attr_t config = (elab_serial_attr_t)ELAB_SERIAL_ATTR_DEFAULT;
    config.baud_rate = 9600;
    elab_serial_set_attr(dev, &config);

    /* Exit the test mode. */
    elab_device_set_normal_mode(dev);
    ret_os = osSemaphoreRelease(sem_test_mode_exit);
    elab_assert(ret_os == osOK);
    
    osThreadExit();
}

/**
  * @brief  Thread function for writing data in testing mode.
  * @param  Thread parameter.
  */
static void thread_func_tx_end(void *paras)
{
    elab_serial_t *serial = (elab_serial_t *)paras;

    osDelay(10);
    elab_serial_tx_end(serial);
    osThreadExit();
}

/**
  * @brief  Make data for reading.
  */
static void make_data_for_reading(void)
{
    char *ch = UT_STR_READ;
    for (uint32_t i = 0; i < strlen(UT_STR_READ); i ++)
    {
        osStatus_t ret = osMessageQueuePut(mq_read, &ch[i], 0, osWaitForever);
        elab_assert(ret == osOK);
    }
}

/**
  * @brief  Make data for reading.
  */
static void inform_serial_tx_end_delay(elab_serial_t *serial)
{
    osThreadNew(thread_func_tx_end, serial, NULL);
}

/**
  * @brief  Thread function for read send-to-self data testing.
  * @param  Thread parameter.
  */
static void entry_send_self(void *paras)
{
    elab_device_t *dev = (elab_device_t *)paras;
    int32_t ret = ELAB_OK;
    int32_t ret_bkp = ELAB_ERR_TIMEOUT;
    uint8_t ch;
    osStatus_t ret_os = osOK;

    while (1)
    {
        ret = elab_serial_read(dev, &ch, 1, 5);
        if (ret > 0)
        {
            count_rd ++;
        }
        else if (ret == ELAB_ERR_TIMEOUT && ret_bkp > 0)
        {
            ret_os = osSemaphoreRelease(sem_send_self);
        }

        ret_bkp = ret;
    }
}

/* ----------------------------- end of file -------------------------------- */
