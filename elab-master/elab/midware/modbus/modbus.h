
/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef MODBUS_H
#define MODBUS_H

/* Includes ------------------------------------------------------------------*/
#include "modbus_cfg.h"
#include "modbus_def.h"
#include "../../os/cmsis_os.h"
#include "../../edf/normal/elab_serial.h"

/* Exported typedef ----------------------------------------------------------*/
typedef struct mb_channel_cb
{
    bool (* coil_read)(uint16_t coil, uint16_t *perr);
    void (* coil_write)(uint16_t coil, bool coil_val, uint16_t *perr);
    bool (* di_read)(uint16_t di, uint16_t *perr);
    uint16_t (* in_reg_read)(uint16_t reg, uint16_t *perr);
    float (* in_reg_read_fp)(uint16_t reg, uint16_t *perr);

    uint16_t (* holding_reg_read)(uint16_t reg, uint16_t *perr);
    float (* holding_reg_read_fp)(uint16_t reg, uint16_t *perr);
    void (* holding_reg_write)(uint16_t reg, uint16_t reg_val_16, uint16_t *perr);
    void (* holding_reg_write_fp)(uint16_t reg, float reg_val_fp, uint16_t *perr);
} mb_channel_cb_t;

typedef struct mb_channel
{
    bool write_en;                              /* MODBUS writing enable */
    uint8_t node_addr;                          /* Node address of the channel */
    uint8_t mode;                               /* Modbus mode: ASCII or RTU */
    uint8_t m_or_s;                             /* Master or Slave */
    uint16_t error;                             /* Code error */

#if (MODBUS_CFG_RTU_EN != 0)
    uint16_t rtu_timeout;
    osThreadId_t thread_slave;
    mb_channel_cb_t cb;
#endif

    uint16_t rx_count;
    uint16_t rx_buff_byte_count;
    uint8_t *p_rx_buff;
    uint8_t rx_buff[MODBUS_CFG_BUF_SIZE];

    uint16_t tx_count;
    uint16_t tx_buff_byte_count;
    uint8_t *p_tx_buff;
    uint8_t tx_buff[MODBUS_CFG_BUF_SIZE];

    uint8_t rx_frame_data[MODBUS_CFG_BUF_SIZE];
    uint16_t rx_frame_ndata_bytes;
    uint16_t rx_frame_crc;
    uint16_t rx_frame_crc_calc;

    uint8_t tx_frame_data[MODBUS_CFG_BUF_SIZE];
    uint16_t tx_frame_ndata_bytes;
    uint16_t tx_frame_crc;

#if (MODBUS_CFG_MASTER_EN != 0)
    osMutexId_t mutex;
    uint16_t rx_timeout;                        /* rx timeout from slave */
    uint16_t size_expect;
#endif

    elab_device_t *serial;
} mb_channel_t;

/*
********************************************************************************
*                  MODBUS INTERFACE FUNCTION PROTOTYPES
*                            (mb.c)
********************************************************************************
*/

mb_channel_t *mb_channel_create(const char *serial,
                                uint8_t node_addr, uint8_t master_slave,
                                uint16_t rx_timeout, uint8_t modbus_mode);
void mb_channel_destroy(mb_channel_t *pch);
void mb_slave_set_cb(mb_channel_t *pch, mb_channel_cb_t *cb);
void mb_slave_write_enable(mb_channel_t *pch, bool status);

/*
********************************************************************************
*                   INTERFACE TO APPLICATION DATA
*                       (defined in mb_data.c)
********************************************************************************
*/

#if (MODBUS_CFG_FC01_EN != 0)
bool mb_coil_read(uint16_t coil, uint16_t *perr);
#endif

#if (MODBUS_CFG_FC05_EN != 0)
void mb_coil_write(uint16_t coil, bool coil_val, uint16_t *perr);
#endif

#if (MODBUS_CFG_FC02_EN != 0)
bool mb_di_read(uint16_t di, uint16_t *perr);
#endif

#if (MODBUS_CFG_FC04_EN != 0)
uint16_t mb_in_reg_read(uint16_t reg, uint16_t *perr);
float mb_in_reg_read_fp(uint16_t reg, uint16_t *perr);
#endif

#if (MODBUS_CFG_FC03_EN != 0)
uint16_t mb_holding_reg_read(uint16_t reg, uint16_t *perr);
float mb_holding_reg_read_fp(uint16_t reg, uint16_t *perr);
#endif

#if (MODBUS_CFG_FC06_EN != 0) || (MODBUS_CFG_FC16_EN != 0)
void mb_holding_reg_write(uint16_t reg, uint16_t reg_val_16, uint16_t *perr);
void mb_holding_reg_write_fp(uint16_t reg, float reg_val_fp, uint16_t *perr);
#endif

/*
********************************************************************************
*                             MODBUS MASTER
*                       GLOBAL FUNCTION PROTOTYPES
********************************************************************************
*/

#if (MODBUS_CFG_MASTER_EN != 0)

#if (MODBUS_CFG_FC01_EN != 0)
uint16_t mbm_fc01_coil_read(mb_channel_t *pch, uint8_t slave_addr,
                            uint16_t start_addr, uint8_t *p_coil_tbl,
                            uint16_t nbr_coils);
#endif

#if (MODBUS_CFG_FC02_EN != 0)
uint16_t mbm_fc02_di_read(mb_channel_t *pch, uint8_t slave_node,
                            uint16_t slave_addr,
                            uint8_t *p_di_tbl, uint16_t nbr_di);
#endif

#if (MODBUS_CFG_FC03_EN != 0)
uint16_t mbm_fc03_holding_reg_read(mb_channel_t *pch,
                                    uint8_t slave_node,
                                    uint16_t slave_addr,
                                    uint16_t *p_reg_tbl,
                                    uint16_t nbr_regs);
#endif

#if (MODBUS_CFG_FC03_EN != 0) && (MODBUS_CFG_FP_EN != 0)
uint16_t mbm_fc03_holding_reg_read_fp(mb_channel_t *pch,
                                        uint8_t slave_node,
                                        uint16_t slave_addr,
                                        float *p_reg_tbl,
                                        uint16_t nbr_regs);
#endif

#if (MODBUS_CFG_FC04_EN != 0)
uint16_t mbm_fc04_in_reg_read(mb_channel_t *pch, uint8_t slave_node,
                                uint16_t slave_addr, uint16_t  *p_reg_tbl,
                                uint16_t nbr_regs);
#endif

#if (MODBUS_CFG_FC04_EN != 0) && (MODBUS_CFG_FP_EN != 0)
uint16_t mbm_fc04_in_reg_read_fp(mb_channel_t *pch, uint8_t slave_node,
                                    uint16_t slave_addr, float *p_reg_tbl,
                                    uint16_t nbr_regs);
#endif

#if (MODBUS_CFG_FC05_EN != 0)
uint16_t mbm_fc05_wirte_coil(mb_channel_t *pch,
                                uint8_t slave_node, uint16_t slave_addr,
                                bool coil_val);
#endif

#if (MODBUS_CFG_FC06_EN != 0)
uint16_t mbm_fc06_holding_reg_write(mb_channel_t *pch, uint8_t slave_node,
                                    uint16_t slave_addr, uint16_t reg_val);
#endif

#if (MODBUS_CFG_FC06_EN != 0) && (MODBUS_CFG_FP_EN != 0)
uint16_t mbm_fc06_holding_reg_write_fp(mb_channel_t *pch,
                                        uint8_t slave_node,
                                        uint16_t slave_addr,
                                        float reg_val_fp);
#endif

#if (MODBUS_CFG_FC15_EN != 0)
uint16_t mbm_fc15_coil_write(mb_channel_t *pch, uint8_t slave_node,
                                uint16_t slave_addr,
                                uint8_t *p_coil_tbl, uint16_t nbr_coils);
#endif

#if (MODBUS_CFG_FC16_EN != 0)
uint16_t mbm_fc16_holding_reg_write(mb_channel_t *pch, uint8_t slave_node, 
                                    uint16_t slave_addr,
                                    uint16_t *p_reg_tbl, uint16_t nbr_regs);
#endif

#if (MODBUS_CFG_FC16_EN != 0) && (MODBUS_CFG_FP_EN != 0)
uint16_t mbm_fc16_holding_reg_write_fp(mb_channel_t *pch,
                                        uint8_t slave_node, uint16_t slave_addr,
                                        float *p_reg_tbl, uint16_t nbr_regs);
#endif

#endif

#endif /* MODBUS_H */

/* ----------------------------- end of file -------------------------------- */
