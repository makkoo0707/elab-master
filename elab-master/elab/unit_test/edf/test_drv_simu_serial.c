/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#if defined(__linux__) || defined(_WIN32)

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "../../edf/driver/simulator/simu_serial.h"
#include "../../edf/normal/elab_serial.h"
#include "../../3rd/Unity/unity.h"
#include "../../3rd/Unity/unity_fixture.h"
#include "../../common/elab_common.h"
#include "../../common/elab_assert.h"

ELAB_TAG("ut_drv_simu_serial");
#include "../../common/elab_log.h"

/* Private config ------------------------------------------------------------*/
#define UT_SIMU_SERIAL_LOCAL_EN                     (0)
#define UT_SIMU_SERIAL_MQTT_EN                      (1)

#define UT_SIMU_SERIAL_BUFF_SIZE                    (256)
#define UT_SIMU_SERIAL_TIMES                        (100)

#define UT_STR_READ                                 "dev_read_data"
#define UT_STR_WRITE                                "dev_write_data"

#if ((UT_SIMU_SERIAL_LOCAL_EN == 0 && UT_SIMU_SERIAL_MQTT_EN == 0) ||          \
    (UT_SIMU_SERIAL_LOCAL_EN != 0 && UT_SIMU_SERIAL_MQTT_EN != 0))
    #error "Only one can be set enabled in MQTT & LOCAL."
#endif

/* Exported function prototypes ----------------------------------------------*/


/* Private function prototypes -----------------------------------------------*/
static void entry_serial_read(void *paras);

/* Private variables ---------------------------------------------------------*/
static uint8_t *buff_tx_read = NULL;
static char *dev_name_1 = NULL;
static char *dev_name_2 = NULL;
static uint8_t *buff_rx = NULL;
static uint8_t *buff_tx = NULL;
static osSemaphoreId_t sem_test_cross_thread = NULL;
static uint32_t count_rx_cross_thread = 0;

static const osThreadAttr_t attr_serial_read = 
{
    .name = "ThreadSerailRead",
    .attr_bits = osThreadDetached, 
    .priority = osPriorityRealtime,
    .stack_size = 2048,
};

/* Exported functions --------------------------------------------------------*/
/**
  * @brief  Define test group of device core
  */
TEST_GROUP(simu_serial);

/**
  * @brief  Define test fixture setup function of device core
  */
TEST_SETUP(simu_serial)
{
    srand(time(0));

    buff_tx_read = elab_malloc(UT_SIMU_SERIAL_BUFF_SIZE);
    TEST_ASSERT_NOT_NULL(buff_tx_read);

    dev_name_1 = elab_malloc(UT_SIMU_SERIAL_BUFF_SIZE);
    TEST_ASSERT_NOT_NULL(dev_name_1);

    dev_name_2 = elab_malloc(UT_SIMU_SERIAL_BUFF_SIZE);
    TEST_ASSERT_NOT_NULL(dev_name_2);

    buff_rx = elab_malloc(UT_SIMU_SERIAL_BUFF_SIZE);
    TEST_ASSERT_NOT_NULL(buff_rx);

    buff_tx = elab_malloc(UT_SIMU_SERIAL_BUFF_SIZE);
    TEST_ASSERT_NOT_NULL(buff_tx);
}

/**
  * @brief  Define test fixture tear down function of device core
  */
TEST_TEAR_DOWN(simu_serial)
{
    elab_free(buff_tx_read);
    elab_free(dev_name_1);
    elab_free(dev_name_2);
    elab_free(buff_rx);
    elab_free(buff_tx);
}

/**
  * @brief  Assert failures in device framework functions.
  */
TEST(simu_serial, new_destroy_single_mode)
{
    elab_device_t *dev = NULL;

    for (uint32_t i = 0; i < UT_SIMU_SERIAL_TIMES; i ++)
    {
        memset(dev_name_1, 0, UT_SIMU_SERIAL_BUFF_SIZE);
        sprintf(dev_name_1, "simu_serial_%u", i);
        simu_serial_new(dev_name_1, SIMU_SERIAL_MODE_SINGLE, 115200);
        dev = elab_device_find(dev_name_1);
        TEST_ASSERT_NOT_NULL(dev);
        simu_serial_destroy(dev_name_1);
        dev = elab_device_find(dev_name_1);
        TEST_ASSERT_NULL(dev);
    }
}

/**
  * @brief  Assert failures in device framework functions.
  */
TEST(simu_serial, new_destroy_pair_uart_mqtt)
{
    elab_device_t *dev = NULL;

    for (uint32_t i = 0; i < UT_SIMU_SERIAL_TIMES; i ++)
    {
        /* Create one pair of simulated serial device. */
        memset(dev_name_1, 0, UT_SIMU_SERIAL_BUFF_SIZE);
        sprintf(dev_name_1, "simu_serial_1_%u", i);
        memset(dev_name_2, 0, UT_SIMU_SERIAL_BUFF_SIZE);
        sprintf(dev_name_2, "simu_serial_2_%u", i);
#if (UT_SIMU_SERIAL_LOCAL_EN != 0)
        simu_serial_new_pair(dev_name_1, dev_name_2, 115200);
#endif
#if (UT_SIMU_SERIAL_MQTT_EN != 0)
        simu_serial_mqtt_new_pair(dev_name_1, dev_name_2, 115200);
#endif

        /* Find the simulated devices from device framework. */
        dev = elab_device_find(dev_name_1);
        TEST_ASSERT_NOT_NULL(dev);
        dev = elab_device_find(dev_name_2);
        TEST_ASSERT_NOT_NULL(dev);

        /* Destroy the simulated devices. */
        simu_serial_destroy(dev_name_1);
        simu_serial_destroy(dev_name_2);

        /* Find the simulated devices from device framework. */
        dev = elab_device_find(dev_name_1);
        TEST_ASSERT_NULL(dev);
        dev = elab_device_find(dev_name_2);
        TEST_ASSERT_NULL(dev);
    }
}

/**
  * @brief  Assert failures in device framework functions.
  */
TEST(simu_serial, make_rx_data)
{
    elab_device_t *dev = NULL;
    simu_serial_new("simu_serial", SIMU_SERIAL_MODE_SINGLE, 115200);
    dev = elab_device_find("simu_serial");
    TEST_ASSERT_NOT_NULL(dev);
    elab_device_open(dev);

    int32_t ret = 0;
    for (uint32_t i = 0; i < UT_SIMU_SERIAL_TIMES; i ++)
    {
        /* Make rx data */
        memset(buff_rx, 0, UT_SIMU_SERIAL_BUFF_SIZE);
        simu_serial_make_rx_data("simu_serial", UT_STR_READ, strlen(UT_STR_READ));
        ret = elab_serial_read(dev, buff_rx, UT_SIMU_SERIAL_BUFF_SIZE, 5);
        TEST_ASSERT_EQUAL_INT32(strlen(UT_STR_READ), ret);
        TEST_ASSERT_EQUAL_MEMORY(UT_STR_READ, buff_rx, ret);

        /* Make rx data in the delay mode. */
        memset(buff_rx, 0, UT_SIMU_SERIAL_BUFF_SIZE);
        simu_serial_make_rx_data_delay("simu_serial",
                                        UT_STR_READ,
                                        strlen(UT_STR_READ),
                                        10);
        ret = elab_serial_read(dev, buff_rx, UT_SIMU_SERIAL_BUFF_SIZE, 15);
        TEST_ASSERT_EQUAL_INT32(strlen(UT_STR_READ), ret);
        TEST_ASSERT_EQUAL_MEMORY(UT_STR_READ, buff_rx, ret);
    }

    elab_device_close(dev);
    simu_serial_destroy("simu_serial");
    dev = elab_device_find("simu_serial");
    TEST_ASSERT_NULL(dev);
}

/**
  * @brief  Assert failures in device framework functions.
  */
TEST(simu_serial, read_tx_data)
{
    elab_device_t *dev = NULL;
    simu_serial_new("simu_serial", SIMU_SERIAL_MODE_SINGLE, 115200);
    dev = elab_device_find("simu_serial");
    TEST_ASSERT_NOT_NULL(dev);
    elab_device_open(dev);

    int32_t ret = 0;
    uint32_t time_start = 0;
    for (uint32_t i = 0; i < UT_SIMU_SERIAL_TIMES; i ++)
    {
        memset(buff_tx, 0, UT_SIMU_SERIAL_BUFF_SIZE);
        elab_serial_write(dev, UT_STR_WRITE, strlen(UT_STR_WRITE));
        time_start = osKernelGetTickCount();
        ret = simu_serial_read_tx_data("simu_serial",
                                        buff_tx, UT_SIMU_SERIAL_BUFF_SIZE, 5);
        TEST_ASSERT_GREATER_OR_EQUAL_UINT32(
                                (time_start + 5), osKernelGetTickCount());
        TEST_ASSERT_EQUAL_INT32(strlen(UT_STR_WRITE), ret);
        TEST_ASSERT_EQUAL_MEMORY(UT_STR_WRITE, buff_tx, ret);
    }

    elab_device_close(dev);
    simu_serial_destroy("simu_serial");
    dev = elab_device_find("simu_serial");
    TEST_ASSERT_NULL(dev);
}

/**
  * @brief  Assert failures in device framework functions.
  */
TEST(simu_serial, rx_tx_pair_uart_mqtt)
{
    int32_t ret = 0;
    uint32_t time_start = 0;

    elab_device_t *dev1 = NULL, *dev2 = NULL;
#if (UT_SIMU_SERIAL_LOCAL_EN != 0)
    simu_serial_new_pair("simu_serial_1", "simu_serial_2", 115200);
#endif
#if (UT_SIMU_SERIAL_MQTT_EN != 0)
    simu_serial_mqtt_new_pair("simu_serial_1", "simu_serial_2", 115200);
#endif
    dev1 = elab_device_find("simu_serial_1");
    TEST_ASSERT_NOT_NULL(dev1);
    elab_device_open(dev1);
    dev2 = elab_device_find("simu_serial_2");
    TEST_ASSERT_NOT_NULL(dev2);
    elab_device_open(dev2);

    /* dev1 -> dev2 */
    for (uint32_t i = 0; i < UT_SIMU_SERIAL_TIMES; i ++)
    {
        memset(buff_tx, 0, UT_SIMU_SERIAL_BUFF_SIZE);
        elab_serial_write(dev1, UT_STR_WRITE, strlen(UT_STR_WRITE));
        time_start = osKernelGetTickCount();
        ret = elab_serial_read(dev2, buff_tx, UT_SIMU_SERIAL_BUFF_SIZE, 5);
        TEST_ASSERT_GREATER_OR_EQUAL_UINT32(
                                (time_start + 5), osKernelGetTickCount());
        TEST_ASSERT_EQUAL_INT32(strlen(UT_STR_WRITE), ret);
        TEST_ASSERT_EQUAL_MEMORY(UT_STR_WRITE, buff_tx, ret);
    }

    /* dev2 -> dev1 */
    for (uint32_t i = 0; i < UT_SIMU_SERIAL_TIMES; i ++)
    {
        memset(buff_tx, 0, UT_SIMU_SERIAL_BUFF_SIZE);
        elab_serial_write(dev2, UT_STR_WRITE, strlen(UT_STR_WRITE));
        time_start = osKernelGetTickCount();
        ret = elab_serial_read(dev1, buff_tx, UT_SIMU_SERIAL_BUFF_SIZE, 5);
        TEST_ASSERT_GREATER_OR_EQUAL_UINT32(
                                (time_start + 5), osKernelGetTickCount());
        TEST_ASSERT_EQUAL_INT32(strlen(UT_STR_WRITE), ret);
        TEST_ASSERT_EQUAL_MEMORY(UT_STR_WRITE, buff_tx, ret);
    }

    /* null -> dev1 */
    for (uint32_t i = 0; i < UT_SIMU_SERIAL_TIMES; i ++)
    {
        memset(buff_tx, 0, UT_SIMU_SERIAL_BUFF_SIZE);
        time_start = osKernelGetTickCount();
        ret = elab_serial_read(dev1, buff_tx, UT_SIMU_SERIAL_BUFF_SIZE, 5);
        TEST_ASSERT_GREATER_OR_EQUAL_UINT32(
                                (time_start + 5), osKernelGetTickCount());
        TEST_ASSERT_EQUAL_INT32(ELAB_ERR_TIMEOUT, ret);
        for (uint32_t j = 0; j < UT_SIMU_SERIAL_BUFF_SIZE; j ++)
        {
            TEST_ASSERT_EQUAL_UINT8(0, ((uint8_t *)buff_tx)[j]);
        }
    }

    elab_device_close(dev1);
    simu_serial_destroy("simu_serial_1");
    dev1 = elab_device_find("simu_serial_1");
    TEST_ASSERT_NULL(dev1);
    elab_device_close(dev2);
    simu_serial_destroy("simu_serial_2");
    dev2 = elab_device_find("simu_serial_2");
    TEST_ASSERT_NULL(dev2);
}

/**
  * @brief  Assert failures in device framework functions.
  */
TEST(simu_serial, rx_tx_pair_uart_mqtt_cross_thread)
{
    int32_t ret = 0;
    uint32_t time_start = 0;
    osStatus_t ret_os = osOK;

    elab_device_t *dev1 = NULL, *dev2 = NULL;
#if (UT_SIMU_SERIAL_LOCAL_EN != 0)
    simu_serial_new_pair("simu_serial_1", "simu_serial_2", 115200);
#endif
#if (UT_SIMU_SERIAL_MQTT_EN != 0)
    simu_serial_mqtt_new_pair("simu_serial_1", "simu_serial_2", 115200);
#endif
    dev1 = elab_device_find("simu_serial_1");
    TEST_ASSERT_NOT_NULL(dev1);
    elab_device_open(dev1);
    dev2 = elab_device_find("simu_serial_2");
    TEST_ASSERT_NOT_NULL(dev2);
    elab_device_open(dev2);

    /* Start one thread for serial port reading. */
    osThreadId_t thread =
        osThreadNew(entry_serial_read, dev2, &attr_serial_read);
    TEST_ASSERT_NOT_NULL(thread);

    /* Create one semaphore. */
    sem_test_cross_thread = osSemaphoreNew(1, 0, NULL);
    TEST_ASSERT_NOT_NULL(sem_test_cross_thread);

    /* Generate random data in the tx buffer. */
    for (uint32_t i = 0; i < UT_SIMU_SERIAL_BUFF_SIZE; i ++)
    {
        buff_tx[i] = rand() % UINT8_MAX;
    }

    for (uint32_t i = 0; i < UT_SIMU_SERIAL_TIMES; i ++)
    {
        count_rx_cross_thread = 0;

        elab_serial_write(dev1, buff_tx, UT_SIMU_SERIAL_BUFF_SIZE);
        ret_os = osSemaphoreAcquire(sem_test_cross_thread, osWaitForever);
        TEST_ASSERT(ret_os == osOK);

        TEST_ASSERT_EQUAL_UINT32(UT_SIMU_SERIAL_BUFF_SIZE, count_rx_cross_thread);
    }

    ret_os = osThreadTerminate(thread);
    TEST_ASSERT(ret_os == osOK);

    ret_os = osSemaphoreDelete(sem_test_cross_thread);
    TEST_ASSERT(ret_os == osOK);

    elab_device_close(dev1);
    simu_serial_destroy("simu_serial_1");
    dev1 = elab_device_find("simu_serial_1");
    TEST_ASSERT_NULL(dev1);
    elab_device_close(dev2);
    simu_serial_destroy("simu_serial_2");
    dev2 = elab_device_find("simu_serial_2");
    TEST_ASSERT_NULL(dev2);
}

/**
  * @brief  Define run test cases of device core
  */
TEST_GROUP_RUNNER(simu_serial)
{
    RUN_TEST_CASE(simu_serial, new_destroy_single_mode);
    RUN_TEST_CASE(simu_serial, new_destroy_pair_uart_mqtt);
    RUN_TEST_CASE(simu_serial, make_rx_data);
    RUN_TEST_CASE(simu_serial, read_tx_data);
    RUN_TEST_CASE(simu_serial, rx_tx_pair_uart_mqtt);
    RUN_TEST_CASE(simu_serial, rx_tx_pair_uart_mqtt_cross_thread);
}

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Simulated driver opening function for serial device testing.
  */
static void entry_serial_read(void *paras)
{
    elab_device_t *dev2 = (elab_device_t *)paras;
    int32_t ret = ELAB_OK;
    int32_t ret_bkp = ELAB_ERR_TIMEOUT;
    char ch;

    while (1)
    {
        ret = elab_serial_read(dev2, &ch, 1, 5);
        if (ret > 0)
        {
            count_rx_cross_thread ++;
        }
        else if (ret == ELAB_ERR_TIMEOUT && ret_bkp > 0)
        {
            osSemaphoreRelease(sem_test_cross_thread);
        }

        ret_bkp = ret;
    }
}

#endif

/* ----------------------------- end of file -------------------------------- */
