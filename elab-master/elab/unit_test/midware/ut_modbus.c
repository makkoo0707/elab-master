/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "../../midware/modbus/modbus.h"
#include "../../3rd/Unity/unity.h"
#include "../../3rd/Unity/unity_fixture.h"
#include "../../common/elab_common.h"
#include "../../common/elab_assert.h"
#include "../../common/elab_log.h"

ELAB_TAG("ut_modbus");

/* Private config ------------------------------------------------------------*/
#define UT_MODBUS_RX_TIMEOUT                    (200)

#define UT_MODBUS_TEST_TIMES                    (1000)
#define UT_MODBUS_DI_START_ADDR                 (2000)
#define UT_MODBUS_REG_START_ADDR                (5000)
#define UT_MODBUS_REG_FP_START_ADDR             (50000)
#define UT_MODBUS_COIL_START_ADDR               (1000)
#define UT_MODBUS_ERR_TEST_TIMES                (5)

/* MODBUS_MODE_ASCII or MODBUS_MODE_RTU */
#define UT_MODBUS_TEST_MODE                     MODBUS_MODE_RTU

/* Test enable */
#define UT_MODBUS_NUM_MAX_EN                    (0)
#define UT_MODBUS_NUM_MIN_EN                    (0)

#if (UT_MODBUS_NUM_MAX_EN != 0 && UT_MODBUS_NUM_MIN_EN != 0)
    #error "UT_MODBUS_NUM_MAX_EN and UT_MODBUS_NUM_MIN_EN can only be set only one !"
#endif

/* Private function prototypes -----------------------------------------------*/
static bool cb_coil_read(uint16_t coil, uint16_t *perr);
static void cb_coil_write(uint16_t coil, bool coil_val, uint16_t *perr);
static bool cb_di_read(uint16_t di, uint16_t *perr);
static uint16_t cb_in_reg_read(uint16_t reg, uint16_t *perr);
static float cb_in_reg_read_fp(uint16_t reg, uint16_t *perr);
static uint16_t cb_holding_reg_read(uint16_t reg, uint16_t *perr);
static float cb_holding_reg_read_fp(uint16_t reg, uint16_t *perr);
static void cb_holding_reg_write(uint16_t reg,
                                    uint16_t reg_val_16, uint16_t *perr);
static void cb_holding_reg_write_fp(uint16_t reg,
                                    float reg_val_fp, uint16_t *perr);
static bool get_di_original_value(uint16_t di);

/* Private variables ---------------------------------------------------------*/
static mb_channel_t *mbm = NULL;                   /* Modbus master */
static mb_channel_t *mbs = NULL;                   /* Modbus slave */

static bool set_coil_value = false;
static bool get_coil_value = false;
static uint16_t set_address = 0;
static uint16_t set_reg_value = 0;
static float set_reg_value_fp = 0;
static uint8_t *status_coil_rd = NULL;
static uint8_t *status_coil_wr = NULL;
static uint8_t *status_di = NULL;
static uint8_t *status_di_origin = NULL;
static uint16_t *status_reg = NULL;
static uint16_t *status_reg_origin = NULL;
static float *status_reg_fp = NULL;
static float *status_reg_origin_fp = NULL;
static uint32_t size_malloc = 0;
static uint16_t coil_set_count = 0;
static uint16_t reg_set_count = 0;

static mb_channel_cb_t cb_mbs =
{
    .coil_read      = cb_coil_read,
    .coil_write     = cb_coil_write,
    .di_read        = cb_di_read,
    .in_reg_read    = cb_in_reg_read,
    .in_reg_read_fp = cb_in_reg_read_fp,

    .holding_reg_read       = cb_holding_reg_read,
    .holding_reg_read_fp    = cb_holding_reg_read_fp,
    .holding_reg_write      = cb_holding_reg_write,
    .holding_reg_write_fp   = cb_holding_reg_write_fp,
};

/* Exported functions --------------------------------------------------------*/
/**
  * @brief  Define test group of modbus protocol stack.
  */
TEST_GROUP(modbus);

/**
  * @brief  Define test fixture setup function of modbus protocol stack.
  */
TEST_SETUP(modbus)
{
    srand(time(0));
    size_malloc = (UT_MODBUS_TEST_TIMES * 2 + 125) * 2;

    /* Modbus master and slave initialization. */
    mbm = mb_channel_create("simu_uart_1", 0, MODBUS_MASTER,
                            UT_MODBUS_RX_TIMEOUT, UT_MODBUS_TEST_MODE);
    mbs = mb_channel_create("simu_uart_2", 2, MODBUS_SLAVE,
                            UT_MODBUS_RX_TIMEOUT, UT_MODBUS_TEST_MODE);
    mb_slave_write_enable(mbs, true);

    TEST_ASSERT_NOT_NULL(mbm);
    TEST_ASSERT_NOT_NULL(mbs);
    mb_slave_set_cb(mbs, &cb_mbs);

    /* Status memories applying. */
    status_coil_rd = elab_malloc(size_malloc);
    TEST_ASSERT_NOT_NULL(status_coil_rd);
    status_di = elab_malloc(size_malloc);
    TEST_ASSERT_NOT_NULL(status_di);
    status_di_origin = elab_malloc(size_malloc);
    TEST_ASSERT_NOT_NULL(status_di_origin);
    status_reg = elab_malloc(size_malloc);
    TEST_ASSERT_NOT_NULL(status_reg);
    status_reg_origin = elab_malloc(size_malloc);
    TEST_ASSERT_NOT_NULL(status_reg_origin);
    status_reg_fp = elab_malloc(size_malloc);
    TEST_ASSERT_NOT_NULL(status_reg_fp);
    status_reg_origin_fp = elab_malloc(size_malloc);
    TEST_ASSERT_NOT_NULL(status_reg_origin_fp);
    status_coil_wr = elab_malloc(size_malloc);
    TEST_ASSERT_NOT_NULL(status_coil_wr);

    /* Generate random status origin value. */
    for (uint32_t i = 0; i < size_malloc; i ++)
    {
        status_di_origin[i] = rand() % 256;
        status_coil_wr[i] = (uint8_t)i;//rand() % 256;
        status_coil_rd[i] = 0;
        status_di[i] = 0;
    }
    for (uint32_t i = 0; i < (size_malloc / 2); i ++)
    {
        status_reg_origin[i] = rand() % 65536;
        status_reg[i] = 0;
    }
    for (uint32_t i = 0; i < (size_malloc / 4); i ++)
    {
        status_reg_origin_fp[i] = (float)(rand() % 65536) * 33.3;
        status_reg[i] = 0.0;
    }

    /* Variables. */
    set_address = 0;
    set_reg_value = 0;
    set_coil_value = false;
    get_coil_value = false;
}

/**
  * @brief  Define test fixture tear down function of modbus protocol stack.
  */
TEST_TEAR_DOWN(modbus)
{
    elab_free(status_coil_rd);
    elab_free(status_di);
    elab_free(status_di_origin);
    elab_free(status_reg);
    elab_free(status_reg_origin);
    elab_free(status_reg_fp);
    elab_free(status_reg_origin_fp);
    elab_free(status_coil_wr);
    status_coil_rd = NULL;
    status_di = NULL;
    status_di_origin = NULL;
    status_reg = NULL;
    status_reg_origin = NULL;
    status_coil_wr = NULL;
    status_reg_fp = NULL;
    status_reg_origin_fp = NULL;

    mb_channel_destroy(mbm);
    mb_channel_destroy(mbs);

    mbm = NULL;
    mbs = NULL;
}

/**
  * @brief  Modbus master function 01, coil reading unit test.
  */
TEST(modbus, 01_coil_read)
{
    uint16_t ret = 0;
    uint16_t coil_num = 1;

    /* Normal mode */
    for (uint32_t i = UT_MODBUS_COIL_START_ADDR;
            i < (UT_MODBUS_TEST_TIMES + UT_MODBUS_COIL_START_ADDR);
            i ++)
    {
        /* Generate the random coil number to be read. */
        coil_num = rand() % (2000 - 1) + 1;           /* (1 ~ 2000)*/
#if (UT_MODBUS_NUM_MAX_EN != 0)
        coil_num = 2000;
#endif
#if (UT_MODBUS_NUM_MIN_EN != 0)
        coil_num = 1;
#endif
        /* Coil reading. */
        ret = mbm_fc01_coil_read(mbm, 2, i, status_coil_rd, coil_num);
        if (ret != MODBUS_ERR_NONE)
        {
            printf("Test index %u failure. ret: %u\n", (i - UT_MODBUS_REG_START_ADDR), ret);
        }
        TEST_ASSERT_EQUAL_UINT16(MODBUS_ERR_NONE, ret);
        
        /* Check the coil value. */
        uint8_t byte;
        uint32_t count_bytes = (coil_num - 1) / 8 + 1;
        for (uint32_t m = 0; m < count_bytes; m ++)
        {
            byte = i % 2 == 0 ? 0xAA : 0x55;
            if (m == (count_bytes - 1) && ((coil_num % 8) != 0))
            {
                for (uint32_t k = (coil_num % 8); k < 8; k ++)
                {
                    byte &=~ (1 << k);
                }
            }
            TEST_ASSERT_EQUAL_HEX8(byte, status_coil_rd[m]);
        }
    }

    /* The wrong slave node number. */
    for (uint32_t i = UT_MODBUS_COIL_START_ADDR;
            i < (UT_MODBUS_ERR_TEST_TIMES + UT_MODBUS_COIL_START_ADDR); i ++)
    {
        ret = mbm_fc01_coil_read(mbm, 1, i, status_coil_rd, coil_num);
        TEST_ASSERT_EQUAL_UINT16(MODBUS_ERR_TIMED_OUT, ret);
    }
}

/**
  * @brief  Modbus master function 02, digital input reading unit test.
  */
TEST(modbus, 02_di_read)
{
    uint16_t ret = 0;
    uint16_t di_num = 1;

    /* Normal mode */
    for (uint32_t i = UT_MODBUS_DI_START_ADDR;
            i < (UT_MODBUS_TEST_TIMES + UT_MODBUS_DI_START_ADDR);
            i ++)
    {
        /* Generate the random coil number to be read. */
        di_num = rand() % (2000 - 1) + 1;           /* (1 ~ 2000)*/
#if (UT_MODBUS_NUM_MAX_EN != 0)
        di_num = 2000;
#endif
#if (UT_MODBUS_NUM_MIN_EN != 0)
        di_num = 1;
#endif
        /* Digital input reading. */
        ret = mbm_fc02_di_read(mbm, 2, i, status_di, di_num);
        if (ret != MODBUS_ERR_NONE)
        {
            printf("Test index %u failure.\n", (i - UT_MODBUS_DI_START_ADDR));
        }
        TEST_ASSERT_EQUAL_UINT16(MODBUS_ERR_NONE, ret);

        /* Check the read status data is correct or not. */
        for (uint32_t j = 0; j < di_num; j ++)
        {
            if ((status_di[j / 8] & (1 << (j % 8))) == 0)
            {
                TEST_ASSERT_FALSE(get_di_original_value(i + j));
            }
            else
            {
                TEST_ASSERT_TRUE(get_di_original_value(i + j));
            }
        }
    }

    /* The wrong slave node number. */
    for (uint32_t i = UT_MODBUS_COIL_START_ADDR;
            i < (UT_MODBUS_ERR_TEST_TIMES + UT_MODBUS_COIL_START_ADDR); i ++)
    {
        ret = mbm_fc02_di_read(mbm, 1, i, status_di, di_num);
        TEST_ASSERT_EQUAL_UINT16(MODBUS_ERR_TIMED_OUT, ret);
    }
}

/**
  * @brief  Modbus master function 03, holding registers reading unit test.
  */
TEST(modbus, 03_holding_reg_read)
{
    uint16_t ret = 0;
    uint8_t reg_num = 1;

    /* Normal mode */
    for (uint32_t i = UT_MODBUS_REG_START_ADDR;
            i < (UT_MODBUS_TEST_TIMES + UT_MODBUS_REG_START_ADDR);
            i ++)
    {
        reg_num = rand() % (125 - 1) + 1;
#if (UT_MODBUS_NUM_MAX_EN != 0)
        reg_num = 125;
#endif
#if (UT_MODBUS_NUM_MIN_EN != 0)
        reg_num = 1;
#endif

        memset(status_reg, 0, size_malloc);
        ret = mbm_fc03_holding_reg_read(mbm, 2, i, status_reg, reg_num);
        if (ret != MODBUS_ERR_NONE)
        {
            printf("Test index %u failure.\n", (i - UT_MODBUS_REG_START_ADDR));
        }
        TEST_ASSERT_EQUAL_UINT16(MODBUS_ERR_NONE, ret);

        /* Check the read status data is correct or not. */
        for (uint32_t j = 0; j < reg_num; j ++)
        {
            TEST_ASSERT_EQUAL_UINT16(status_reg_origin[i - UT_MODBUS_REG_START_ADDR + j],
                                     status_reg[j]);
        }
    }

    /* The wrong slave node number. */
    for (uint32_t i = UT_MODBUS_REG_START_ADDR;
            i < (UT_MODBUS_ERR_TEST_TIMES + UT_MODBUS_REG_START_ADDR); i ++)
    {
        ret = mbm_fc03_holding_reg_read(mbm, 1, i, status_reg, reg_num);
        TEST_ASSERT_EQUAL_UINT16(MODBUS_ERR_TIMED_OUT, ret);
    }
}

/**
  * @brief  Modbus master function 03, float holding registers reading unit test.
  */
#if (MODBUS_CFG_FC03_EN != 0) && (MODBUS_CFG_FP_EN != 0)
TEST(modbus, 03_holding_reg_read_fp)
{
    uint16_t ret = 0;
    uint8_t reg_num = 1;

    /* Normal mode */
    for (uint32_t i = UT_MODBUS_REG_FP_START_ADDR;
            i < (UT_MODBUS_TEST_TIMES + UT_MODBUS_REG_FP_START_ADDR);
            i ++)
    {
        reg_num = rand() % (62 - 1) + 1;
#if (UT_MODBUS_NUM_MAX_EN != 0)
        reg_num = 62;
#endif
#if (UT_MODBUS_NUM_MIN_EN != 0)
        reg_num = 1;
#endif
        for (uint32_t f = 0; f < 62; f ++)
        {
            status_reg_fp[f] = 0.0;
        }
        ret = mbm_fc03_holding_reg_read_fp(mbm, 2, i, status_reg_fp, reg_num);
        if (ret != MODBUS_ERR_NONE)
        {
            printf("Test index %u failure.\n", (i - UT_MODBUS_REG_FP_START_ADDR));
        }
        TEST_ASSERT_EQUAL_UINT16(MODBUS_ERR_NONE, ret);

        /* Check the read status data is correct or not. */
        for (uint32_t j = 0; j < reg_num; j ++)
        {
            TEST_ASSERT_EQUAL_FLOAT(status_reg_origin_fp[i - UT_MODBUS_REG_FP_START_ADDR + j],
                                    status_reg_fp[j]);
        }
    }

    /* The wrong slave node number. */
    for (uint32_t i = UT_MODBUS_REG_FP_START_ADDR;
            i < (UT_MODBUS_ERR_TEST_TIMES + UT_MODBUS_REG_FP_START_ADDR); i ++)
    {
        ret = mbm_fc03_holding_reg_read_fp(mbm, 1, i, status_reg_fp, reg_num);
        TEST_ASSERT_EQUAL_UINT16(MODBUS_ERR_TIMED_OUT, ret);
    }
}
#endif

/**
  * @brief  Modbus master function 04, input registers reading unit test.
  */
#if (MODBUS_CFG_FC04_EN != 0)
TEST(modbus, 04_in_reg_read)
{
    uint16_t ret = 0;
    uint8_t reg_num = 1;

    /* Normal mode */
    for (uint32_t i = UT_MODBUS_REG_START_ADDR;
            i < (UT_MODBUS_TEST_TIMES + UT_MODBUS_REG_START_ADDR);
            i ++)
    {
        reg_num = rand() % (125 - 1) + 1;
#if (UT_MODBUS_NUM_MAX_EN != 0)
        reg_num = 125;
#endif
#if (UT_MODBUS_NUM_MIN_EN != 0)
        reg_num = 1;
#endif
        memset(status_reg, 0, size_malloc);
        ret = mbm_fc04_in_reg_read(mbm, 2, i, status_reg, reg_num);
        if (ret != MODBUS_ERR_NONE)
        {
            printf("Test index %u failure.\n", (i - UT_MODBUS_REG_START_ADDR));
        }
        TEST_ASSERT_EQUAL_UINT16(MODBUS_ERR_NONE, ret);

        /* Check the read status data is correct or not. */
        for (uint32_t j = 0; j < reg_num; j ++)
        {
            TEST_ASSERT_EQUAL_UINT16(status_reg_origin[i - UT_MODBUS_REG_START_ADDR + j],
                                     status_reg[j]);
        }
    }

    /* The wrong slave node number. */
    for (uint32_t i = UT_MODBUS_REG_START_ADDR;
            i < (UT_MODBUS_ERR_TEST_TIMES + UT_MODBUS_REG_START_ADDR); i ++)
    {
        ret = mbm_fc04_in_reg_read(mbm, 1, i, status_reg, reg_num);
        TEST_ASSERT_EQUAL_UINT16(MODBUS_ERR_TIMED_OUT, ret);
    }
}
#endif

/**
  * @brief  Modbus master function 04, float input registers reading unit test.
  */
#if (MODBUS_CFG_FC04_EN != 0) && (MODBUS_CFG_FP_EN != 0)
TEST(modbus, 04_in_reg_read_fp)
{
    uint16_t ret = 0;
    uint8_t reg_num = 1;

    /* Normal mode */
    for (uint32_t i = UT_MODBUS_REG_FP_START_ADDR;
            i < (UT_MODBUS_TEST_TIMES + UT_MODBUS_REG_FP_START_ADDR);
            i ++)
    {
        reg_num = rand() % (62 - 1) + 1;
#if (UT_MODBUS_NUM_MAX_EN != 0)
        reg_num = 62;
#endif
#if (UT_MODBUS_NUM_MIN_EN != 0)
        reg_num = 1;
#endif
        for (uint32_t f = 0; f < 62; f ++)
        {
            status_reg_fp[f] = 0.0;
        }
        ret = mbm_fc04_in_reg_read_fp(mbm, 2, i, status_reg_fp, reg_num);
        if (ret != MODBUS_ERR_NONE)
        {
            printf("Test index %u failure.\n", (i - UT_MODBUS_REG_FP_START_ADDR));
        }
        TEST_ASSERT_EQUAL_UINT16(MODBUS_ERR_NONE, ret);

        /* Check the read status data is correct or not. */
        for (uint32_t j = 0; j < reg_num; j ++)
        {
            TEST_ASSERT_EQUAL_UINT16(status_reg_origin_fp[i - UT_MODBUS_REG_FP_START_ADDR + j],
                                     status_reg_fp[j]);
        }
    }

    /* The wrong slave node number. */
    for (uint32_t i = UT_MODBUS_REG_FP_START_ADDR;
            i < (UT_MODBUS_ERR_TEST_TIMES + UT_MODBUS_REG_FP_START_ADDR); i ++)
    {
        ret = mbm_fc04_in_reg_read_fp(mbm, 1, i, status_reg_fp, reg_num);
        TEST_ASSERT_EQUAL_UINT16(MODBUS_ERR_TIMED_OUT, ret);
    }
}
#endif

/**
  * @brief  Modbus master function 05, coil writing unit test.
  */
#if (MODBUS_CFG_FC05_EN != 0)
TEST(modbus, 05_coil_write)
{
    uint16_t ret = 0;

    /* Normal mode */
    for (uint32_t i = UT_MODBUS_COIL_START_ADDR;
            i < (UT_MODBUS_TEST_TIMES + UT_MODBUS_COIL_START_ADDR);
            i ++)
    {
        coil_set_count = 0;
        ret = mbm_fc05_wirte_coil(mbm, 2, i, (i % 2));
        if (ret != MODBUS_ERR_NONE)
        {
            printf("Test index %u failure. ret: %u\n", (i - UT_MODBUS_REG_START_ADDR), ret);
        }
        TEST_ASSERT_EQUAL_UINT16(MODBUS_ERR_NONE, ret);
        TEST_ASSERT_EQUAL_UINT16(1, coil_set_count);
        TEST_ASSERT_EQUAL_UINT16(i, set_address);
        TEST_ASSERT_EQUAL_UINT8((i % 2), set_coil_value);
    }

    /* The wrong slave node number. */
    set_address = 0;
    set_coil_value = false;
    for (uint32_t i = UT_MODBUS_COIL_START_ADDR;
            i < (UT_MODBUS_ERR_TEST_TIMES + UT_MODBUS_COIL_START_ADDR); i ++)
    {
        ret = mbm_fc05_wirte_coil(mbm, 1, i, (i % 2));
        TEST_ASSERT_EQUAL_UINT16(MODBUS_ERR_TIMED_OUT, ret);
        TEST_ASSERT_EQUAL_UINT16(0, set_address);
        TEST_ASSERT_FALSE(set_coil_value);
    }
}
#endif

/**
  * @brief  Modbus master function 06, holding register writing unit test.
  */
#if (MODBUS_CFG_FC06_EN != 0)
TEST(modbus, 06_holding_reg_write)
{
    uint16_t ret = 0;

    /* Normal mode */
    for (uint32_t i = UT_MODBUS_REG_START_ADDR;
            i < (UT_MODBUS_TEST_TIMES + UT_MODBUS_REG_START_ADDR);
            i ++)
    {
        reg_set_count = 0;
        ret = mbm_fc06_holding_reg_write(mbm, 2, i, (i + UT_MODBUS_REG_START_ADDR));
        if (ret != MODBUS_ERR_NONE)
        {
            printf("Test index %u failure.\n", (i - UT_MODBUS_REG_START_ADDR));
        }
        TEST_ASSERT_EQUAL_UINT16(MODBUS_ERR_NONE, ret);
        
        TEST_ASSERT_EQUAL_UINT16(i, set_address);
        TEST_ASSERT_EQUAL_UINT16((i + UT_MODBUS_REG_START_ADDR), set_reg_value);
    }

    /* The wrong slave node number. */
    set_address = 0;
    set_reg_value = 0;
    for (uint32_t i = UT_MODBUS_REG_START_ADDR;
            i < (UT_MODBUS_ERR_TEST_TIMES + UT_MODBUS_REG_START_ADDR); i ++)
    {
        ret = mbm_fc06_holding_reg_write(mbm, 1, i, (i + UT_MODBUS_REG_START_ADDR));
        TEST_ASSERT_EQUAL_UINT16(MODBUS_ERR_TIMED_OUT, ret);
        TEST_ASSERT_EQUAL_UINT16(0, set_address);
        TEST_ASSERT_EQUAL_UINT16(0, set_reg_value);
    }
}
#endif

/**
  * @brief  Modbus master function 06, holding register writing unit test.
  */
#if (MODBUS_CFG_FC06_EN != 0) && (MODBUS_CFG_FP_EN != 0)
TEST(modbus, 06_holding_reg_write_fp)
{
    uint16_t ret = 0;

    /* Normal mode */
    for (uint32_t i = UT_MODBUS_REG_FP_START_ADDR;
            i < (UT_MODBUS_TEST_TIMES + UT_MODBUS_REG_FP_START_ADDR);
            i ++)
    {
        reg_set_count = 0;
        ret = mbm_fc06_holding_reg_write_fp(
                mbm, 2, i,
                status_reg_origin_fp[i - UT_MODBUS_REG_FP_START_ADDR]);
        if (ret != MODBUS_ERR_NONE)
        {
            printf("Test index %u failure.\n", (i - UT_MODBUS_REG_START_ADDR));
        }
        TEST_ASSERT_EQUAL_UINT16(MODBUS_ERR_NONE, ret);
        TEST_ASSERT_EQUAL_UINT16(i, set_address);
        TEST_ASSERT_EQUAL_UINT16(status_reg_origin_fp[i - UT_MODBUS_REG_FP_START_ADDR],
                                    set_reg_value_fp);
    }

    /* The wrong slave node number. */
    set_address = 0;
    set_reg_value = 0;
    for (uint32_t i = UT_MODBUS_REG_FP_START_ADDR;
            i < (UT_MODBUS_ERR_TEST_TIMES + UT_MODBUS_REG_FP_START_ADDR); i ++)
    {
        ret = mbm_fc06_holding_reg_write_fp(
                    mbm, 1, i,
                    status_reg_origin_fp[i - UT_MODBUS_REG_FP_START_ADDR]);
        TEST_ASSERT_EQUAL_UINT16(MODBUS_ERR_TIMED_OUT, ret);
        TEST_ASSERT_EQUAL_UINT16(0, set_address);
        TEST_ASSERT_EQUAL_UINT16(0, set_reg_value);
    }
}
#endif

/**
  * @brief  Modbus master function 15, coil writing unit test.
  */
#if (MODBUS_CFG_FC15_EN != 0)
TEST(modbus, 15_coil_write)
{
    uint16_t ret = 0;
    uint16_t coil_num = 1;

    /* Normal mode */
    for (uint32_t i = UT_MODBUS_COIL_START_ADDR;
            i < (UT_MODBUS_TEST_TIMES + UT_MODBUS_COIL_START_ADDR);
            i ++)
    {
        /* Generate the random coil number to be read. */
        coil_num = rand() % (2000 - 1) + 1;           /* (1 ~ 2000)*/
        coil_num = 2000;
#if (UT_MODBUS_NUM_MAX_EN != 0)
        coil_num = 2000;
#endif
#if (UT_MODBUS_NUM_MIN_EN != 0)
        coil_num = 1;
#endif
        coil_set_count = 0;
        ret = mbm_fc15_coil_write(mbm, 2, i, status_coil_wr, coil_num);
        if (ret != MODBUS_ERR_NONE)
        {
            printf("Test index %u failure. ret: %u\n", (i - UT_MODBUS_COIL_START_ADDR), ret);
        }
        TEST_ASSERT_EQUAL_UINT16(MODBUS_ERR_NONE, ret);
        TEST_ASSERT_EQUAL_UINT16(coil_num, coil_set_count);
        TEST_ASSERT_EQUAL_UINT16((i + coil_num - 1), set_address);
        TEST_ASSERT_EQUAL_UINT8(((coil_num % 2) ? 0 : 1), set_coil_value);
        TEST_ASSERT_EQUAL_MEMORY(status_coil_rd, status_coil_wr, ((coil_num - 1) / 8 + 1));

#if 0
        for (uint32_t m = 0; m < ((coil_num - 1) / 8 + 1); m ++)
        {
            printf("0x%02x ", status_coil_rd[m]);
            if ((m + 1) % 16 == 0)
            {
                printf("\n");
            }
        }
#endif
    }

    return;

    /* The wrong slave node number. */
    set_address = 0;
    set_reg_value = 0;
    for (uint32_t i = UT_MODBUS_REG_START_ADDR;
            i < (UT_MODBUS_ERR_TEST_TIMES + UT_MODBUS_REG_START_ADDR); i ++)
    {
        ret = mbm_fc15_coil_write(mbm, 1, i, status_coil_wr, coil_num);
        TEST_ASSERT_EQUAL_UINT16(MODBUS_ERR_TIMED_OUT, ret);
    }
}
#endif

/**
  * @brief  Modbus master function 16, registers writing unit test.
  */
#if (MODBUS_CFG_FC16_EN != 0)
TEST(modbus, 16_reg_write)
{
    uint16_t ret = 0;
    uint8_t reg_num = 1;

    /* Normal mode */
    for (uint32_t i = UT_MODBUS_REG_START_ADDR;
            i < (UT_MODBUS_TEST_TIMES + UT_MODBUS_REG_START_ADDR);
            i ++)
    {
        reg_num = rand() % (125 - 1) + 1;
#if (UT_MODBUS_NUM_MAX_EN != 0)
        reg_num = 125;
#endif
#if (UT_MODBUS_NUM_MIN_EN != 0)
        reg_num = 1;
#endif
        reg_set_count = 0;
        memset(status_reg, 0, size_malloc);
        ret = mbm_fc16_holding_reg_write(mbm, 2, i, status_reg_origin, reg_num);
        if (ret != MODBUS_ERR_NONE)
        {
            printf("Test index %u failure. ret: %u\n",
                    (i - UT_MODBUS_REG_START_ADDR), ret);
        }
        TEST_ASSERT_EQUAL_UINT16(MODBUS_ERR_NONE, ret);
        
        TEST_ASSERT_EQUAL_UINT16(reg_num, reg_set_count);
        for (uint32_t j = 0; j < reg_num; j ++)
        {
            TEST_ASSERT_EQUAL_UINT16(status_reg_origin[j], status_reg[j]);
        }
    }

    /* The wrong slave node number. */
    set_address = 0;
    set_reg_value = 0;
    for (uint32_t i = UT_MODBUS_REG_START_ADDR;
            i < (UT_MODBUS_ERR_TEST_TIMES + UT_MODBUS_REG_START_ADDR); i ++)
    {
        reg_set_count = 0;
        ret = mbm_fc16_holding_reg_write(mbm, 1, i, status_reg_origin, reg_num);
        TEST_ASSERT_EQUAL_UINT16(MODBUS_ERR_TIMED_OUT, ret);
    }
}
#endif

/**
  * @brief  Modbus master function 16, registers writing unit test.
  */
#if (MODBUS_CFG_FC16_EN != 0) && (MODBUS_CFG_FP_EN != 0)
TEST(modbus, 16_reg_write_fp)
{
    uint16_t ret = 0;
    uint8_t reg_num = 1;

    /* Normal mode */
    for (uint32_t i = UT_MODBUS_REG_FP_START_ADDR;
            i < (UT_MODBUS_TEST_TIMES + UT_MODBUS_REG_FP_START_ADDR);
            i ++)
    {
        reg_num = rand() % (62 - 1) + 1;
#if (UT_MODBUS_NUM_MAX_EN != 0)
        reg_num = 62;
#endif
#if (UT_MODBUS_NUM_MIN_EN != 0)
        reg_num = 1;
#endif
        reg_set_count = 0;
        for (uint32_t f = 0; f < 62; f ++)
        {
            status_reg[f] = 0.0;
        }
        ret = mbm_fc16_holding_reg_write_fp(mbm, 2, i,
                                            status_reg_origin_fp, reg_num);
        if (ret != MODBUS_ERR_NONE)
        {
            printf("Test index %u failure. ret: %u\n",
                    (i - UT_MODBUS_REG_FP_START_ADDR), ret);
        }
        TEST_ASSERT_EQUAL_UINT16(MODBUS_ERR_NONE, ret);
        
        TEST_ASSERT_EQUAL_UINT16(reg_num, reg_set_count);
        for (uint32_t j = 0; j < reg_num; j ++)
        {
            TEST_ASSERT_EQUAL_FLOAT(status_reg_origin_fp[j], status_reg_fp[j]);
        }
    }

    /* The wrong slave node number. */
    set_address = 0;
    set_reg_value = 0;
    for (uint32_t i = UT_MODBUS_REG_FP_START_ADDR;
            i < (UT_MODBUS_ERR_TEST_TIMES + UT_MODBUS_REG_FP_START_ADDR); i ++)
    {
        reg_set_count = 0;
        ret = mbm_fc16_holding_reg_write_fp(mbm, 1, i,
                                            status_reg_origin_fp, reg_num);
        TEST_ASSERT_EQUAL_UINT16(MODBUS_ERR_TIMED_OUT, ret);
    }
}
#endif

/**
  * @brief  Define run test cases of modbus master & slave.
  */
TEST_GROUP_RUNNER(modbus)
{
#if (MODBUS_CFG_FC01_EN != 0)
    RUN_TEST_CASE(modbus, 01_coil_read);
#endif
#if (MODBUS_CFG_FC02_EN != 0)
    RUN_TEST_CASE(modbus, 02_di_read);
#endif
#if (MODBUS_CFG_FC03_EN != 0)
    RUN_TEST_CASE(modbus, 03_holding_reg_read);
#endif
#if (MODBUS_CFG_FC03_EN != 0) && (MODBUS_CFG_FP_EN != 0)
    RUN_TEST_CASE(modbus, 03_holding_reg_read_fp);
#endif
#if (MODBUS_CFG_FC04_EN != 0)
    RUN_TEST_CASE(modbus, 04_in_reg_read);
#endif
#if (MODBUS_CFG_FC04_EN != 0) && (MODBUS_CFG_FP_EN != 0)
    RUN_TEST_CASE(modbus, 04_in_reg_read_fp);
#endif
#if (MODBUS_CFG_FC05_EN != 0)
    RUN_TEST_CASE(modbus, 05_coil_write);
#endif
#if (MODBUS_CFG_FC06_EN != 0)
    RUN_TEST_CASE(modbus, 06_holding_reg_write);
#endif
#if (MODBUS_CFG_FC06_EN != 0) && (MODBUS_CFG_FP_EN != 0)
    RUN_TEST_CASE(modbus, 06_holding_reg_write_fp);
#endif
#if (MODBUS_CFG_FC15_EN != 0)
    RUN_TEST_CASE(modbus, 15_coil_write);
#endif
#if (MODBUS_CFG_FC16_EN != 0)
    RUN_TEST_CASE(modbus, 16_reg_write);
#endif
#if (MODBUS_CFG_FC16_EN != 0) && (MODBUS_CFG_FP_EN != 0)
    RUN_TEST_CASE(modbus, 16_reg_write_fp);
#endif
}

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Modbus slave callback for reading coil.
  * @param  coil    Coil address.
  * @param  perr    Error information output.
  * @retval coil value.
  */
static bool cb_coil_read(uint16_t coil, uint16_t *perr)
{
    *perr = MODBUS_ERR_NONE;
    get_coil_value = (coil % 2);
    return get_coil_value;
}

/**
  * @brief  Modbus slave callback for write coil.
  * @param  coil        Coil address.
  * @param  coil_val    Coil value.
  * @param  perr        Error information output.
  * @retval None.
  */
static void cb_coil_write(uint16_t coil, bool coil_val, uint16_t *perr)
{
    *perr = MODBUS_ERR_NONE;
    set_address = coil;
    set_coil_value = coil_val;
    if (coil_val)
    {
        status_coil_rd[coil_set_count / 8] |= (coil_val << (coil_set_count % 8));
    }
    
    coil_set_count ++;
}

/**
  * @brief  Modbus slave callback for reading digital input.
  * @param  di      Digital input index.
  * @param  perr    Error information output.
  * @retval Digital input value.
  */
static bool cb_di_read(uint16_t di, uint16_t *perr)
{
    elab_assert(di >= UT_MODBUS_DI_START_ADDR);

    *perr = MODBUS_ERR_NONE;

    uint16_t index = (di - UT_MODBUS_DI_START_ADDR) / 8;
    uint8_t bit = (di - UT_MODBUS_DI_START_ADDR) % 8;
    return ((status_di_origin[index] & (0x01 << bit)) == 0) ? false : true;
}

/**
  * @brief  Modbus slave callback for reading input register.
  * @param  reg     Register address.
  * @param  perr    Error information output.
  * @retval Register value.
  */
static uint16_t cb_in_reg_read(uint16_t reg, uint16_t *perr)
{
    *perr = MODBUS_ERR_NONE;
    return status_reg_origin[reg - UT_MODBUS_REG_START_ADDR];
}

/**
  * @brief  Modbus slave callback for reading float input register.
  * @param  reg     Register address.
  * @param  perr    Error information output.
  * @retval Register value.
  */
static float cb_in_reg_read_fp(uint16_t reg, uint16_t *perr)
{
    *perr = MODBUS_ERR_NONE;
    return status_reg_origin_fp[reg - UT_MODBUS_REG_FP_START_ADDR];
}

/**
  * @brief  Modbus slave callback for reading holding register.
  * @param  reg     Holding register index.
  * @param  perr    Error information output.
  * @retval Register value.
  */
static uint16_t cb_holding_reg_read(uint16_t reg, uint16_t *perr)
{
    *perr = MODBUS_ERR_NONE;
    return status_reg_origin[reg - UT_MODBUS_REG_START_ADDR];
}

/**
  * @brief  Modbus slave callback for reading float holding register.
  * @param  reg     Holding register index.
  * @param  perr    Error information output.
  * @retval Float type register value.
  */
static float cb_holding_reg_read_fp(uint16_t reg, uint16_t *perr)
{
    *perr = MODBUS_ERR_NONE;
    return status_reg_origin_fp[reg - UT_MODBUS_REG_FP_START_ADDR];
}

/**
  * @brief  Modbus slave callback for write register.
  * @param  reg         Register address.
  * @param  reg_val_16  Register value.
  * @param  perr        Error information output.
  * @retval None.
  */
static void cb_holding_reg_write(uint16_t reg, uint16_t reg_val_16, uint16_t *perr)
{
    *perr = MODBUS_ERR_NONE;
    set_address = reg;
    set_reg_value = reg_val_16;
    status_reg[reg_set_count ++] = reg_val_16;
}

/**
  * @brief  Modbus slave callback for write register.
  * @param  reg         Register address.
  * @param  reg_val_fp  Float type register value.
  * @param  perr        Error information output.
  * @retval None.
  */
static void cb_holding_reg_write_fp(uint16_t reg,
                                    float reg_val_fp, uint16_t *perr)
{
    *perr = MODBUS_ERR_NONE;
    set_address = reg;
    set_reg_value_fp = reg_val_fp;
    status_reg_fp[reg_set_count ++] = reg_val_fp;
}

/**
  * @brief  Get the digital input original value.
  * @param  di      Digital input address.
  * @retval Digital input value.
  */
static bool get_di_original_value(uint16_t di)
{
    uint16_t index = (di - UT_MODBUS_DI_START_ADDR) / 8;
    uint8_t bit = (di - UT_MODBUS_DI_START_ADDR) % 8;
    return ((status_di_origin[index] & (0x01 << bit)) == 0) ? false : true;
}

/* ----------------------------- end of file -------------------------------- */
