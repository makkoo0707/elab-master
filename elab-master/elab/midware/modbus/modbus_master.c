/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* Includes ------------------------------------------------------------------*/
#include "modbus.h"
#include "../../common/elab_assert.h"

ELAB_TAG("ModbusMaster");

#if (MODBUS_CFG_MASTER_EN != 0)

/* Private config ------------------------------------------------------------*/
#define MBM_TX_FRAME                            (&pch->TxFrame)
#define MBM_TX_FRAME_NBYTES                     (pch->tx_frame_ndata_bytes)

#define MBM_TX_FRAME_SLAVE_ADDR                 (pch->tx_frame_data[0])
#define MBM_TX_FRAME_FC                         (pch->tx_frame_data[1])

#define MBM_TX_FRAME_FC01_ADDR_HI               (pch->tx_frame_data[2])
#define MBM_TX_FRAME_FC01_ADDR_LO               (pch->tx_frame_data[3])
#define MBM_TX_FRAME_FC01_NBR_POINTS_HI         (pch->tx_frame_data[4])
#define MBM_TX_FRAME_FC01_NBR_POINTS_LO         (pch->tx_frame_data[5])

#define MBM_TX_FRAME_FC02_ADDR_HI               (pch->tx_frame_data[2])
#define MBM_TX_FRAME_FC02_ADDR_LO               (pch->tx_frame_data[3])
#define MBM_TX_FRAME_FC02_NBR_POINTS_HI         (pch->tx_frame_data[4])
#define MBM_TX_FRAME_FC02_NBR_POINTS_LO         (pch->tx_frame_data[5])

#define MBM_TX_FRAME_FC03_ADDR_HI               (pch->tx_frame_data[2])
#define MBM_TX_FRAME_FC03_ADDR_LO               (pch->tx_frame_data[3])
#define MBM_TX_FRAME_FC03_NBR_POINTS_HI         (pch->tx_frame_data[4])
#define MBM_TX_FRAME_FC03_NBR_POINTS_LO         (pch->tx_frame_data[5])

#define MBM_TX_FRAME_FC04_ADDR_HI               (pch->tx_frame_data[2])
#define MBM_TX_FRAME_FC04_ADDR_LO               (pch->tx_frame_data[3])
#define MBM_TX_FRAME_FC04_NBR_POINTS_HI         (pch->tx_frame_data[4])
#define MBM_TX_FRAME_FC04_NBR_POINTS_LO         (pch->tx_frame_data[5])

#define MBM_TX_FRAME_FC05_ADDR_HI               (pch->tx_frame_data[2])
#define MBM_TX_FRAME_FC05_ADDR_LO               (pch->tx_frame_data[3])
#define MBM_TX_FRAME_FC05_FORCE_DATA_HI         (pch->tx_frame_data[4])
#define MBM_TX_FRAME_FC05_FORCE_DATA_LO         (pch->tx_frame_data[5])

#define MBM_TX_FRAME_FC06_ADDR_HI               (pch->tx_frame_data[2])
#define MBM_TX_FRAME_FC06_ADDR_LO               (pch->tx_frame_data[3])
#define MBM_TX_FRAME_FC06_DATA_ADDR            (&pch->tx_frame_data[4])
#define MBM_TX_FRAME_FC06_DATA_HI               (pch->tx_frame_data[4])
#define MBM_TX_FRAME_FC06_DATA_LO               (pch->tx_frame_data[5])

#define MBM_TX_FRAME_FC15_ADDR_HI               (pch->tx_frame_data[2])
#define MBM_TX_FRAME_FC15_ADDR_LO               (pch->tx_frame_data[3])
#define MBM_TX_FRAME_FC15_NBR_POINTS_HI         (pch->tx_frame_data[4])
#define MBM_TX_FRAME_FC15_NBR_POINTS_LO         (pch->tx_frame_data[5])
#define MBM_TX_FRAME_FC15_BYTE_CNT              (pch->tx_frame_data[6])
#define MBM_TX_FRAME_FC15_DATA                 (&pch->tx_frame_data[7])

#define MBM_TX_FRAME_FC16_ADDR_HI               (pch->tx_frame_data[2])
#define MBM_TX_FRAME_FC16_ADDR_LO               (pch->tx_frame_data[3])
#define MBM_TX_FRAME_FC16_NBR_REGS_HI           (pch->tx_frame_data[4])
#define MBM_TX_FRAME_FC16_NBR_REGS_LO           (pch->tx_frame_data[5])
#define MBM_TX_FRAME_FC16_BYTE_CNT              (pch->tx_frame_data[6])
#define MBM_TX_FRAME_FC16_DATA                 (&pch->tx_frame_data[7])


#define MBM_TX_FRAME_DIAG_FNCT_HI               (pch->tx_frame_data[2])
#define MBM_TX_FRAME_DIAG_FNCT_LO               (pch->tx_frame_data[3])
#define MBM_TX_FRAME_DIAG_FNCT_DATA_HI          (pch->tx_frame_data[4])
#define MBM_TX_FRAME_DIAG_FNCT_DATA_LO          (pch->tx_frame_data[5])

/*
********************************************************************************
*                     LOCAL FUNCTION PROTOTYPES
********************************************************************************
*/

#if (MODBUS_CFG_FC01_EN != 0) || (MODBUS_CFG_FC02_EN != 0)
static uint16_t mbm_coil_di_read_resp(mb_channel_t *pch, uint8_t *ptbl);
#endif

#if (MODBUS_CFG_FC03_EN != 0) || (MODBUS_CFG_FC04_EN != 0)
static uint16_t mbm_reg_read_resp(mb_channel_t *pch, uint16_t *ptbl);
#endif

#if (MODBUS_CFG_FC03_EN != 0) && (MODBUS_CFG_FP_EN   != 0)
static uint16_t mbm_reg_read_fp_resp(mb_channel_t *pch, float *ptbl);
#endif

#if (MODBUS_CFG_FC05_EN != 0)
static uint16_t mbm_coil_write_resp(mb_channel_t *pch);
#endif

#if (MODBUS_CFG_FC15_EN != 0)
static uint16_t mbm_coil_write_n_resp(mb_channel_t *pch);
#endif

#if (MODBUS_CFG_FC06_EN != 0)
static uint16_t mbm_reg_write_resp(mb_channel_t *pch);
#endif

#if (MODBUS_CFG_FC16_EN != 0)
static uint16_t mbm_reg_write_n_resp(mb_channel_t *pch);
#endif

#if (MODBUS_CFG_ASCII_EN != 0)
void mb_ascii_rx_byte(mb_channel_t *pch, uint8_t rx_byte);
bool mb_ascii_rx(mb_channel_t *pch);
void mb_ascii_tx(mb_channel_t *pch);
#endif

#if (MODBUS_CFG_RTU_EN != 0)
void mb_rtu_rx_byte(mb_channel_t *pch, uint8_t rx_byte);
bool mb_rtu_rx(mb_channel_t *pch);
void mb_rtu_tx(mb_channel_t *pch);
#endif

static bool mbm_rx_reply(mb_channel_t *pch);
static void mbm_tx_cmd(mb_channel_t *pch);
void mb_rx_byte(mb_channel_t *pch, uint8_t rx_byte);
void mb_rx_task(mb_channel_t*pch);
void mb_tx(mb_channel_t *pch);
void mb_tx_byte(mb_channel_t *pch);
uint16_t mb_rtu_tx_calc_crc(mb_channel_t *pch);
uint16_t mbm_rx_blocking(mb_channel_t *pch);

/*
********************************************************************************
*                              mbm_fc01_coil_read()
*
* Description : Sends a MODBUS message to read the status of coils from a slave unit.
*
* Argument(s) : pch              Is a pointer to the Modbus channel to send the request to.
*
*               slave_node       Is the Modbus node number of the desired slave to obtain the information from.
*
*               slave_addr       Is the Modbus coil start address
*
*               p_coil_tbl       Is a pointer to an array of bytes containing the value of the coils read.  The
*                                format is:
*                                                MSB                               LSB
*                                                B7   B6   B5   B4   B3   B2   B1   B0
*                                                -------------------------------------
*                                p_coil_tbl[0]   #8   #7                            #1
*                                p_coil_tbl[1]   #16  #15                           #9
*                                     :
*                                     :
*
*                                Note that the array that will be receiving the coil values must be greater
*                                than or equal to:   (nbr_coils - 1) / 8 + 1
*
*               nbr_coils        Is the desired number of coils to read
*
* Return(s)   : MODBUS_ERR_NONE          If the function was sucessful.
*               MODBUS_ERR_RX            If a timeout occurred before receiving a response from the slave.
*               MODBUS_ERR_SLAVE_ADDR    If the transmitted slave address doesn't correspond to the received slave address
*               MODBUS_ERR_FC            If the transmitted function code doesn't correspond to the received function code
*               MODBUS_ERR_BYTE_COUNT    If the expected number of bytes to receive doesn't correspond to the number of bytes received
*
* Caller(s)   : Application.
*
* Note(s)     : none.
********************************************************************************
*/

#if (MODBUS_CFG_FC01_EN != 0)
uint16_t mbm_fc01_coil_read(mb_channel_t *pch,
                            uint8_t slave_addr,
                            uint16_t start_addr,
                            uint8_t *p_coil_tbl,
                            uint16_t nbr_coils)
{
    elab_assert(pch != NULL);
    elab_assert(p_coil_tbl != NULL);
    elab_assert(nbr_coils > 0 && nbr_coils <= 2000);
    elab_assert(slave_addr > 0 && slave_addr <= MODBUS_NODE_ADDR_MAX);
    elab_assert((uint32_t)(start_addr + (nbr_coils - 1) / 8) <= UINT16_MAX);

    osStatus_t ret_os = osOK;
    ret_os = osMutexAcquire(pch->mutex, osWaitForever);
    elab_assert(ret_os == osOK);

    MBM_TX_FRAME_NBYTES             = 4;
    MBM_TX_FRAME_SLAVE_ADDR         = slave_addr;                               /* Slave Address                     */
    MBM_TX_FRAME_FC                 = 1;                                        /* Function Code                     */
    MBM_TX_FRAME_FC01_ADDR_HI       = (uint8_t)((start_addr >> 8) & 0x00FF); /* Staring Address                   */
    MBM_TX_FRAME_FC01_ADDR_LO       = (uint8_t) (start_addr       & 0x00FF);
    MBM_TX_FRAME_FC01_NBR_POINTS_HI = (uint8_t)((nbr_coils  >> 8) & 0x00FF); /* Number of points                  */
    MBM_TX_FRAME_FC01_NBR_POINTS_LO = (uint8_t) (nbr_coils        & 0x00FF);
    if (pch->mode == MODBUS_MODE_ASCII)
    {
        pch->size_expect = MODBUS_ASCII_MIN_MSG_SIZE + ((nbr_coils - 1) / 8 + 1) * 2;
    }
    else
    {
        pch->size_expect = 5 + ((nbr_coils - 1) / 8 + 1);
    }

    /* Send command and wait for response from slave. */
    mbm_tx_cmd(pch);
    uint16_t err = mbm_rx_blocking(pch);

    if (err == MODBUS_ERR_NONE)
    {
        bool ok = mbm_rx_reply(pch);
        if (ok == true)
        {
            /* Parse the response from the slave */
            err = mbm_coil_di_read_resp(pch, p_coil_tbl);
        }
        else
        {
            err = MODBUS_ERR_RX;
        }
    }

    pch->rx_buff_byte_count = 0;
    pch->p_rx_buff = &pch->rx_buff[0];

    ret_os = osMutexRelease(pch->mutex);
    elab_assert(ret_os == osOK);

    return (err);
}
#endif

/*
********************************************************************************
*                                             mbm_fc02_di_read()
*
* Description : Sends a MODBUS message to read the status of discrete inputs from a slave unit.
*
* Argument(s) : pch              Is a pointer to the Modbus channel to send the request to.
*
*               slave_node       Is the Modbus node number of the desired slave to obtain the information from.
*
*               slave_addr       Is the Modbus discrete input start address
*
*               p_di_tbl         Is a pointer to an array that will receive the state of the desired discrete inputs.
*                                The format of the array is as follows:
*
*                                              MSB                               LSB
*                                              B7   B6   B5   B4   B3   B2   B1   B0
*                                              -------------------------------------
*                                p_di_tbl[0]   #8   #7                            #1
*                                p_di_tbl[1]   #16  #15                           #9
*                                     :
*                                     :
*
*                                Note that the array that will be receiving the discrete input values must be greater
*                                than or equal to:   (nbr_di - 1) / 8 + 1
*
*               nbr_di           Is the desired number of discrete inputs to read
*
* Return(s)   : MODBUS_ERR_NONE          If the function was sucessful.
*               MODBUS_ERR_RX            If a timeout occurred before receiving a response from the slave.
*               MODBUS_ERR_SLAVE_ADDR    If the transmitted slave address doesn't correspond to the received slave address
*               MODBUS_ERR_FC            If the transmitted function code doesn't correspond to the received function code
*               MODBUS_ERR_BYTE_COUNT    If the expected number of bytes to receive doesn't correspond to the number of bytes received
*
* Caller(s)   : Application.
*
* Note(s)     : none.
********************************************************************************
*/

#if (MODBUS_CFG_FC02_EN != 0)
uint16_t mbm_fc02_di_read(mb_channel_t *pch,
                            uint8_t slave_node,
                            uint16_t slave_addr,
                            uint8_t *p_di_tbl,
                            uint16_t nbr_di)
{
    elab_assert(pch != NULL);
    elab_assert(p_di_tbl != NULL);
    elab_assert(nbr_di > 0 && nbr_di <= 2000);
    elab_assert(slave_node > 0 && slave_node <= MODBUS_NODE_ADDR_MAX);
    elab_assert((uint32_t)(slave_addr + (nbr_di - 1) / 8) <= UINT16_MAX);

    osStatus_t ret_os = osOK;
    ret_os = osMutexAcquire(pch->mutex, osWaitForever);
    elab_assert(ret_os == osOK);

    MBM_TX_FRAME_NBYTES             = 4;
    MBM_TX_FRAME_SLAVE_ADDR         = slave_node;                               /* Setup command                     */
    MBM_TX_FRAME_FC                 = 2;
    MBM_TX_FRAME_FC02_ADDR_HI       = (uint8_t)((slave_addr >> 8) & 0x00FF);
    MBM_TX_FRAME_FC02_ADDR_LO       = (uint8_t) (slave_addr       & 0x00FF);
    MBM_TX_FRAME_FC02_NBR_POINTS_HI = (uint8_t)((nbr_di     >> 8) & 0x00FF);
    MBM_TX_FRAME_FC02_NBR_POINTS_LO = (uint8_t) (nbr_di           & 0x00FF);
    if (pch->mode == MODBUS_MODE_ASCII)
    {
        pch->size_expect = MODBUS_ASCII_MIN_MSG_SIZE + ((nbr_di - 1) / 8 + 1) * 2;
    }
    else
    {
        pch->size_expect = 5 + ((nbr_di - 1) / 8 + 1);
    }

    /* Send command and wait for response from slave. */
    mbm_tx_cmd(pch);
    uint16_t err = mbm_rx_blocking(pch);

    if (err == MODBUS_ERR_NONE)
    {
        bool ok = mbm_rx_reply(pch);
        if (ok == true)
        {
            /* Parse the response from the slave */
            err = mbm_coil_di_read_resp(pch, p_di_tbl);
        }
        else
        {
            err = MODBUS_ERR_RX;
        }
    }

    pch->rx_buff_byte_count = 0;
    pch->p_rx_buff = &pch->rx_buff[0];

    ret_os = osMutexRelease(pch->mutex);
    elab_assert(ret_os == osOK);

    return (err);
}
#endif

/*
********************************************************************************
*                                       mbm_fc03_holding_reg_read()
*
* Description : Sends a MODBUS message to read the value of holding registers from a slave unit.
*
* Argument(s) : pch              Is a pointer to the Modbus channel to send the request to.
*
*               slave_node       Is the Modbus node number of the desired slave to obtain the information from.
*
*               slave_addr       Is the Modbus holding register start address
*
*               p_reg_tbl        Is a pointer to an array of integers that will receive the current value of
*                                the desired holding registers from the slave.  The array pointed to by
*                                'p_reg_tbl' needs to be able to hold at least 'nbr_regs' entries.
*
*               nbr_regs         Is the desired number of holding registers to read
*
* Return(s)   : MODBUS_ERR_NONE          If the function was sucessful.
*               MODBUS_ERR_RX            If a timeout occurred before receiving a response from the slave.
*               MODBUS_ERR_SLAVE_ADDR    If the transmitted slave address doesn't correspond to the received slave address
*               MODBUS_ERR_FC            If the transmitted function code doesn't correspond to the received function code
*               MODBUS_ERR_BYTE_COUNT    If the expected number of bytes to receive doesn't correspond to the number of bytes received
*
* Caller(s)   : Application.
*
* Note(s)     : none.
********************************************************************************
*/

#if (MODBUS_CFG_FC03_EN != 0)
uint16_t mbm_fc03_holding_reg_read(mb_channel_t *pch,
                                    uint8_t slave_node,
                                    uint16_t slave_addr,
                                    uint16_t *p_reg_tbl,
                                    uint16_t nbr_regs)
{
    elab_assert(pch != NULL);
    elab_assert(p_reg_tbl != NULL);
    elab_assert(nbr_regs > 0 && nbr_regs <= 125);
    elab_assert(slave_node > 0 && slave_node <= MODBUS_NODE_ADDR_MAX);
    elab_assert(slave_addr < MODBUS_CFG_FP_START_IX);

    osStatus_t ret_os = osOK;
    ret_os = osMutexAcquire(pch->mutex, osWaitForever);
    elab_assert(ret_os == osOK);

    MBM_TX_FRAME_NBYTES             = 4;
    MBM_TX_FRAME_SLAVE_ADDR         = slave_node;                               /* Setup command                     */
    MBM_TX_FRAME_FC                 = 3;
    MBM_TX_FRAME_FC03_ADDR_HI       = (uint8_t)((slave_addr >> 8) & 0x00FF);
    MBM_TX_FRAME_FC03_ADDR_LO       = (uint8_t) (slave_addr       & 0x00FF);
    MBM_TX_FRAME_FC03_NBR_POINTS_HI = (uint8_t)((nbr_regs   >> 8) & 0x00FF);
    MBM_TX_FRAME_FC03_NBR_POINTS_LO = (uint8_t) (nbr_regs         & 0x00FF);
    pch->size_expect = 5 + nbr_regs * 2;

    if (pch->mode == MODBUS_MODE_ASCII)
    {
        pch->size_expect = MODBUS_ASCII_MIN_MSG_SIZE + nbr_regs * 2 * 2;
    }
    else
    {
        pch->size_expect = 5 + nbr_regs * 2;
    }

    /* Send command and wait for response from slave. */
    mbm_tx_cmd(pch);
    uint16_t err = mbm_rx_blocking(pch);

    if (err == MODBUS_ERR_NONE) {
        bool ok = mbm_rx_reply(pch);
        if (ok == true) {
            err = mbm_reg_read_resp(pch,                                           /* Parse the response from the slave */
                                 p_reg_tbl);
        } else {
            err = MODBUS_ERR_RX;
        }
    }

    pch->rx_buff_byte_count = 0;
    pch->p_rx_buff     = &pch->rx_buff[0];

    ret_os = osMutexRelease(pch->mutex);
    elab_assert(ret_os == osOK);

    return (err);
}
#endif

/*
********************************************************************************
*                                       mbm_fc03_holding_reg_read_fp()
*
* Description : Sends a MODBUS message to read the value of floating-point holding registers from a slave unit.
*
* Argument(s) : pch              Is a pointer to the Modbus channel to send the request to.
*
*               slave_node       Is the Modbus node number of the desired slave to obtain the information from.
*
*               slave_addr       Is the Modbus floating-point holding register start address
*
*               p_reg_tbl        Is a pointer to an array of floating-points that will receive the current
*                                value of the desired holding registers from the slave.  The array pointed to
*                                by 'p_reg_tbl' needs to be able to hold at least 'nbr_regs' entries.
*
*               nbr_regs         Is the desired number of holding registers to read
*
* Return(s)   : MODBUS_ERR_NONE  If the function was sucessful.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
********************************************************************************
*/

#if (MODBUS_CFG_FC03_EN != 0) && (MODBUS_CFG_FP_EN   != 0)
uint16_t mbm_fc03_holding_reg_read_fp(mb_channel_t *pch,
                                        uint8_t slave_node,
                                        uint16_t slave_addr,
                                        float *p_reg_tbl,
                                        uint16_t nbr_regs)
{
    elab_assert(pch != NULL);
    elab_assert(p_reg_tbl != NULL);
    elab_assert(nbr_regs > 0 && nbr_regs <= 62);
    elab_assert(slave_node > 0 && slave_node <= MODBUS_NODE_ADDR_MAX);
    elab_assert(slave_addr >= MODBUS_CFG_FP_START_IX);

    osStatus_t ret_os = osOK;
    ret_os = osMutexAcquire(pch->mutex, osWaitForever);
    elab_assert(ret_os == osOK);

    MBM_TX_FRAME_NBYTES             = 4;
    MBM_TX_FRAME_SLAVE_ADDR         = slave_node;                               /* Setup command                     */
    MBM_TX_FRAME_FC                 = 3;
    MBM_TX_FRAME_FC03_ADDR_HI       = (uint8_t)((slave_addr >> 8) & 0x00FF);
    MBM_TX_FRAME_FC03_ADDR_LO       = (uint8_t) (slave_addr       & 0x00FF);
    MBM_TX_FRAME_FC03_NBR_POINTS_HI = (uint8_t)((nbr_regs   >> 8) & 0x00FF);
    MBM_TX_FRAME_FC03_NBR_POINTS_LO = (uint8_t) (nbr_regs         & 0x00FF);

    if (pch->mode == MODBUS_MODE_ASCII)
    {
        pch->size_expect = MODBUS_ASCII_MIN_MSG_SIZE + nbr_regs * 4 * 2;
    }
    else
    {
        pch->size_expect = 5 + nbr_regs * 4;
    }

    /* Send command and wait for response from slave. */
    mbm_tx_cmd(pch);
    uint16_t err = mbm_rx_blocking(pch);

    if (err == MODBUS_ERR_NONE)
    {
        /* Parse the response from the slave */
        err = mbm_rx_reply(pch) ?
                mbm_reg_read_fp_resp(pch, p_reg_tbl) : MODBUS_ERR_RX;
    }

    pch->rx_buff_byte_count = 0;
    pch->p_rx_buff = &pch->rx_buff[0];

    ret_os = osMutexRelease(pch->mutex);
    elab_assert(ret_os == osOK);

    return (err);
}
#endif


/*
********************************************************************************
*                                          mbm_fc04_in_reg_read()
*
* Description : Sends a MODBUS message to read the value of input registers from a slave unit.
*
* Argument(s) : pch              Is a pointer to the Modbus channel to send the request to.
*
*               slave_node       Is the Modbus node number of the desired slave to obtain the information from.
*
*               slave_addr       Is the Modbus input register start address
*
*               p_reg_tbl        Is a pointer to an array of integers that will receive the current value of
*                                the desired holding registers from the slave.  The array pointed to by
*                                'p_reg_tbl' needs to be able to hold at least 'nbr_regs' entries.
*
*               nbr_regs         Is the desired number of input registers to read
*
* Return(s)   : MODBUS_ERR_NONE          If the function was sucessful.
*               MODBUS_ERR_RX            If a timeout occurred before receiving a response from the slave.
*               MODBUS_ERR_SLAVE_ADDR    If the transmitted slave address doesn't correspond to the received slave address
*               MODBUS_ERR_FC            If the transmitted function code doesn't correspond to the received function code
*               MODBUS_ERR_BYTE_COUNT    If the expected number of bytes to receive doesn't correspond to the number of bytes received
*
* Caller(s)   : Application.
*
* Note(s)     : none.
********************************************************************************
*/

#if (MODBUS_CFG_FC04_EN != 0)
uint16_t mbm_fc04_in_reg_read(mb_channel_t *pch,
                              uint8_t slave_node,
                              uint16_t slave_addr,
                              uint16_t *p_reg_tbl,
                              uint16_t nbr_regs)
{
    elab_assert(pch != NULL);
    elab_assert(p_reg_tbl != NULL);
    elab_assert(nbr_regs > 0 && nbr_regs <= 125);
    elab_assert(slave_node > 0 && slave_node <= MODBUS_NODE_ADDR_MAX);
    elab_assert(slave_addr < MODBUS_CFG_FP_START_IX);

    osStatus_t ret_os = osOK;
    ret_os = osMutexAcquire(pch->mutex, osWaitForever);
    elab_assert(ret_os == osOK);

    bool ok;

    MBM_TX_FRAME_NBYTES             = 4;
    MBM_TX_FRAME_SLAVE_ADDR         = slave_node;                               /* Setup command                     */
    MBM_TX_FRAME_FC                 = 4;
    MBM_TX_FRAME_FC04_ADDR_HI       = (uint8_t)((slave_addr >> 8) & 0x00FF);
    MBM_TX_FRAME_FC04_ADDR_LO       = (uint8_t) (slave_addr       & 0x00FF);
    MBM_TX_FRAME_FC04_NBR_POINTS_HI = (uint8_t)((nbr_regs   >> 8) & 0x00FF);
    MBM_TX_FRAME_FC04_NBR_POINTS_LO = (uint8_t) (nbr_regs         & 0x00FF);

    if (pch->mode == MODBUS_MODE_ASCII)
    {
        pch->size_expect = MODBUS_ASCII_MIN_MSG_SIZE + nbr_regs * 2 * 2;
    }
    else
    {
        pch->size_expect = 5 + nbr_regs * 2;
    }

    /* Send command and wait for response from slave. */
    mbm_tx_cmd(pch);
    uint16_t err = mbm_rx_blocking(pch);

    if (err == MODBUS_ERR_NONE)
    {
        ok = mbm_rx_reply(pch);
        if (ok == true)
        {
            /* Parse the response from the slave */
            err = mbm_reg_read_resp(pch, p_reg_tbl);
        }
        else
        {
            err = MODBUS_ERR_RX;
        }
    }

    pch->rx_buff_byte_count = 0;
    pch->p_rx_buff = &pch->rx_buff[0];

    ret_os = osMutexRelease(pch->mutex);
    elab_assert(ret_os == osOK);

    return (err);
}
#endif

#if (MODBUS_CFG_FC04_EN != 0) && (MODBUS_CFG_FP_EN != 0)
uint16_t mbm_fc04_in_reg_read_fp(mb_channel_t *pch, uint8_t slave_node,
                                    uint16_t slave_addr, float *p_reg_tbl,
                                    uint16_t nbr_regs)
{
    elab_assert(pch != NULL);
    elab_assert(p_reg_tbl != NULL);
    elab_assert(nbr_regs > 0 && nbr_regs <= 62);
    elab_assert(slave_node > 0 && slave_node <= MODBUS_NODE_ADDR_MAX);
    elab_assert(slave_addr >= MODBUS_CFG_FP_START_IX);

    osStatus_t ret_os = osOK;
    ret_os = osMutexAcquire(pch->mutex, osWaitForever);
    elab_assert(ret_os == osOK);

    MBM_TX_FRAME_NBYTES             = 4;
    MBM_TX_FRAME_SLAVE_ADDR         = slave_node;                               /* Setup command                     */
    MBM_TX_FRAME_FC                 = 4;
    MBM_TX_FRAME_FC04_ADDR_HI       = (uint8_t)((slave_addr >> 8) & 0x00FF);
    MBM_TX_FRAME_FC04_ADDR_LO       = (uint8_t) (slave_addr       & 0x00FF);
    MBM_TX_FRAME_FC04_NBR_POINTS_HI = (uint8_t)((nbr_regs   >> 8) & 0x00FF);
    MBM_TX_FRAME_FC04_NBR_POINTS_LO = (uint8_t) (nbr_regs         & 0x00FF);

    if (pch->mode == MODBUS_MODE_ASCII)
    {
        pch->size_expect = MODBUS_ASCII_MIN_MSG_SIZE + nbr_regs * 4 * 2;
    }
    else
    {
        pch->size_expect = 5 + nbr_regs * 4;
    }

    /* Send command and wait for response from slave. */
    mbm_tx_cmd(pch);
    uint16_t err = mbm_rx_blocking(pch);

    if (err == MODBUS_ERR_NONE)
    {
        bool ok = mbm_rx_reply(pch);
        if (ok == true)
        {
            /* Parse the response from the slave */
            err = mbm_reg_read_fp_resp(pch, p_reg_tbl);
        }
        else
        {
            err = MODBUS_ERR_RX;
        }
    }

    pch->rx_buff_byte_count = 0;
    pch->p_rx_buff = &pch->rx_buff[0];

    ret_os = osMutexRelease(pch->mutex);
    elab_assert(ret_os == osOK);

    return (err);
}
#endif

/*
********************************************************************************
*                                              mbm_fc05_wirte_coil()
*
* Description : Sends a MODBUS message to write the value of single coil to a slave unit.
*
* Argument(s) : pch              Is a pointer to the Modbus channel to send the request to.
*
*               slave_node       Is the Modbus node number of the desired slave to obtain the information from.
*
*               slave_addr       Is the Modbus coil to change
*
*               coil_val         Is the desired value of the coil:
*                                   MODBUS_COIL_ON   to turn the coil on.
*                                   false  to turn the coil off.
*
* Return(s)   : MODBUS_ERR_NONE          If the function was sucessful.
*               MODBUS_ERR_RX            If a timeout occurred before receiving a response from the slave.
*               MODBUS_ERR_SLAVE_ADDR    If the transmitted slave address doesn't correspond to the received slave address
*               MODBUS_ERR_FC            If the transmitted function code doesn't correspond to the received function code
*               MODBUS_ERR_BYTE_COUNT    If the expected number of bytes to receive doesn't correspond to the number of bytes received
*               MODBUS_ERR_COIL_ADDR     If you specified an invalid coil address
*
* Caller(s)   : Application.
*
* Note(s)     : none.
********************************************************************************
*/

#if (MODBUS_CFG_FC05_EN != 0)
uint16_t mbm_fc05_wirte_coil(mb_channel_t *pch,
                                    uint8_t slave_node,
                                    uint16_t slave_addr,
                                    bool coil_val)
{
    elab_assert(pch != NULL);
    elab_assert(slave_node > 0 && slave_node <= MODBUS_NODE_ADDR_MAX);

    osStatus_t ret_os = osOK;
    ret_os = osMutexAcquire(pch->mutex, osWaitForever);
    elab_assert(ret_os == osOK);


    MBM_TX_FRAME_NBYTES        = 4;
    MBM_TX_FRAME_SLAVE_ADDR    = slave_node;                                    /* Setup command                     */
    MBM_TX_FRAME_FC            = 5;
    MBM_TX_FRAME_FC05_ADDR_HI  = (uint8_t)((slave_addr >> 8) & 0x00FF);
    MBM_TX_FRAME_FC05_ADDR_LO  = (uint8_t) (slave_addr       & 0x00FF);
    if (coil_val == false)
    {
        MBM_TX_FRAME_FC05_FORCE_DATA_HI = (uint8_t)0x00;
        MBM_TX_FRAME_FC05_FORCE_DATA_LO = (uint8_t)0x00;
    }
    else
    {
        MBM_TX_FRAME_FC05_FORCE_DATA_HI = (uint8_t)0xFF;
        MBM_TX_FRAME_FC05_FORCE_DATA_LO = (uint8_t)0x00;
    }
    pch->size_expect = pch->mode == MODBUS_MODE_ASCII ? 17 : 8;

    /* Send command and wait for response from slave. */
    mbm_tx_cmd(pch);
    uint16_t err = mbm_rx_blocking(pch);

    if (err == MODBUS_ERR_NONE)
    {
        bool ok = mbm_rx_reply(pch);
        if (ok == true)
        {
            /* Parse the response from the slave */
            err = mbm_coil_write_resp(pch);
        }
        else
        {
            err = MODBUS_ERR_RX;
        }
    }

    pch->rx_buff_byte_count = 0;
    pch->p_rx_buff = &pch->rx_buff[0];

    ret_os = osMutexRelease(pch->mutex);
    elab_assert(ret_os == osOK);

    return (err);
}
#endif


/*
********************************************************************************
*                                          mbm_fc06_holding_reg_write()
*
* Description : Sends a MODBUS message to write the value of single holding register to a slave unit.
*
* Argument(s) : pch              Is a pointer to the Modbus channel to send the request to.
*
*               slave_node       Is the Modbus node number of the desired slave to obtain the information from.
*
*               slave_addr       Is the Modbus holding register address
*
*               reg_val          Is the desired 'integer' value of the holding register:
*
* Return(s)   : MODBUS_ERR_NONE          If the function was sucessful.
*               MODBUS_ERR_RX            If a timeout occurred before receiving a response from the slave.
*               MODBUS_ERR_SLAVE_ADDR    If the transmitted slave address doesn't correspond to the received slave address
*               MODBUS_ERR_FC            If the transmitted function code doesn't correspond to the received function code
*               MODBUS_ERR_REG_ADDR      If you specified an invalid register address
*
* Caller(s)   : Application.
*
* Note(s)     : none.
********************************************************************************
*/

#if (MODBUS_CFG_FC06_EN != 0)
uint16_t mbm_fc06_holding_reg_write(mb_channel_t *pch,
                                uint8_t slave_node,
                                uint16_t slave_addr,
                                uint16_t reg_val)
{
    osStatus_t ret_os = osOK;
    ret_os = osMutexAcquire(pch->mutex, osWaitForever);
    elab_assert(ret_os == osOK);


    MBM_TX_FRAME_NBYTES       = 4;
    MBM_TX_FRAME_SLAVE_ADDR   = slave_node;                                     /* Setup command                     */
    MBM_TX_FRAME_FC           = 6;
    MBM_TX_FRAME_FC06_ADDR_HI = (uint8_t)((slave_addr >> 8) & 0x00FF);
    MBM_TX_FRAME_FC06_ADDR_LO = (uint8_t) (slave_addr       & 0x00FF);
    MBM_TX_FRAME_FC06_DATA_HI = (uint8_t)((reg_val >> 8)    & 0x00FF);
    MBM_TX_FRAME_FC06_DATA_LO = (uint8_t) (reg_val          & 0x00FF);
    pch->size_expect = pch->mode == MODBUS_MODE_ASCII ? 17 : 8;

    /* Send command and wait for response from slave. */
    mbm_tx_cmd(pch);
    uint16_t err = mbm_rx_blocking(pch);

    if (err == MODBUS_ERR_NONE)
    {
        bool ok = mbm_rx_reply(pch);
        if (ok == true)
        {
            err = mbm_reg_write_resp(pch);                                          /* Parse the response from the slave */
        }
        else
        {
            err = MODBUS_ERR_RX;
        }
    }

    pch->rx_buff_byte_count = 0;
    pch->p_rx_buff = &pch->rx_buff[0];

    ret_os = osMutexRelease(pch->mutex);
    elab_assert(ret_os == osOK);

    return (err);
}

#endif


/*
********************************************************************************
*                                         mbm_fc06_holding_reg_write_fp()
*
* Description : Sends a MODBUS message to write the value of single holding register to a slave unit.
*
* Argument(s) : pch              Is a pointer to the Modbus channel to send the request to.
*
*               slave_node       Is the Modbus node number of the desired slave to obtain the information from.
*
*               slave_addr       Is the Modbus holding register address
*
*               reg_val_fp       Is the desired value of the floating-point holding register:
*
* Return(s)   : MODBUS_ERR_NONE          If the function was sucessful.
*               MODBUS_ERR_RX            If a timeout occurred before receiving a response from the slave.
*               MODBUS_ERR_SLAVE_ADDR    If the transmitted slave address doesn't correspond to the received slave address
*               MODBUS_ERR_FC            If the transmitted function code doesn't correspond to the received function code
*               MODBUS_ERR_REG_ADDR      If you specified an invalid register address
*
* Caller(s)   : Application.
*
* Note(s)     : none.
********************************************************************************
*/

#if (MODBUS_CFG_FC06_EN != 0) && \
    (MODBUS_CFG_FP_EN   != 0)
uint16_t mbm_fc06_holding_reg_write_fp(mb_channel_t *pch,
                                     uint8_t slave_node,
                                     uint16_t slave_addr,
                                     float     reg_val_fp)
{
    osStatus_t ret_os = osOK;
    ret_os = osMutexAcquire(pch->mutex, osWaitForever);
    elab_assert(ret_os == osOK);

    bool ok;
    uint8_t   i;
    uint8_t *p_fp;
    uint8_t *p_data;

    MBM_TX_FRAME_NBYTES       = 6;
    MBM_TX_FRAME_SLAVE_ADDR   = slave_node;             /* Setup command. */
    MBM_TX_FRAME_FC           = 6;
    MBM_TX_FRAME_FC06_ADDR_HI = (uint8_t)((slave_addr >> 8) & 0x00FF);
    MBM_TX_FRAME_FC06_ADDR_LO = (uint8_t) (slave_addr       & 0x00FF);
    pch->size_expect = pch->mode == MODBUS_MODE_ASCII ? 17 : 8;

    /* Point to the FP value. */
    p_fp   = (uint8_t *)&reg_val_fp;
    p_data = MBM_TX_FRAME_FC06_DATA_ADDR;

    /* Copy value to transmit buffer. */
    for (i = 0; i < sizeof(float); i++)
    {
        *p_data++ = *p_fp++;
        
    }

    /* Send command and wait for response from slave. */
    mbm_tx_cmd(pch);
    uint16_t err = mbm_rx_blocking(pch);

    if (err == MODBUS_ERR_NONE)
    {
        ok = mbm_rx_reply(pch);
        if (ok == true)
        {
            /* Parse the response from the slave */
            err = mbm_reg_write_resp(pch);
        }
        else {
            err = MODBUS_ERR_RX;
        }
    }

    pch->rx_buff_byte_count = 0;
    pch->p_rx_buff     = &pch->rx_buff[0];

    ret_os = osMutexRelease(pch->mutex);
    elab_assert(ret_os == osOK);

    return (err);
}
#endif

/*
********************************************************************************
*                                            mbm_fc15_coil_write()
*
* Description : Sends a MODBUS message to write to coils on a slave unit.
*
* Argument(s) : pch              Is a pointer to the Modbus channel to send the request to.
*
*               slave_node       Is the Modbus node number of the desired slave to write to.
*
*               slave_addr       Is the Modbus coil start address
*
*               p_coil_tbl       Is a pointer to an array of bytes containing the value of the coils to write.
*                                The format is:
*
*                                                MSB                               LSB
*                                                B7   B6   B5   B4   B3   B2   B1   B0
*                                                -------------------------------------
*                                p_coil_tbl[0]   #8   #7                            #1
*                                p_coil_tbl[1]   #16  #15                           #9
*                                     :
*                                     :
*
*                                Note that the array containing the coil values must be greater than or equal
*                                to:   nbr_coils / 8 + 1
*
*               nbr_coils        Is the desired number of coils to write
*
* Return(s)   : MODBUS_ERR_NONE          If the function was sucessful.
*               MODBUS_ERR_RX            If a timeout occurred before receiving a response from the slave.
*               MODBUS_ERR_SLAVE_ADDR    If the transmitted slave address doesn't correspond to the received slave address
*               MODBUS_ERR_FC            If the transmitted function code doesn't correspond to the received function code
*               MODBUS_ERR_BYTE_COUNT    If the expected number of bytes to receive doesn't correspond to the number of bytes received
*
* Caller(s)   : Application.
*
* Note(s)     : none.
********************************************************************************
*/

#if (MODBUS_CFG_FC15_EN != 0)
uint16_t mbm_fc15_coil_write(mb_channel_t *pch,
                             uint8_t slave_node,
                             uint16_t slave_addr,
                             uint8_t *p_coil_tbl,
                             uint16_t nbr_coils)
{
    elab_assert(pch != NULL);
    elab_assert(nbr_coils > 0 && nbr_coils <= 2000);

    osStatus_t ret_os = osOK;
    ret_os = osMutexAcquire(pch->mutex, osWaitForever);
    elab_assert(ret_os == osOK);

    MBM_TX_FRAME_SLAVE_ADDR         = slave_node;       /* Setup command. */
    MBM_TX_FRAME_FC                 = 15;
    MBM_TX_FRAME_FC15_ADDR_HI       = (uint8_t) ((slave_addr >> 8) & 0x00FF);
    MBM_TX_FRAME_FC15_ADDR_LO       = (uint8_t)  (slave_addr       & 0x00FF);
    MBM_TX_FRAME_FC15_NBR_POINTS_HI = (uint8_t) ((nbr_coils  >> 8) & 0x00FF);
    MBM_TX_FRAME_FC15_NBR_POINTS_LO = (uint8_t)  (nbr_coils        & 0x00FF);
    uint8_t nbr_bytes               = (uint8_t)(((nbr_coils - 1) / 8) + 1);
    MBM_TX_FRAME_FC15_BYTE_CNT      = nbr_bytes;
    uint8_t *p_data                 = MBM_TX_FRAME_FC15_DATA;
    MBM_TX_FRAME_NBYTES             = 5 + nbr_bytes;

    for (uint8_t i = 0; i < nbr_bytes; i++)
    {
        *p_data++ = *p_coil_tbl++;
    }
    pch->size_expect = pch->mode == MODBUS_MODE_ASCII ? 17 : 8;

    /* Send command and wait for response from slave. */
    mbm_tx_cmd(pch);
    uint16_t err = mbm_rx_blocking(pch);

    if (err == MODBUS_ERR_NONE)
    {
        bool ok = mbm_rx_reply(pch);
        /* Parse the response from the slave */
        err = ok ? mbm_coil_write_n_resp(pch) : MODBUS_ERR_RX;
    }

    pch->rx_buff_byte_count = 0;
    pch->p_rx_buff = &pch->rx_buff[0];

    ret_os = osMutexRelease(pch->mutex);
    elab_assert(ret_os == osOK);

    return (err);
}
#endif


/*
********************************************************************************
*                                       mbm_fc16_holding_reg_write()
*
* Description : Sends a MODBUS message to write to integer holding registers to a slave unit.
*
* Argument(s) : pch              Is a pointer to the Modbus channel to send the request to.
*
*               slave_node       Is the Modbus node number of the desired slave to write to.
*
*               slave_addr       Is the Modbus holding register start address
*
*               p_reg_tbl        Is a pointer to an array of integers containing the value of the holding
*                                registers to write.
*
*                                Note that the array containing the register values must be greater than or equal
*                                to 'nbr_regs'
*
*               nbr_regs         Is the desired number of registers to write
*
* Return(s)   : MODBUS_ERR_NONE          If the function was sucessful.
*               MODBUS_ERR_RX            If a timeout occurred before receiving a response from the slave.
*               MODBUS_ERR_SLAVE_ADDR    If the transmitted slave address doesn't correspond to the received slave address
*               MODBUS_ERR_FC            If the transmitted function code doesn't correspond to the received function code
*               MODBUS_ERR_REG_ADDR      If you specified an invalid register address
*               MODBUS_ERR_NBR_REG       If you specified an invalid number of registers
*
* Caller(s)   : Application.
*
* Note(s)     : none.
********************************************************************************
*/

#if (MODBUS_CFG_FC16_EN != 0)
uint16_t mbm_fc16_holding_reg_write(mb_channel_t *pch,
                                    uint8_t slave_node,
                                    uint16_t slave_addr,
                                    uint16_t *p_reg_tbl,
                                    uint16_t nbr_regs)
{
    elab_assert(pch != NULL);
    elab_assert(p_reg_tbl != NULL);
    elab_assert(nbr_regs > 0 && nbr_regs <= 125);
    elab_assert(slave_node > 0 && slave_node <= MODBUS_NODE_ADDR_MAX);
    elab_assert(slave_addr >= 0 && slave_addr < MODBUS_CFG_FP_START_IX);

    osStatus_t ret_os = osOK;
    ret_os = osMutexAcquire(pch->mutex, osWaitForever);
    elab_assert(ret_os == osOK);


    memset(pch->tx_frame_data, 0, MODBUS_CFG_BUF_SIZE);
    memset(pch->tx_buff, 0, MODBUS_CFG_BUF_SIZE);

    MBM_TX_FRAME_NBYTES             = nbr_regs * sizeof(uint16_t) + 5;
    MBM_TX_FRAME_SLAVE_ADDR         = slave_node;                               /* Setup command                     */
    MBM_TX_FRAME_FC                 = 16;
    MBM_TX_FRAME_FC16_ADDR_HI       = (uint8_t)((slave_addr >> 8) & 0x00FF);
    MBM_TX_FRAME_FC16_ADDR_LO       = (uint8_t) (slave_addr       & 0x00FF);
    MBM_TX_FRAME_FC16_NBR_REGS_HI   = (uint8_t)((nbr_regs   >> 8) & 0x00FF);
    MBM_TX_FRAME_FC16_NBR_REGS_LO   = (uint8_t) (nbr_regs         & 0x00FF);
    MBM_TX_FRAME_FC16_BYTE_CNT      = (uint8_t) (nbr_regs * 2);
    uint8_t *p_data                 = MBM_TX_FRAME_FC16_DATA;

    for (uint8_t i = 0; i < nbr_regs; i++)
    {
        /* Write HIGH data byte. */
        *p_data++ = (uint8_t)((*p_reg_tbl >> 8) & 0x00FF);
        /* Write LOW  data byte. */
        *p_data++ = (uint8_t) (*p_reg_tbl       & 0x00FF);
        p_reg_tbl++;
    }
    pch->size_expect = pch->mode == MODBUS_MODE_ASCII ? 17 : 8;

    /* Send command and wait for response from slave. */
    mbm_tx_cmd(pch);
    uint16_t err = mbm_rx_blocking(pch);

    if (err == MODBUS_ERR_NONE)
    {
        bool ok = mbm_rx_reply(pch);
        /* Parse the response from the slave */
        err = ok ? mbm_reg_write_n_resp(pch) : MODBUS_ERR_RX;
    }

    pch->rx_buff_byte_count = 0;
    pch->p_rx_buff = &pch->rx_buff[0];

    ret_os = osMutexRelease(pch->mutex);
    elab_assert(ret_os == osOK);

    return (err);
}
#endif


/*
********************************************************************************
*                                       mbm_fc16_holding_reg_write_fp()
*
* Description : Sends a MODBUS message to write to floating-point holding registers to a slave unit.
*
* Argument(s) : pch              Is a pointer to the Modbus channel to send the request to.
*
*               slave_node       Is the Modbus node number of the desired slave to write to.
*
*               slave_addr       Is the Modbus holding register start address
*
*               p_reg_tbl        Is a pointer to an array of floating-points containing the value of the holding
*                                registers to write.
*
*                                Note that the array containing the register values must be greater than or equal
*                                to 'nbr_regs'
*
*               nbr_regs         Is the desired number of registers to write
*
* Return(s)   : MODBUS_ERR_NONE          If the function was sucessful.
*               MODBUS_ERR_RX            If a timeout occurred before receiving a response from the slave.
*               MODBUS_ERR_SLAVE_ADDR    If the transmitted slave address doesn't correspond to the received slave address
*               MODBUS_ERR_FC            If the transmitted function code doesn't correspond to the received function code
*               MODBUS_ERR_REG_ADDR      If you specified an invalid register address
*               MODBUS_ERR_NBR_REG       If you specified an invalid number of registers
*
* Caller(s)   : Application.
*
* Note(s)     : none.
********************************************************************************
*/

#if (MODBUS_CFG_FC16_EN != 0) && (MODBUS_CFG_FP_EN   != 0)
uint16_t mbm_fc16_holding_reg_write_fp(mb_channel_t *pch,
                                        uint8_t slave_node,
                                        uint16_t slave_addr,
                                        float *p_reg_tbl,
                                        uint16_t nbr_regs)
{
    elab_assert(pch != NULL);
    elab_assert(p_reg_tbl != NULL);
    elab_assert(nbr_regs > 0 && nbr_regs <= 62);
    elab_assert(slave_node > 0 && slave_node <= MODBUS_NODE_ADDR_MAX);
    elab_assert(slave_addr >= MODBUS_CFG_FP_START_IX && slave_addr <= 65535);

    osStatus_t ret_os = osOK;
    ret_os = osMutexAcquire(pch->mutex, osWaitForever);
    elab_assert(ret_os == osOK);

    bool ok;
    uint8_t   nbr_bytes;
    uint8_t   i;
    uint16_t n;
    uint8_t *p_data;
    uint8_t *p_fp;

    MBM_TX_FRAME_NBYTES           = nbr_regs * sizeof(float) + 5;
    MBM_TX_FRAME_SLAVE_ADDR       = slave_node;                                 /* Setup command                     */
    MBM_TX_FRAME_FC               = 16;
    MBM_TX_FRAME_FC16_ADDR_HI     = (uint8_t)((slave_addr >> 8) & 0x00FF);
    MBM_TX_FRAME_FC16_ADDR_LO     = (uint8_t) (slave_addr       & 0x00FF);
    MBM_TX_FRAME_FC16_NBR_REGS_HI = (uint8_t)((nbr_regs   >> 8) & 0x00FF);
    MBM_TX_FRAME_FC16_NBR_REGS_LO = (uint8_t) (nbr_regs         & 0x00FF);
    nbr_bytes                     = (uint8_t) (nbr_regs * sizeof(float));
    MBM_TX_FRAME_FC16_BYTE_CNT    = nbr_bytes;
    p_data                        = MBM_TX_FRAME_FC16_DATA;
    pch->size_expect = pch->mode == MODBUS_MODE_ASCII ? 17 : 8;

    for (n = 0; n < nbr_regs; n++) {                                            /* Copy all floating point values    */
        p_fp = (uint8_t *)p_reg_tbl;
        for (i = 0; i < sizeof(float); i++)
        {
            *p_data++ = *p_fp++;
        }
        p_reg_tbl++;
    }

    /* Send command and wait for response from slave. */
    mbm_tx_cmd(pch);
    uint16_t err = mbm_rx_blocking(pch);

    if (err == MODBUS_ERR_NONE)
    {
        bool ok = mbm_rx_reply(pch);
        if (ok == true)
        {
            err = mbm_reg_write_n_resp(pch);                                         /* Parse the response from the slave */
        }
        else
        {
            err = MODBUS_ERR_RX;
        }
    }

    pch->rx_buff_byte_count = 0;
    pch->p_rx_buff = &pch->rx_buff[0];

    ret_os = osMutexRelease(pch->mutex);
    elab_assert(ret_os == osOK);

    return (err);
}
#endif


/*
********************************************************************************
*                                       mbm_coil_di_read_resp()
*
* Description : Checks the slave's response to a request to read the status of coils or discrete inputs.
*
* Argument(s) : pch       A pointer to the channel that the message was received on.
*
*               ptbl      Pointer to where data will be placed
*
* Return(s)   : MODBUS_ERR_NONE          If the function was sucessful.
*               MODBUS_ERR_SLAVE_ADDR    If the transmitted slave address doesn't correspond to the received slave address
*               MODBUS_ERR_FC            If the transmitted function code doesn't correspond to the received function code
*               MODBUS_ERR_BYTE_COUNT    If the expected number of bytes to receive doesn't correspond to the number of bytes received
*
* Caller(s)   : mbm_fc01_coil_read(),
*               mbm_fc02_di_read().
*
* Note(s)     : none.
********************************************************************************
*/

#if(MODBUS_CFG_FC01_EN != 0) || \
   (MODBUS_CFG_FC02_EN != 0)
static uint16_t mbm_coil_di_read_resp(mb_channel_t *pch, uint8_t *ptbl)
{
    uint8_t *psrc;

    /* Validate slave address */
    if (MBM_TX_FRAME_SLAVE_ADDR != pch->rx_frame_data[0])
    {
        return (MODBUS_ERR_SLAVE_ADDR);
    }

    /* Validate function code */
    if (MBM_TX_FRAME_FC != pch->rx_frame_data[1])
    {
        return (MODBUS_ERR_FC);
    }

    /* Validate number of bytes received           */
    uint16_t nbr_points = (MBM_TX_FRAME_FC01_NBR_POINTS_HI << 8) +
                            MBM_TX_FRAME_FC01_NBR_POINTS_LO;
    uint8_t byte_cnt   = (uint8_t)((nbr_points - 1) / 8) + 1;
    if (byte_cnt != pch->rx_frame_data[2])
    {
        return (MODBUS_ERR_BYTE_COUNT);
    }

    /* Copy received data to destination array     */
    memcpy(ptbl, &pch->rx_frame_data[3], byte_cnt);

    return (MODBUS_ERR_NONE);
}
#endif

/*
********************************************************************************
*                                          mbm_reg_read_resp()
*
* Description : Checks the slave's response to a request to read the status of input or output registers.
*
* Argument(s) : pch             A pointer to the channel that the message was received on.
*
*               ptbl            A pointer to where data will be placed
*
* Return(s)   : MODBUS_ERR_NONE          If the function was sucessful.
*               MODBUS_ERR_SLAVE_ADDR    If the transmitted slave address doesn't correspond to the received slave address
*               MODBUS_ERR_FC            If the transmitted function code doesn't correspond to the received function code
*               MODBUS_ERR_BYTE_COUNT    If the expected number of bytes to receive doesn't correspond to the number of bytes received
*
* Caller(s)   : none.
*
* Note(s)     : none.
********************************************************************************
*/

#if (MODBUS_CFG_FC03_EN != 0) || \
    (MODBUS_CFG_FC04_EN != 0)
static uint16_t mbm_reg_read_resp(mb_channel_t *pch, uint16_t *ptbl)
{
    /* Validate slave address. */
    if (MBM_TX_FRAME_SLAVE_ADDR != pch->rx_frame_data[0])
    {
        return (MODBUS_ERR_SLAVE_ADDR);
    }

    /* Validate function code. */
    if (MBM_TX_FRAME_FC != pch->rx_frame_data[1])
    {
        return (MODBUS_ERR_FC);
    }

    /* Validate number of bytes received. */
    uint8_t nbr_points = (MBM_TX_FRAME_FC01_NBR_POINTS_HI << 8) +
                            MBM_TX_FRAME_FC01_NBR_POINTS_LO;
    uint8_t byte_cnt = nbr_points * sizeof(uint16_t);
    if (byte_cnt != pch->rx_frame_data[2])
    {
        return (MODBUS_ERR_BYTE_COUNT);
    }

    /* Copy received data to destination array. */
    uint8_t *psrc = &pch->rx_frame_data[3];
    for (uint32_t i = 0; i < byte_cnt; i += sizeof(uint16_t))
    {
        uint8_t data_hi = *psrc++;
        uint8_t data_lo = *psrc++;
        *ptbl++ = ((uint16_t)data_hi << 8) + (uint16_t)data_lo;
    }

    return (MODBUS_ERR_NONE);
}
#endif

/*
********************************************************************************
*                                          mbm_reg_read_fp_resp()
*
* Description : Checks the slave's response to a request to read the status of input or output registers.
*
* Argument(s) : pch             A pointer to the channel that the message was received on.
*
*               ptbl            A pointer to where data will be placed
*
* Return(s)   : MODBUS_ERR_NONE          If the function was sucessful.
*               MODBUS_ERR_SLAVE_ADDR    If the transmitted slave address doesn't correspond to the received slave address
*               MODBUS_ERR_FC            If the transmitted function code doesn't correspond to the received function code
*               MODBUS_ERR_BYTE_COUNT    If the expected number of bytes to receive doesn't correspond to the number of bytes received
*
* Caller(s)   : mbm_fc03_holding_reg_read_fp().
*
* Note(s)     : none.
********************************************************************************
*/

#if (MODBUS_CFG_FC03_EN != 0) && \
    (MODBUS_CFG_FP_EN   != 0)
static uint16_t mbm_reg_read_fp_resp(mb_channel_t *pch, float *ptbl)
{
    uint8_t       slave_addr;
    uint8_t       fnct_code;
    uint8_t       byte_cnt;
    uint8_t       nbr_points;
    uint8_t       i;
    uint8_t       j;
    uint8_t      *psrc;
    uint8_t      *pdest;



    slave_addr = MBM_TX_FRAME_SLAVE_ADDR;                     /* Validate slave address                      */
    if (slave_addr != pch->rx_frame_data[0])
    {
        return (MODBUS_ERR_SLAVE_ADDR);
    }

    fnct_code   = MBM_TX_FRAME_FC;                            /* Validate function code                      */
    if (fnct_code != pch->rx_frame_data[1])
    {
        return (MODBUS_ERR_FC);
    }
                                                              /* Validate number of bytes received           */
    nbr_points = (MBM_TX_FRAME_FC01_NBR_POINTS_HI << 8)
               +  MBM_TX_FRAME_FC01_NBR_POINTS_LO;
    byte_cnt   = nbr_points * sizeof(float);
    if (byte_cnt != pch->rx_frame_data[2])
    {
        return (MODBUS_ERR_BYTE_COUNT);
    }

    psrc  = &pch->rx_frame_data[3];                             /* Copy received data to destination array     */
    pdest = (uint8_t *)ptbl;
    for (i = 0; i < byte_cnt; i += sizeof(float)) {
        for (j = 0; j < sizeof(float); j++)
        {
            *pdest++ = *psrc++;
        }
    }

    return (MODBUS_ERR_NONE);
}
#endif

/*
********************************************************************************
*                                          mbm_coil_write_resp()
*
* Description : Checks the slave's response to a request to write coils.
*
* Argument(s) : pch             A pointer to the channel that the message was received on.
*
* Return(s)   : MODBUS_ERR_NONE          If the function was sucessful.
*               MODBUS_ERR_SLAVE_ADDR    If the transmitted slave address doesn't correspond to the received slave address
*               MODBUS_ERR_FC            If the transmitted function code doesn't correspond to the received function code
*               MODBUS_ERR_BYTE_COUNT    If the expected number of bytes to receive doesn't correspond to the number of bytes received
*               MODBUS_ERR_COIL_ADDR     If you specified an invalid coil address
*
* Caller(s)   : mbm_fc05_wirte_coil().
*
* Note(s)     : none.
********************************************************************************
*/

#if (MODBUS_CFG_FC05_EN != 0)
static uint16_t mbm_coil_write_resp(mb_channel_t *pch)
{
    /* Validate slave address. */
    if (MBM_TX_FRAME_SLAVE_ADDR != pch->rx_frame_data[0])
    {
        return (MODBUS_ERR_SLAVE_ADDR);
    }

    /* Validate function code. */
    if (MBM_TX_FRAME_FC != pch->rx_frame_data[1])
    {
        return (MODBUS_ERR_FC);
    }

    /* Validate function code. */
    if ((MBM_TX_FRAME_FC05_ADDR_HI != pch->rx_frame_data[2]) ||
        (MBM_TX_FRAME_FC05_ADDR_LO != pch->rx_frame_data[3]))
    {
        return (MODBUS_ERR_COIL_ADDR);
    }

    return (MODBUS_ERR_NONE);
}
#endif

/*
********************************************************************************
*                                          mbm_coil_write_n_resp()
*
* Description : Checks the slave's response to a request to write coils.
*
* Argument(s) : pch             A pointer to the channel that the message was received on.
*
* Return(s)   : MODBUS_ERR_NONE          If the function was sucessful.
*               MODBUS_ERR_SLAVE_ADDR    If the transmitted slave address doesn't correspond to the received slave address
*               MODBUS_ERR_FC            If the transmitted function code doesn't correspond to the received function code
*               MODBUS_ERR_BYTE_COUNT    If the expected number of bytes to receive doesn't correspond to the number of bytes received
*
* Caller(s)   : mbm_fc15_coil_write().
*
* Note(s)     : none.
********************************************************************************
*/

#if (MODBUS_CFG_FC15_EN != 0)
static uint16_t mbm_coil_write_n_resp(mb_channel_t *pch)
{
    /* Validate slave address. */
    if (MBM_TX_FRAME_SLAVE_ADDR != pch->rx_frame_data[0])
    {
        return (MODBUS_ERR_SLAVE_ADDR);
    }

    /* Validate function code. */
    if (MBM_TX_FRAME_FC != pch->rx_frame_data[1])
    {
        return (MODBUS_ERR_FC);
    }

    /* Validate coil address. */
    if ((MBM_TX_FRAME_FC05_ADDR_HI != pch->rx_frame_data[2]) ||
        (MBM_TX_FRAME_FC05_ADDR_LO != pch->rx_frame_data[3]))
    {
        return (MODBUS_ERR_COIL_ADDR);
    }

    /* Validate number of coils. */
    if ((MBM_TX_FRAME_FC15_NBR_POINTS_HI != pch->rx_frame_data[4]) ||
        (MBM_TX_FRAME_FC15_NBR_POINTS_LO != pch->rx_frame_data[5]))
    {
        return (MODBUS_ERR_COIL_QTY);
    }

    return (MODBUS_ERR_NONE);
}
#endif

/*
********************************************************************************
*                                            mbm_reg_write_resp()
*
* Description : Checks the slave's response to a request to write a register.
*
* Argument(s) : pch             A pointer to the channel that the message was received on.
*
* Return(s)   : MODBUS_ERR_NONE          If the function was sucessful.
*               MODBUS_ERR_SLAVE_ADDR    If the transmitted slave address doesn't correspond to the received slave address
*               MODBUS_ERR_FC            If the transmitted function code doesn't correspond to the received function code
*               MODBUS_ERR_BYTE_COUNT    If the expected number of bytes to receive doesn't correspond to the number of bytes received
*
* Caller(s)   : mbm_fc06_holding_reg_write(),
*               mbm_fc06_holding_reg_write_fp().
*
* Note(s)     : none.
********************************************************************************
*/

#if (MODBUS_CFG_FC06_EN != 0)
static uint16_t mbm_reg_write_resp(mb_channel_t *pch)
{
    /* Validate slave address. */
    if (MBM_TX_FRAME_SLAVE_ADDR != pch->rx_frame_data[0])
    {
        return (MODBUS_ERR_SLAVE_ADDR);
    }

    /* Validate function code. */
    if (MBM_TX_FRAME_FC != pch->rx_frame_data[1])
    {
        return (MODBUS_ERR_FC);
    }

    /* Validate register address. */
    if ((MBM_TX_FRAME_FC06_ADDR_HI != pch->rx_frame_data[2]) ||
        (MBM_TX_FRAME_FC06_ADDR_LO != pch->rx_frame_data[3]))
    {
        return (MODBUS_ERR_REG_ADDR);
    }

    return (MODBUS_ERR_NONE);
}
#endif

/*
********************************************************************************
*                                             mbm_reg_write_n_resp()
*
* Description : Checks the slave's response to a request to write a register.
*
* Argument(s) : pch             A pointer to the channel that the message was received on.
*
* Return(s)   : MODBUS_ERR_NONE          If the function was sucessful.
*               MODBUS_ERR_SLAVE_ADDR    If the transmitted slave address doesn't correspond to the received slave address
*               MODBUS_ERR_FC            If the transmitted function code doesn't correspond to the received function code
*               MODBUS_ERR_BYTE_COUNT    If the expected number of bytes to receive doesn't correspond to the number of bytes received
*
* Caller(s)   : mbm_fc16_holding_reg_write(),
*               mbm_fc16_holding_reg_write_fp().
*
* Note(s)     : none.
********************************************************************************
*/

#if (MODBUS_CFG_FC16_EN != 0)
static uint16_t mbm_reg_write_n_resp(mb_channel_t *pch)
{
    /* Validate slave address. */
    if (MBM_TX_FRAME_SLAVE_ADDR != pch->rx_frame_data[0])
    {
        return (MODBUS_ERR_SLAVE_ADDR);
    }

    /* Validate function code. */
    if (MBM_TX_FRAME_FC != pch->rx_frame_data[1])
    {
        return (MODBUS_ERR_FC);
    }

    /* Validate register address. */
    if ((MBM_TX_FRAME_FC16_ADDR_HI != pch->rx_frame_data[2]) ||
        (MBM_TX_FRAME_FC16_ADDR_LO != pch->rx_frame_data[3]))
    {
        return (MODBUS_ERR_REG_ADDR);
    }

    /* Validate number of registers. */
    if ((MBM_TX_FRAME_FC16_NBR_REGS_HI != pch->rx_frame_data[4]) ||
        (MBM_TX_FRAME_FC16_NBR_REGS_LO != pch->rx_frame_data[5]))
    {
        return (MODBUS_ERR_NBR_REG);
    }

    return (MODBUS_ERR_NONE);
}
#endif

/*
********************************************************************************
*                                             mbm_rx_reply()
*
* Description : Receive a reply from a Slave
*
* Argument(s) : ch       Specifies the Modbus channel on which the reply is coming from
*
* Return(s)   : none.
*
* Caller(s)   : MBM_Fxx  Modbus Master Functions.
*
* Note(s)     : none.
********************************************************************************
*/

static bool mbm_rx_reply(mb_channel_t *pch)
{
    bool ok = false;

#if (MODBUS_CFG_ASCII_EN != 0)
    if (pch->mode == MODBUS_MODE_ASCII)
    {
        ok = mb_ascii_rx(pch);
    }
#endif

#if (MODBUS_CFG_RTU_EN != 0)
    if (pch->mode == MODBUS_MODE_RTU)
    {
        ok = mb_rtu_rx(pch);
    }
#endif

    return (ok);
}


/*
********************************************************************************
*                                              mbm_tx_cmd()
*
* Description : Send a command to a Slave
*
* Argument(s) : ch      Specifies the Modbus channel on which the command will
                        be sent
*
* Return(s)   : none.
*
* Caller(s)   : MBM_Fxx Modbus Master Functions.
*
* Note(s)     : none.
********************************************************************************
*/

static void mbm_tx_cmd(mb_channel_t *pch)
{
#if (MODBUS_CFG_ASCII_EN != 0)
    if (pch->mode == MODBUS_MODE_ASCII)
    {
        mb_ascii_tx(pch);
    }
#endif

#if (MODBUS_CFG_RTU_EN != 0)
    if (pch->mode == MODBUS_MODE_RTU)
    {
        mb_rtu_tx(pch);
    }
#endif
}

#endif

/* ----------------------------- end of file -------------------------------- */
