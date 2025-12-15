/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include <stdint.h>
#include <stdio.h>
#include "cmsis_os.h"
#include "simu_driver_serial.h"
#include "simu_driver_pin.h"
#include "elab_device.h"
#include "elab_pin.h"
#include "elab_assert.h"
#include "mb.h"
#include "elab_log.h"
#include "cmsis_os.h"

ELAB_TAG("main");

/* private functions -------------------------------------------------------- */
static void _entry_mb_master(void *parameter);

/* private functions -------------------------------------------------------- */
static bool _cb_coil_read(uint16_t coil, uint16_t *perr);
static void _cb_coil_write(uint16_t coil, bool coil_val, uint16_t *perr);
static bool _cb_di_read(uint16_t di, uint16_t *perr);
static uint16_t _cb_in_reg_read(uint16_t reg, uint16_t *perr);
static float _cb_in_reg_read_fp(uint16_t reg, uint16_t *perr);
static uint16_t _cb_holding_reg_read(uint16_t reg, uint16_t *perr);
static float _cb_holding_reg_read_fp(uint16_t reg, uint16_t *perr);
static void _cb_holding_reg_write(uint16_t reg, uint16_t reg_val_16, uint16_t *perr);
static void _cb_holding_reg_wr_fp(uint16_t reg, float reg_val_fp, uint16_t *perr);

/* private variables ---------------------------------------------------------*/
/**
 * @brief  The thread attribute for testing.
 */
static const osThreadAttr_t thread_attr_mb = 
{
    .name = "ThreadTest",
    .attr_bits = osThreadDetached,
    .priority = osPriorityNormal,
    .stack_size = 2048,
};

static elab_mb_channel_t *mb_ch_m = NULL;
static elab_mb_channel_t *mb_ch_s = NULL;

static elab_mb_channel_cb_t cb_mb =
{
    .coil_read      = _cb_coil_read,
    .coil_write     = _cb_coil_write,
    .di_read        = _cb_di_read,
    .in_reg_read    = _cb_in_reg_read,
    .in_reg_read_fp = _cb_in_reg_read_fp,

    .holding_reg_read       = _cb_holding_reg_read,
    .holding_reg_read_fp    = _cb_holding_reg_read_fp,
    .holding_reg_write      = _cb_holding_reg_write,
    .holding_reg_write_fp   = _cb_holding_reg_wr_fp,

    .file_read = NULL,
    .file_write = NULL,
};

/* public functions --------------------------------------------------------- */
/**
  * @brief  The main function.
  */
int main(int32_t argc, char **argv)
{
    elab_err_t ret = ELAB_OK;

    elog_level_set(ELOG_LEVEL_DEBUG);

    elab_mb_os_init();

    simu_serial_new_pair("uart1", "uart2");
    simu_io_new("P1.1", true);
    simu_io_new("P1.2", true);

    elab_mb_init(1000);
    
    mb_ch_m = elab_mb_config_channel(0, MODBUS_MASTER, 500, MODBUS_MODE_RTU,
                                        0, 115200, MODBUS_PARITY_NONE, 8, 1,
                                        MODBUS_WR_EN);

    mb_ch_s = elab_mb_config_channel(1, MODBUS_SLAVE, 200, MODBUS_MODE_RTU,
                                        1, 115200, MODBUS_PARITY_NONE, 8, 1,
                                        MODBUS_WR_EN);
    elab_mb_slave_set_cb(mb_ch_s, &cb_mb);

    osThreadNew(_entry_mb_master, NULL, &thread_attr_mb);
    osKernelStart();

    return 0;
}

/* private functions -------------------------------------------------------- */
void elab_mb_os_init(void);

static void _entry_mb_master(void *parameter)
{
    uint8_t value_coil = 0;
    uint8_t value_di = 0;
    uint16_t value_honging_reg[8];
    float value_honding_reg_f[8];
    uint16_t value_in_reg[8];
    uint16_t err_id = MODBUS_ERR_NONE;

    while (1)
    {
        /*
        err_id = MBM_FC01_CoilRd(mb_ch_m, 1, 1000, &value_coil, 8);
        elab_assert(err_id == MODBUS_ERR_NONE);
        printf(">> value_coil: 0x%02x.\n", value_coil);

        err_id = MBM_FC02_DIRd(mb_ch_m, 1, 1000, (uint8_t *)&value_di, 8);
        elab_assert(err_id == MODBUS_ERR_NONE);
        printf(">> value_di: 0x%02x.\n", value_di);

        err_id = MBM_FC03_HoldingRegRd(mb_ch_m, 1, 1000, value_honging_reg, 8);
        elab_assert(err_id == MODBUS_ERR_NONE);
        for (uint32_t i = 0; i < 8; i ++)
        {
            printf(">> value_hoilding_reg %u : %u.\n", (1000 + i), value_honging_reg[i]);
        }
        printf("\n");
       
        err_id = MBM_FC03_HoldingRegRdFP(mb_ch_m, 1, 5000, value_honding_reg_f, 8);
        elab_assert(err_id == MODBUS_ERR_NONE);
        for (uint32_t i = 0; i < 8; i ++)
        {
            printf(">> value_honding_reg_f %u : %f.\n", (5000 + i), value_honding_reg_f[i]);
        }
        printf("\n");

        err_id = MBM_FC04_InRegRd(mb_ch_m, 1, 1000, value_in_reg, 8);
        elab_assert(err_id == MODBUS_ERR_NONE);
        for (uint32_t i = 0; i < 8; i ++)
        {
            printf(">> value_in_reg %u : %u.\n", (1000 + i), value_in_reg[i]);
        }
        printf("\n");
        
        err_id = elab_mbm_fc05_wirte_coil(mb_ch_m, 1, 1000, true);
        elab_assert(err_id == MODBUS_ERR_NONE);
        


        err_id = MBM_FC06_HoldingRegWrFP(mb_ch_m, 1, 5024, 123.45);
        elab_assert(err_id == MODBUS_ERR_NONE);

        uint8_t coil_value = 0xff;
        err_id = MBM_FC15_CoilWr(mb_ch_m, 1, 1000, &coil_value, 4);
        elab_assert(err_id == MODBUS_ERR_NONE);

        uint16_t reg_table[8] =
        {
            12341, 12342, 12343, 12344, 12345, 12346, 12347, 12348, 
        };
        err_id = MBM_FC16_HoldingRegWrN(mb_ch_m, 1, 1000, reg_table, 1);
        elab_assert(err_id == MODBUS_ERR_NONE);
        */

        err_id = MBM_FC06_HoldingRegWr(mb_ch_m, 1, 1024, 12345);
        elab_assert(err_id == MODBUS_ERR_NONE);

        osDelay(1000);
    }
}

/* private functions -------------------------------------------------------- */
static bool _cb_coil_read(uint16_t coil, uint16_t *perr)
{
    *perr = MODBUS_ERR_NONE;
    return (0x39 & (1 << (coil - 1000))) ? true : false;
}

static void _cb_coil_write(uint16_t coil, bool coil_val, uint16_t *perr)
{
    *perr = MODBUS_ERR_NONE;
    printf("coil %u value: %d.\n", coil, coil_val);
}

static bool _cb_di_read(uint16_t di, uint16_t *perr)
{
    *perr = MODBUS_ERR_NONE;

    return (0x68 & (1 << (di - 1000))) ? true : false;
}

static uint16_t _cb_in_reg_read(uint16_t reg, uint16_t *perr)
{
    *perr = MODBUS_ERR_NONE;

    return (reg + 300);
}

static float _cb_in_reg_read_fp(uint16_t reg, uint16_t *perr)
{

}

static uint16_t _cb_holding_reg_read(uint16_t reg, uint16_t *perr)
{
    *perr = MODBUS_ERR_NONE;

    return (reg + 200);
}

static float _cb_holding_reg_read_fp(uint16_t reg, uint16_t *perr)
{
    *perr = MODBUS_ERR_NONE;

    return ((float)reg + 0.666);
}

static void _cb_holding_reg_write(uint16_t reg, uint16_t reg_val_16, uint16_t *perr)
{
    *perr = MODBUS_ERR_NONE;
    printf("HoldingRegWr %u value: %u.\n", reg, reg_val_16);
}

static void _cb_holding_reg_wr_fp(uint16_t reg, float reg_val_fp, uint16_t *perr)
{
    *perr = MODBUS_ERR_NONE;
    printf("MB_HoldingRegWrFP %u value: %f.\n", reg, reg_val_fp);
}

static uint16_t _cb_file_read(uint16_t file_nbr, uint16_t record_nbr, uint16_t ix,
                                uint8_t record_len, uint16_t *perr)
{
    
}

static void _cb_file_write(uint16_t file_nbr, uint16_t record_nbr, uint16_t ix,
                            uint8_t record_len, uint16_t value, uint16_t *perr)
{
    
}

/* ----------------------------- end of file -------------------------------- */
