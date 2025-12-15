/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/*
********************************************************************************
*                                INCLUDE FILES
********************************************************************************
*/

#include "../../common/elab_assert.h"
#include "modbus.h"

ELAB_TAG("ModbusSlave");

#if (MODBUS_CFG_ASCII_EN != 0)
#define MODBUS_ASCII_MIN_MSG_SIZE                 11
#endif

#if     (MODBUS_CFG_RTU_EN != 0)
#define MODBUS_RTU_MIN_MSG_SIZE                    4
#endif

#define  MODBUS_COIL_OFF_CODE                  0x0000
#define  MODBUS_COIL_ON_CODE                   0xFF00

#if (MODBUS_CFG_SLAVE_EN != 0)

#define  MBS_RX_DATA_START     (((uint16_t)pch->rx_frame_data[2] << 8) + (uint16_t)pch->rx_frame_data[3])
#define  MBS_RX_DATA_START_H    (pch->rx_frame_data[2])
#define  MBS_RX_DATA_START_L    (pch->rx_frame_data[3])

#define  MBS_RX_DATA_POINTS    (((uint16_t)pch->rx_frame_data[4] << 8) + (uint16_t)pch->rx_frame_data[5])
#define  MBS_RX_DATA_POINTS_H   (pch->rx_frame_data[4])
#define  MBS_RX_DATA_POINTS_L   (pch->rx_frame_data[5])

#define  MBS_RX_DATA_BYTES      (pch->rx_frame_data[6])

#define  MBS_RX_DATA_COIL      (((uint16_t)pch->rx_frame_data[4] << 8) + (uint16_t)pch->rx_frame_data[5])
#define  MBS_RX_DATA_COIL_H     (pch->rx_frame_data[4])
#define  MBS_RX_DATA_COIL_L     (pch->rx_frame_data[5])

#define  MBS_RX_DATA_REG       (((uint16_t)pch->rx_frame_data[4] << 8) + (uint16_t)pch->rx_frame_data[5])
#define  MBS_RX_DATA_REG_H      (pch->rx_frame_data[4])
#define  MBS_RX_DATA_REG_L      (pch->rx_frame_data[5])

#define  MBS_RX_DIAG_CODE      (((uint16_t)pch->rx_frame_data[2] << 8) + (uint16_t)pch->rx_frame_data[3])
#define  MBS_RX_DIAG_CODE_H     (pch->rx_frame_data[2])
#define  MBS_RX_DIAG_CODE_L     (pch->rx_frame_data[3])
#define  MBS_RX_DIAG_DATA      (((uint16_t)pch->rx_frame_data[4] << 8) + (uint16_t)pch->rx_frame_data[5])
#define  MBS_RX_DIAG_DATA_H     (pch->rx_frame_data[4])
#define  MBS_RX_DIAG_DATA_L     (pch->rx_frame_data[5])

#define  MBS_RX_FRAME          (&pch->RxFrame)
#define  MBS_RX_FRAME_ADDR      (pch->rx_frame_data[0])
#define  MBS_RX_FRAME_FC        (pch->rx_frame_data[1])
#define  MBS_RX_FRAME_DATA      (pch->rx_frame_data[2])
#define  MBS_RX_FRAME_NBYTES    (pch->rx_frame_ndata_bytes)


#define  MBS_TX_DATA_START_H    (pch->tx_frame_data[2])
#define  MBS_TX_DATA_START_L    (pch->tx_frame_data[3])

#define  MBS_TX_DATA_POINTS_H   (pch->tx_frame_data[4])
#define  MBS_TX_DATA_POINTS_L   (pch->tx_frame_data[5])

#define  MBS_TX_DATA_COIL_H     (pch->tx_frame_data[4])
#define  MBS_TX_DATA_COIL_L     (pch->tx_frame_data[5])

#define  MBS_TX_DATA_REG_H      (pch->tx_frame_data[4])
#define  MBS_TX_DATA_REG_L      (pch->tx_frame_data[5])

#define  MBS_TX_DIAG_CODE_H     (pch->tx_frame_data[2])
#define  MBS_TX_DIAG_CODE_L     (pch->tx_frame_data[3])
#define  MBS_TX_DIAG_DATA_H     (pch->tx_frame_data[4])
#define  MBS_TX_DIAG_DATA_L     (pch->tx_frame_data[5])


#define  MBS_TX_FRAME           (&pch->TxFrame)
#define  MBS_TX_FRAME_ADDR      (pch->tx_frame_data[0])
#define  MBS_TX_FRAME_FC        (pch->tx_frame_data[1])
#define  MBS_TX_FRAME_DATA      (pch->tx_frame_data[2])
#define  MBS_TX_FRAME_NBYTES    (pch->tx_frame_ndata_bytes)


/*
********************************************************************************
*                         LOCAL FUNCTION PROTOTYPES
********************************************************************************
*/

#if (MODBUS_CFG_ASCII_EN != 0)
uint8_t mb_ascii_rx_calc_lrc(mb_channel_t *pch);
void mb_ascii_rx_byte(mb_channel_t *pch, uint8_t rx_byte);
bool mb_ascii_rx(mb_channel_t *pch);
void mb_ascii_tx(mb_channel_t *pch);
#endif

#if (MODBUS_CFG_RTU_EN != 0)
bool mb_rtu_rx(mb_channel_t *pch);
void mb_rtu_tx(mb_channel_t *pch);
uint16_t mb_rtu_calc_crc(mb_channel_t *pch);
uint16_t mb_rtu_tx_calc_crc(mb_channel_t *pch);
uint16_t mb_rtu_rx_calc_crc(mb_channel_t *pch);
void mb_rtu_rx_byte(mb_channel_t *pch, uint8_t rx_byte);
#endif

#if (MODBUS_CFG_SLAVE_EN != 0)

static void mbs_error_resp_set(mb_channel_t *pch, uint8_t errcode);

#if (MODBUS_CFG_FC01_EN != 0)
static bool mbs_fc01_coil_read(mb_channel_t   *pch);
#endif

#if (MODBUS_CFG_FC02_EN != 0)
static bool mbs_fc02_di_read(mb_channel_t   *pch);
#endif

#if (MODBUS_CFG_FC03_EN != 0)
static bool mbs_fc03_holding_reg_read(mb_channel_t   *pch);
#endif

#if (MODBUS_CFG_FC04_EN != 0)
static bool mbs_fc04_input_reg_read(mb_channel_t   *pch);
#endif

#if (MODBUS_CFG_FC05_EN != 0)
static bool mbs_fc05_coil_write(mb_channel_t   *pch);
#endif

#if (MODBUS_CFG_FC06_EN != 0)
static bool mbs_fc06_holding_reg_wr(mb_channel_t   *pch);
#endif

#if (MODBUS_CFG_FC15_EN != 0)
static bool mbs_fc15_coil_write_multi(mb_channel_t   *pch);
#endif

#if (MODBUS_CFG_FC16_EN != 0)
static bool mbs_fc16_holding_reg_write_multi(mb_channel_t   *pch);
#endif

#if (MODBUS_CFG_ASCII_EN != 0)
static void mbs_ascii_task(mb_channel_t   *pch);
#endif

#if (MODBUS_CFG_RTU_EN != 0)
static void mbs_rtu_task(mb_channel_t   *pch);
#endif

#endif

#if (MODBUS_CFG_SLAVE_EN != 0)
void mbs_rx_task(mb_channel_t *pch);
#endif

void mb_rx_byte(mb_channel_t *pch, uint8_t rx_byte);
void mb_rx_task(mb_channel_t*pch);
void mb_tx(mb_channel_t *pch);
void mb_tx_byte(mb_channel_t   *pch);

void entry_slave_rx(void *paras)
{
    mb_channel_t *pch = (mb_channel_t *)paras;
    uint8_t c = 0;
    int32_t ret = 0;
    int32_t ret_bkp = ELAB_ERR_TIMEOUT;
    uint32_t timeout = osWaitForever;

    if (pch->mode == MODBUS_MODE_RTU)
    {
        timeout = pch->rtu_timeout;
    }

    while (1)
    {
        ret = elab_serial_read(pch->serial, &c, 1, timeout);
        if (ret > 0)
        {
            mb_rx_byte(pch, c);
        }
        else if (pch->mode == MODBUS_MODE_RTU &&
                    ret == ELAB_ERR_TIMEOUT && ret_bkp > 0)
        {
            mbs_rx_task(pch);
        }

        ret_bkp = ret;
    }
}

 
/*
********************************************************************************
*                            mbs_error_resp_set()
*
* Description : This function sets the indicated error response code into the 
*               response frame. Then theroutine is called to calculate the error
*               check value.
*
* Argument(s) : pch         Is a pointer to the Modbus channel's data structure.
*
*               errcode     An unsigned byte value containing the error code that is to be placed in the
*                           response frame.
*
* Return(s)   : none.
*
* Caller(s)   : mbs_handler(),
*               Modbus Slave functions.
*
* Note(s)     : none.
********************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN != 0)
static void mbs_error_resp_set(mb_channel_t *pch, uint8_t err_code)
{
    MBS_TX_FRAME_ADDR   = MBS_RX_FRAME_ADDR;
    MBS_TX_FRAME_FC     = MBS_RX_FRAME_FC | 0x80;                /* Set the high order bit of the function code.             */
    MBS_TX_FRAME_DATA   = err_code;                              /* Set the high order bit of the function code.             */
    MBS_TX_FRAME_NBYTES = 1;                                     /* Nbr of data bytes in exception response is 1.            */
}
#endif

/*
********************************************************************************
*                              mbs_handler()
*
* Description : This is the main processing function for MODBUS commands. The 
*               message integrity is verified, and if valid, the function 
*               requested is processed.  Unimplemented functions will generate 
*               an Illegal Function Exception Response code (01).
*
* Argument(s) : pch      Is a pointer to the Modbus channel's data structure.
*
* Return(s)   : none.
*
* Caller(s)   : mbs_ascii_task(),
*               mbs_rtu_task().
*
* Note(s)     : none.
********************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN != 0)
bool mbs_handler(mb_channel_t *pch)
{
    bool send_reply = false;

    /* Proper node address? or a 'broadcast' address? */
    if ((MBS_RX_FRAME_ADDR == pch->node_addr) || (MBS_RX_FRAME_ADDR == 0))
    {
        switch (MBS_RX_FRAME_FC)                            /* Handle the function requested in the frame.              */
        {
#if (MODBUS_CFG_FC01_EN != 0)
            case MODBUS_FC01_COIL_RD:
                send_reply = mbs_fc01_coil_read(pch);
                break;
#endif

#if (MODBUS_CFG_FC02_EN != 0)
            case MODBUS_FC02_DI_RD:
                send_reply = mbs_fc02_di_read(pch);
                break;
#endif

#if (MODBUS_CFG_FC03_EN != 0)
            case MODBUS_FC03_HOLDING_REG_RD:                 /* Process read output registers command.                   */
                send_reply = mbs_fc03_holding_reg_read(pch);
                break;
#endif

#if (MODBUS_CFG_FC04_EN != 0)
            case MODBUS_FC04_IN_REG_RD:
                send_reply = mbs_fc04_input_reg_read(pch);
                break;
#endif

#if (MODBUS_CFG_FC05_EN != 0)
            case MODBUS_FC05_COIL_WR:
                send_reply = pch->write_en ? mbs_fc05_coil_write(pch) : false;
                break;
#endif

#if (MODBUS_CFG_FC06_EN != 0)
            case MODBUS_FC06_HOLDING_REG_WR:
                send_reply = pch->write_en ? mbs_fc06_holding_reg_wr(pch) : false;
                break;
#endif

#if (MODBUS_CFG_FC15_EN != 0)
            case MODBUS_FC15_COIL_WR_MULTIPLE:
                send_reply = pch->write_en ? mbs_fc15_coil_write_multi(pch) : false;
                break;
#endif

#if (MODBUS_CFG_FC16_EN != 0)
            case MODBUS_FC16_HOLDING_REG_WR_MULTIPLE:
                send_reply = pch->write_en ?
                                mbs_fc16_holding_reg_write_multi(pch) : false;
                break;
#endif

            default:
                /* Function code not implemented, set error response. */
                pch->error = MODBUS_ERR_ILLEGAL_FC;
                mbs_error_resp_set(pch, MODBUS_ERR_ILLEGAL_FC);
                send_reply = true;
                break;
        }
    }

    /* If the command received a 'broadcast', */
    if (MBS_RX_FRAME_ADDR == 0)
    {
        /* Yes, don't reply. */
        return (false);
    }
    else
    {
        /* No,  reply according to the outcome of the command. */
        return (send_reply);
    }
}
#endif

/*
********************************************************************************
*                              mbs_fc01_coil_read()
*
* Description : Responds to a request to read the status of any number of coils.
*
* Argument(s) : pch       Is a pointer to the Modbus channel's data structure.
*
* Return(s)   : true      If a response needs to be sent
*               false     If not
*
* Caller(s)   : mbs_handler().
*
* Note(s)     : 1) RX command format:             Example:
*                  <slave address>                0x11
*                  <function code>                0x01
*                  <start address HI>             0x00
*                  <start address LO>             0x13
*                  <# coils HI>                   0x00
*                  <# coils LO>                   0x25
*                  <Error Check (LRC or CRC)>     0x??
*
*               2) TX reply format:               Example:
*                  <slave address>                0x11
*                  <function code>                0x01
*                  <byte count>                   0x05
*                  <Data Coils>                   0xCD  (Bit set to 1 means ON, Bit cleared means == OFF)
*                  <Data Coils>                   0x6B  (Bit set to 1 means ON, Bit cleared means == OFF)
*                  <Data Coils>                   0xB2  (Bit set to 1 means ON, Bit cleared means == OFF)
*                  <Data Coils>                   0x0E  (Bit set to 1 means ON, Bit cleared means == OFF)
*                  <Data Coils>                   0x1B  (Bit set to 1 means ON, Bit cleared means == OFF)
*                  <Error Check (LRC or CRC)>     0x??
********************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN != 0)
#if (MODBUS_CFG_FC01_EN  != 0)
static bool mbs_fc01_coil_read(mb_channel_t *pch)
{
    uint8_t   *presp;
    uint16_t    coil;
    bool   coil_val;
    uint16_t    err;
    uint16_t    nbr_coils;
    uint16_t    nbr_bytes;
    uint8_t    bit_mask;
    uint16_t    ix;


    if (pch->rx_frame_ndata_bytes != 4) {               /* 4 data bytes for this message.                           */
        return (false);                                 /* Tell caller that we DON'T need to send a response        */
    }
    coil      = MBS_RX_DATA_START;                      /* Get the starting address of the desired coils            */
    nbr_coils = MBS_RX_DATA_POINTS;                     /* Find out how many coils                                  */
    if (nbr_coils == 0 || nbr_coils > 2000) {           /* Make sure we don't exceed the allowed limit per request  */
        pch->error = MODBUS_ERR_FC01_01;
        mbs_error_resp_set(pch,
                       MODBUS_ERR_ILLEGAL_DATA_QTY);
        return (true);                                  /* Tell caller that we need to send a response              */
    }
    nbr_bytes              = ((nbr_coils - 1) / 8) + 1; /* Find #bytes needed for response.                         */
    pch->tx_frame_ndata_bytes = nbr_bytes + 1;          /* Number of data bytes + byte count.                       */
    presp                  = &pch->tx_frame_data[0];    /* Clear bytes in response                                  */
    for (ix = 0; ix < (nbr_bytes + 3); ix++) {
        *presp++ = 0x00;
    }
    bit_mask = 0x01;                                    /* Start with bit 0 in response byte data mask.             */
    ix       =    0;                                    /* Initialize loop counter.                                 */
    presp    = &pch->tx_frame_data[0];                  /* Reset the pointer to the start of the response           */
    *presp++ = MBS_RX_FRAME_ADDR;                       /* Prepare response packet                                  */
    *presp++ = MBS_RX_FRAME_FC;
    *presp++ = (uint8_t)nbr_bytes;                      /* Set number of data bytes in response message.            */
    while (ix < nbr_coils) {                            /* Loop through each COIL requested.                        */
        /* Get the current value of the coil. */
        elab_assert(pch->cb.coil_read != NULL);
        coil_val = pch->cb.coil_read(coil, &err);
        switch (err) {
            case MODBUS_ERR_NONE:
                if (coil_val == true)                   /* Only set data response bit if COIL is on.                */
                {
                    *presp |= bit_mask;
                }
                coil ++;
                ix ++;                                  /* Increment COIL counter.                                  */
                if ((ix % 8) == 0)                      /* Determine if 8 data bits have been filled.               */
                {
                    bit_mask   = 0x01;                  /* Reset the data mask.                                     */
                    presp ++;                           /* Increment data frame index.                              */
                }
                else
                {                                       /* Still in same data byte, so                              */
                    bit_mask <<= 1;                     /* Shift the data mask to the next higher bit position.     */
                }
                break;

            case MODBUS_ERR_RANGE:
            default:
                pch->error = MODBUS_ERR_FC01_02;
                mbs_error_resp_set(pch, MODBUS_ERR_ILLEGAL_DATA_ADDR);
                return (true);                          /* Tell caller that we need to send a response. */
        }
    }
    pch->error = MODBUS_ERR_NONE;
    return (true);                                      /* Tell caller that we need to send a response              */
}
#endif
#endif

/*
********************************************************************************
*                               mbs_fc02_di_read()
*
* Description : Responds to a request to read the status of any number of Discrete Inputs (DIs).
*
* Argument(s) : pch       Is a pointer to the Modbus channel's data structure.
*
* Return(s)   : true      If a response needs to be sent
*               false     If not
*
* Caller(s)   : mbs_handler().
*
* Note(s)     : 1) RX command format:             Example:
*                  <slave address>                0x11
*                  <function code>                0x02
*                  <start address HI>             0x00
*                  <start address LO>             0xC4
*                  <# input statuses HI>          0x00
*                  <# input statuses LO>          0x16
*                  <Error Check (LRC or CRC)>     0x??
*
*               2) TX reply format:               Example:
*                  <slave address>                0x11
*                  <function code>                0x02
*                  <byte count>                   0x03
*                  <Data Inputs>                  0xAC  (Bit set to 1 means ON, Bit cleared means == OFF)
*                  <Data Inputs>                  0xDB  (Bit set to 1 means ON, Bit cleared means == OFF)
*                  <Data Inputs>                  0x35  (Bit set to 1 means ON, Bit cleared means == OFF)
*                  <Error Check (LRC or CRC)>     0x??
********************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN != 0)
#if (MODBUS_CFG_FC02_EN  != 0)
static bool mbs_fc02_di_read(mb_channel_t *pch)
{
    uint8_t   *presp;
    uint16_t    di;
    bool   di_val;
    uint16_t    err;
    uint16_t    nbr_di;
    uint16_t    nbr_bytes;
    uint8_t    bit_mask;
    uint16_t    ix;


    if (pch->rx_frame_ndata_bytes != 4)                            /* 4 data bytes for this message.                           */
    {
        return (false);                                      /* Tell caller that we DON'T need to send a response        */
    }
    di     = MBS_RX_DATA_START;                                  /* Get the starting address of the desired DIs              */
    nbr_di = MBS_RX_DATA_POINTS;                                 /* Find out how many DIs                                    */
    if (nbr_di == 0 || nbr_di > 2000) {                          /* Make sure we don't exceed the allowed limit per request  */
        pch->error = MODBUS_ERR_FC02_01;
        mbs_error_resp_set(pch,
                       MODBUS_ERR_ILLEGAL_DATA_QTY);
        return (true);                                       /* Tell caller that we need to send a response              */
    }
    nbr_bytes             = ((nbr_di - 1) / 8) + 1;              /* Find #bytes needed for response.                         */
    pch->tx_frame_ndata_bytes = nbr_bytes + 1;                      /* Number of data bytes + byte count.                       */
    presp                 = &pch->tx_frame_data[0];                /* Clear bytes in response                                  */
    for (ix = 0; ix < (nbr_bytes + 3); ix++) {
        *presp++ = 0x00;
    }
    bit_mask = 0x01;                                             /* Start with bit 0 in response byte data mask.             */
    ix       =    0;                                             /* Initialize loop counter.                                 */
    presp    = &pch->tx_frame_data[0];                             /* Reset the pointer to the start of the response           */
    *presp++ =  MBS_RX_FRAME_ADDR;                               /* Prepare response packet                                  */
    *presp++ =  MBS_RX_FRAME_FC;
    *presp++ = (uint8_t)nbr_bytes;                            /* Set number of data bytes in response message.            */
    while (ix < nbr_di)                                         /* Loop through each DI requested.                          */
    {
        /* Get the current value of the DI. */
        elab_assert(pch->cb.di_read != NULL);
        di_val = pch->cb.di_read(di, &err);
        switch (err)
        {
            case MODBUS_ERR_NONE:
                if (di_val == true)                  /* Only set data response bit if DI is on.                  */
                {
                    *presp |= bit_mask;
                }
                di ++;
                ix ++;                                           /* Increment DI counter.                                    */
                if ((ix % 8) == 0)                             /* Determine if 8 data bits have been filled.               */
                {
                    bit_mask   = 0x01;                          /* Reset the data mask.                                     */
                    presp ++;                                    /* Increment data frame index.                              */
                }
                else
                {                                        /* Still in same data byte, so                              */
                    bit_mask <<= 1;                             /* Shift the data mask to the next higher bit position.     */
                }
                break;

            case MODBUS_ERR_RANGE:
            default:
                 pch->error = MODBUS_ERR_FC02_02;
                 mbs_error_resp_set(pch,
                                MODBUS_ERR_ILLEGAL_DATA_ADDR);
                 return (true);                              /* Tell caller that we need to send a response              */
        }
    }
    pch->error = MODBUS_ERR_NONE;
    return (true);                                           /* Tell caller that we need to send a response              */
}
#endif
#endif

/*
********************************************************************************
*                           mbs_fc03_holding_reg_read()
*
* Description : Obtains the contents of the specified holding registers.
*
* Argument(s) : pch       Is a pointer to the Modbus channel's data structure.
*
* Return(s)   : true      If a response needs to be sent
*               false     If not
*
* Caller(s)   : mbs_handler().
*
* Note(s)     : 1) RX command format:             Example:
*                  <slave address>                0x11
*                  <function code>                0x03
*                  <start address HI>             0x00
*                  <start address LO>             0x6B
*                  <# registers HI>               0x00
*                  <# registers LO>               0x03
*                  <Error Check (LRC or CRC)>     0x??
*
*               2) TX reply format:               Example:
*                  <slave address>                0x11
*                  <function code>                0x03
*                  <byte count>                   0x06
*                  <Data HI register>             0x02
*                  <Data LO register>             0x2B
*                  <Data HI register>             0x00
*                  <Data LO register>             0x00
*                  <Data HI register>             0x00
*                  <Data LO register>             0x64
*                  <Error Check (LRC or CRC)>     0x??
********************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN != 0)
#if (MODBUS_CFG_FC03_EN  != 0)
static bool mbs_fc03_holding_reg_read(mb_channel_t *pch)
{
    uint8_t  *presp;
    uint16_t err;
    uint16_t   reg;
    uint16_t   nbr_regs;
    uint16_t   nbr_bytes;
    uint16_t   reg_val_16;
#if (MODBUS_CFG_FP_EN != 0)
    uint16_t   ix;
    float     reg_val_fp;
    uint8_t  *pfp;
#endif

    if (pch->rx_frame_ndata_bytes != 4)                            /* Nbr of data bytes must be 4.                             */
    {
        return (false);                                      /* Tell caller that we DON'T need to send a response        */
    }
    reg       = MBS_RX_DATA_START;
    nbr_regs  = MBS_RX_DATA_POINTS;
#if (MODBUS_CFG_FP_EN != 0)
    if (reg < MODBUS_CFG_FP_START_IX)                           /* See if we want integer registers                         */
    {
        if (nbr_regs == 0 || nbr_regs > 125)                    /* Make sure we don't exceed the allowed limit per request  */
        {
            pch->error = MODBUS_ERR_FC03_03;
            mbs_error_resp_set(pch, MODBUS_ERR_ILLEGAL_DATA_QTY);
            return (true);                                   /* Tell caller that we need to send a response              */
        }
        nbr_bytes = (uint8_t)(nbr_regs * sizeof(uint16_t)); /* Find #bytes needed for response.                         */
    }
    else
    {
        if (nbr_regs == 0 || nbr_regs > 62)                     /* Make sure we don't exceed the allowed limit per request  */
        {
            pch->error = MODBUS_ERR_FC03_04;
            mbs_error_resp_set(pch, MODBUS_ERR_ILLEGAL_DATA_QTY);
            return (true);                                   /* Tell caller that we need to send a response              */
        }
        nbr_bytes = (uint8_t)(nbr_regs * sizeof(float));   /* Find #bytes needed for response.                         */
    }
#else
    if (nbr_regs == 0 || nbr_regs > 125)                        /* Make sure we don't exceed the allowed limit per request  */
    {
        pch->error = MODBUS_ERR_FC03_03;
        mbs_error_resp_set(pch, MODBUS_ERR_ILLEGAL_DATA_QTY);
        return (true);                                       /* Tell caller that we need to send a response              */
    }
    nbr_bytes = (uint8_t)(nbr_regs * sizeof(uint16_t));     /* Find #bytes needed for response.                         */
#endif
    pch->tx_frame_ndata_bytes = nbr_bytes + 1;                      /* Number of data bytes + byte count.                       */
    presp                 = &pch->tx_frame_data[0];                /* Reset the pointer to the start of the response           */
    *presp++              =  MBS_RX_FRAME_ADDR;
    *presp++              =  MBS_RX_FRAME_FC;
    *presp++              = (uint8_t)nbr_bytes;               /* Set number of data bytes in response message             */

    /* Loop through each register requested. */
    while (nbr_regs > 0)
    {
        /* See if we want an integer register. */
        if (reg < MODBUS_CFG_FP_START_IX)
        {
            /* Yes, get its value. */
            elab_assert(pch->cb.holding_reg_read != NULL);
            reg_val_16 = pch->cb.holding_reg_read(reg, &err);
            switch (err)
            {
                case MODBUS_ERR_NONE:
                    *presp++ = (uint8_t)((reg_val_16 >> 8) & 0x00FF); /* MSB */
                    *presp++ = (uint8_t)(reg_val_16 & 0x00FF);        /* LSB */
                    break;

                case MODBUS_ERR_RANGE:
                default:
                    pch->error = MODBUS_ERR_FC03_01;
                    mbs_error_resp_set(pch, MODBUS_ERR_ILLEGAL_DATA_ADDR);
                    return (true);
            }
        }
        else
        {
#if (MODBUS_CFG_FP_EN != 0)
            /* No, get the value of the FP register. */
            elab_assert(pch->cb.holding_reg_read_fp != NULL);
            reg_val_fp = pch->cb.holding_reg_read_fp(reg, &err);
            switch (err)
            {
                case MODBUS_ERR_NONE:
                    /* Point to the FP register. */
                    pfp = (uint8_t *)&reg_val_fp;            
                    for (ix = 0; ix < sizeof(float); ix++)  /* Copy value to response buffer                            */
                    {
                        *presp++ = *pfp ++;
                    }
                    break;

                case MODBUS_ERR_RANGE:
                default:
                    pch->error = MODBUS_ERR_FC03_02;
                    mbs_error_resp_set(pch, MODBUS_ERR_ILLEGAL_DATA_ADDR);
                    return (true);
            }
#endif
        }

        /* Increment current register number. */
        reg ++;
        nbr_regs --;
    }
    pch->error = MODBUS_ERR_NONE;

    /* Tell caller that we need to send a response. */
    return (true);
}
#endif
#endif

/*
********************************************************************************
*                             mbs_fc04_input_reg_read()
*
* Description : Obtains the contents of the specified input registers.
*
* Argument(s) : pch       Is a pointer to the Modbus channel's data structure.
*
* Return(s)   : true      If a response needs to be sent
*               false     If not
*
* Caller(s)   : mbs_handler().
*
* Note(s)     : 1) RX command format:             Example:
*                  <slave address>                0x11
*                  <function code>                0x04
*                  <start address HI>             0x00
*                  <start address LO>             0x08
*                  <# registers HI>               0x00
*                  <# registers LO>               0x01
*                  <Error Check (LRC or CRC)>     0x??
*
*               2) TX reply format:               Example:
*                  <slave address>                0x11
*                  <function code>                0x04
*                  <byte count>                   0x02
*                  <Data HI register value>       0x00
*                  <Data LO register value>       0x0A
*                  <Error Check (LRC or CRC)>     0x??
********************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN != 0)
#if (MODBUS_CFG_FC04_EN  != 0)
static bool mbs_fc04_input_reg_read(mb_channel_t *pch)
{
    uint8_t  *presp;
    uint16_t err;
    uint16_t   reg;
    uint16_t   nbr_regs;
    uint16_t   nbr_bytes;
    uint16_t   reg_val_16;
#if (MODBUS_CFG_FP_EN != 0)
    uint16_t   ix;
    float     reg_val_fp;
    uint8_t  *pfp;
#endif


    if (pch->rx_frame_ndata_bytes != 4) {                           /* Nbr of data bytes must be 4.                             */
        return (false);                                      /* Tell caller that we DON'T need to send a response        */
    }
    reg       = MBS_RX_DATA_START;
    nbr_regs  = MBS_RX_DATA_POINTS;
#if (MODBUS_CFG_FP_EN != 0)
    if (reg < MODBUS_CFG_FP_START_IX) {                          /* See if we want integer registers                         */
        if (nbr_regs == 0 || nbr_regs > 125) {                   /* Make sure we don't exceed the allowed limit per request  */
            pch->error = MODBUS_ERR_FC04_03;
            mbs_error_resp_set(pch, MODBUS_ERR_ILLEGAL_DATA_QTY);
            return (true);                                   /* Tell caller that we need to send a response              */
        }
        nbr_bytes = (uint8_t)(nbr_regs * sizeof(uint16_t)); /* Find #bytes needed for response.                         */
    }
    else
    {
        if (nbr_regs == 0 || nbr_regs > 62) {                    /* Make sure we don't exceed the allowed limit per request  */
            pch->error = MODBUS_ERR_FC04_04;
            mbs_error_resp_set(pch, MODBUS_ERR_ILLEGAL_DATA_QTY);
            return (true);                                   /* Tell caller that we need to send a response              */
        }
        nbr_bytes = (uint8_t)(nbr_regs * sizeof(float));   /* Find #bytes needed for response.                         */
    }
#else
    if (nbr_regs == 0 || nbr_regs > 125)                        /* Make sure we don't exceed the allowed limit per request  */
    {
        pch->error = MODBUS_ERR_FC04_03;
        mbs_error_resp_set(pch,
                       MODBUS_ERR_ILLEGAL_DATA_QTY);
        return (true);                                       /* Tell caller that we need to send a response              */
    }
    nbr_bytes = (uint8_t)(nbr_regs * sizeof(uint16_t));     /* Find #bytes needed for response.                         */
#endif
    pch->tx_frame_ndata_bytes = nbr_bytes + 1;                      /* Number of data bytes + byte count.                       */
    presp                 = &pch->tx_frame_data[0];                /* Reset the pointer to the start of the response           */
    *presp++              =  MBS_RX_FRAME_ADDR;                  /* Prepare response packet                                  */
    *presp++              =  MBS_RX_FRAME_FC;
    *presp++              = (uint8_t)nbr_bytes;               /* Set number of data bytes in response message             */
    while (nbr_regs > 0) {                                       /* Loop through each register requested.                    */
        if (reg < MODBUS_CFG_FP_START_IX)                       /* See if we want an integer register                       */
        {
            elab_assert(pch->cb.in_reg_read != NULL);
            /* Yes, get its value. */
            reg_val_16 = pch->cb.in_reg_read(reg, &err);
            switch (err)
            {
                case MODBUS_ERR_NONE:
                    *presp++ = (uint8_t)((reg_val_16 >> 8) & 0x00FF); /*      Get MSB first.                             */
                    *presp++ = (uint8_t)(reg_val_16 & 0x00FF);        /*      Get LSB next.                              */
                    break;

                case MODBUS_ERR_RANGE:
                default:
                    pch->error = MODBUS_ERR_FC04_01;
                    mbs_error_resp_set(pch,
                                MODBUS_ERR_ILLEGAL_DATA_ADDR);
                    return (true);
            }
        }
        else
        {
#if (MODBUS_CFG_FP_EN != 0)
            elab_assert(pch->cb.in_reg_read_fp != NULL);
            /* No,  get the value of the FP register. */
            reg_val_fp = pch->cb.in_reg_read_fp(reg, &err);

            switch (err) {
                case MODBUS_ERR_NONE:
                    pfp = (uint8_t *)&reg_val_fp;            /* Point to the FP register                                 */
                    for (ix = 0; ix < sizeof(float); ix++) { /* Copy value to response buffer                            */
                        *presp++ = *pfp ++;
                    }
                    break;

                case MODBUS_ERR_RANGE:
                default:
                    pch->error = MODBUS_ERR_FC04_02;
                    mbs_error_resp_set(pch,
                                MODBUS_ERR_ILLEGAL_DATA_ADDR);
                    return (true);
            }
#endif
        }
        reg ++;                                                   /* Increment current register number                        */
        nbr_regs--;
    }
    pch->error = MODBUS_ERR_NONE;
    return (true);                                           /* Tell caller that we need to send a response              */
}
#endif
#endif

/*
********************************************************************************
*                              mbs_fc05_coil_write()
*
* Description : Responds to a request to force a coil to a specified state.
*
* Argument(s) : pch       Is a pointer to the Modbus channel's data structure.
*
* Return(s)   : true      If a response needs to be sent
*               false     If not
*
* Caller(s)   : mbs_handler().
*
* Note(s)     : 1) A value of 0xFF00 forces a coil ON and 0x0000 to OFF
*
*               2) RX command format:             Example:
*                  <slave address>                0x11
*                  <function code>                0x05
*                  <Coil address HI>              0x00
*                  <Coil address LO>              0xAC
*                  <Force coil value HI>          0xFF
*                  <Force coil value LO>          0x00
*                  <Error Check (LRC or CRC)>     0x??
*
*               3) TX reply format:               Example:
*                  <slave address>                0x11
*                  <function code>                0x05
*                  <Coil address HI>              0x00
*                  <Coil address LO>              0xAC
*                  <Force coil value HI>          0xFF
*                  <Force coil value LO>          0x00
*                  <Error Check (LRC or CRC)>     0x??
********************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN != 0)
#if (MODBUS_CFG_FC05_EN  != 0)
static bool mbs_fc05_coil_write(mb_channel_t *pch)
{
    uint8_t   *prx_data;
    uint8_t   *ptx_data;
    uint8_t    i;
    uint16_t    coil;
    bool   coil_val;
    uint16_t    temp;
    uint16_t    err;


    if (pch->rx_frame_ndata_bytes != 4)                            /* Nbr of data bytes must be 4.                             */
    {
        return (false);                                      /* Tell caller that we DON'T need to send a response        */
    }
    coil = MBS_RX_DATA_START;                                    /* Get the desired coil number                              */
    temp = MBS_RX_DATA_COIL;
    if (pch->write_en == true)
    {
        if (temp == MODBUS_COIL_OFF_CODE) {                      /* See if coil needs to be OFF?                             */
            coil_val = 0;                                        /* Yes, Turn coil OFF                                       */
        }
        else
        {
            coil_val = 1;                                        /* No,  Turn coil ON                                        */
        }
        /* Force coil. */
        elab_assert(pch->cb.coil_write != NULL);
        pch->cb.coil_write(coil, coil_val, &err);
    }
    else
    {
        pch->error = MODBUS_ERR_FC05_02;
        mbs_error_resp_set(pch,                                      /* Writes are not enabled                                   */
                       MODBUS_ERR_ILLEGAL_DATA_VAL);
        return (true);                                       /* Tell caller that we need to send a response              */
    }
    pch->tx_frame_ndata_bytes = 4;
    MBS_TX_FRAME_ADDR      = MBS_RX_FRAME_ADDR;                  /* Prepare response packet                                  */
    MBS_TX_FRAME_FC        = MBS_RX_FRAME_FC;
    prx_data               = &pch->rx_frame_data[2];               /* Copy four data bytes from the receive packet             */
    ptx_data               = &pch->tx_frame_data[2];
    for (i = 0; i < 4; i++)
    {
        *ptx_data++ = *prx_data ++;
    }
    switch (err)
    {
        case MODBUS_ERR_NONE:                                    /* We simply echoe back with the command received           */
            pch->error = MODBUS_ERR_NONE;
            break;

        case MODBUS_ERR_RANGE:
        default:
            pch->error = MODBUS_ERR_FC05_01;
            mbs_error_resp_set(pch,
                        MODBUS_ERR_ILLEGAL_DATA_ADDR);
            break;
    }
    return (true);                                           /* Tell caller that we need to send a response              */
}
#endif
#endif

/*
********************************************************************************
*                           mbs_fc06_holding_reg_wr()
*
* Description : Change the value of a single register.
*
* Argument(s) : pch       Is a pointer to the Modbus channel's data structure.
*
* Return(s)   : true      If a response needs to be sent
*               false     If not
*
* Caller(s)   : mbs_handler().
*
* Note(s)     : 1) RX command format:             Example:
*                  <slave address>                0x11
*                  <function code>                0x06
*                  <start address HI>             0x00
*                  <start address LO>             0x01
*                  <Register value HI>            0x00
*                  <Register value LO>            0x03
*                  <Error Check (LRC or CRC)>     0x??
*
*               2) TX reply format:               Example:
*                  <slave address>                0x11
*                  <function code>                0x06
*                  <start address HI>             0x00
*                  <start address LO>             0x01
*                  <Register value HI>            0x00
*                  <Register value LO>            0x03
*                  <Error Check (LRC or CRC)>     0x??
********************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN != 0)
#if (MODBUS_CFG_FC06_EN  != 0)
static bool mbs_fc06_holding_reg_wr(mb_channel_t *pch)
{
    uint8_t  *prx_data;
    uint8_t  *ptx_data;
    uint8_t   i;
    uint8_t   max;
    uint16_t err;
    uint16_t reg = MBS_RX_DATA_START;
    uint16_t   reg_val_16;
#if (MODBUS_CFG_FP_EN != 0)
    float     reg_val_fp = 1.0;
    uint8_t  *pfp;
#endif

    /* Nbr of data bytes must be 4. */
    if (pch->rx_frame_ndata_bytes != 4 && reg < MODBUS_CFG_FP_START_IX)
    {
        return (false);
    }

    /* Nbr of data bytes must be 6. */
    if (pch->rx_frame_ndata_bytes != 6 && reg >= MODBUS_CFG_FP_START_IX)
    {
        return (false);
    }
    
    
    /* Nbr of data bytes must be 6. */
    if (pch->rx_frame_ndata_bytes != 6 && reg >= MODBUS_CFG_FP_START_IX)
    {
        return (false);
    }
    
#if (MODBUS_CFG_FP_EN != 0)
    if (reg < MODBUS_CFG_FP_START_IX)
    {
        reg_val_16 = MBS_RX_DATA_REG;
        /* Write to integer register. */
        elab_assert(pch->cb.holding_reg_write != NULL);
        pch->cb.holding_reg_write(reg, reg_val_16, &err);
    }
    else
    {
        prx_data = &pch->rx_frame_data[4];                         /* Point to data in the received frame.                     */
        pfp      = (uint8_t *)&reg_val_fp;
        for (i = 0; i < sizeof(float); i++) {
            *pfp++ = *prx_data ++;
        }
        /* Write to floating point register. */
        elab_assert(pch->cb.holding_reg_write_fp != NULL);
        pch->cb.holding_reg_write_fp(reg, reg_val_fp, &err);
    }
#else
    reg_val_16 = MBS_RX_DATA_REG;
    /* Write to integer register. */
    elab_assert(pch->cb.holding_reg_write != NULL);
    pch->cb.holding_reg_write(reg, reg_val_16, &err);
#endif
    pch->tx_frame_ndata_bytes = 4;
    MBS_TX_FRAME_ADDR      = MBS_RX_FRAME_ADDR;                  /* Prepare response packet (duplicate Rx frame)             */
    MBS_TX_FRAME_FC        = MBS_RX_FRAME_FC;
    prx_data               = &pch->rx_frame_data[2];               /* Copy received register address and data to response      */
    ptx_data               = &pch->tx_frame_data[2];
    if (reg < MODBUS_CFG_FP_START_IX)
    {
        max = sizeof(uint16_t) + 2;
    }
    else
    {
        max = sizeof(float) + 2;
    }
    for (i = 0; i < max; i++) {
        *ptx_data++ = *prx_data ++;
    }
    switch (err)
    {
        case MODBUS_ERR_NONE:                                    /* Reply with echoe of command received                     */
            pch->error = MODBUS_ERR_NONE;
            break;

        case MODBUS_ERR_RANGE:
        default:
            pch->error = MODBUS_ERR_FC06_01;
            mbs_error_resp_set(pch, MODBUS_ERR_ILLEGAL_DATA_ADDR);
            break;
    }
    return (true);
}
#endif
#endif

/*
********************************************************************************
*                          mbs_fc15_coil_write_multi()
*
* Description : Processes the MODBUS "Force Multiple COILS" command and writes the COIL states.
*
* Argument(s) : pch       Is a pointer to the Modbus channel's data structure.
*
* Return(s)   : true      If a response needs to be sent
*               false     If not
*
* Caller(s)   : mbs_handler().
*
* Note(s)     : 1) RX command format:             Example:
*                  <slave address>                0x11
*                  <function code>                0x0F
*                  <Coil address HI>              0x00
*                  <Coil address LO>              0x13
*                  <# coils HI>                   0x00
*                  <# coils LO>                   0x0A
*                  <byte count>                   0x02
*                  <Force Data HI>                0xCD
*                  <Force Data LO>                0x01
*                  <Error Check (LRC or CRC)>     0x??
*
*               2) TX reply format:               Example:
*                  <slave address>                0x11
*                  <function code>                0x0F
*                  <Coil address HI>              0x00
*                  <Coil address LO>              0x13
*                  <# coils HI>                   0x00
*                  <# coils LO>                   0x0A
*                  <Error Check (LRC or CRC)>     0x??
********************************************************************************
*/
uint32_t coun__t = 0;
#if (MODBUS_CFG_SLAVE_EN != 0)
#if (MODBUS_CFG_FC15_EN  != 0)
static bool mbs_fc15_coil_write_multi(mb_channel_t *pch)
{
    uint16_t   ix;
    uint16_t   coil;
    uint16_t   nbr_coils;
    uint16_t   nbr_bytes;
    uint16_t   data_ix;
    bool  coil_val;
    uint8_t   temp;
    uint16_t err;

    if (pch->write_en == true)
    {
        /* Minimum Nbr of data bytes must be 6. */
        if (pch->rx_frame_ndata_bytes < 6)
        {                        
            return (false);                                     /* Tell caller that we DON'T need to send a response        */
        }
        coil      = MBS_RX_DATA_START;
        nbr_coils = MBS_RX_DATA_POINTS;
        nbr_bytes = MBS_RX_DATA_BYTES;                          /* Get the byte count for the data. */
        if (((((nbr_coils - 1) / 8) + 1) ==  nbr_bytes) &&      /* Be sure #bytes valid for number COILS. */
            (pch->rx_frame_ndata_bytes  == (nbr_bytes + 5)))
        {
            ix      = 0;                                        /* Initialize COIL/loop counter variable. */
            data_ix = 7;                                        /* The 1st COIL data byte is 5th element in data frame. */
            /* Loop through each COIL to be forced. */
            while (ix < nbr_coils)
            {
                if ((ix % 8) == 0)
                {                             /* Move to the next data byte after every eight bits.       */
                    temp = pch->rx_frame_data[data_ix++];
                }
                if (temp & 0x01)
                {                                               /* Get LSBit */
                    coil_val = true;
                }
                else
                {
                    coil_val = false;
                }
                elab_assert(pch->cb.coil_write != NULL);
                pch->cb.coil_write(coil + ix, coil_val, &err);
                switch (err)
                {
                    case MODBUS_ERR_NONE:
                        break;                                  /* Continue with the next coil if no error                  */

                    case MODBUS_ERR_RANGE:
                    default:
                        pch->error = MODBUS_ERR_FC15_01;
                        mbs_error_resp_set(pch, MODBUS_ERR_ILLEGAL_DATA_ADDR);
                        return (true);                          /* Tell caller that we need to send a response              */
                }
                temp >>= 1;                                     /* Shift the data one bit position to the right.            */
                ix ++;                                          /* Increment the COIL counter.                              */
            }
        }
        else
        {
            pch->error = MODBUS_ERR_FC15_02;
            mbs_error_resp_set(pch, MODBUS_ERR_ILLEGAL_DATA_VAL);
            return (true);                                   /* Tell caller that we need to send a response              */
        }
        pch->tx_frame_ndata_bytes = 4;                              /* Don't echo the whole message back!                       */
        MBS_TX_FRAME_ADDR      = MBS_RX_FRAME_ADDR;              /* Prepare response packet                                  */
        MBS_TX_FRAME_FC        = MBS_RX_FRAME_FC;
        MBS_TX_DATA_START_H    = MBS_RX_DATA_START_H;
        MBS_TX_DATA_START_L    = MBS_RX_DATA_START_L;
        MBS_TX_DATA_POINTS_H   = MBS_RX_DATA_POINTS_H;
        MBS_TX_DATA_POINTS_L   = MBS_RX_DATA_POINTS_L;
        pch->error             = MODBUS_ERR_NONE;
    }
    else
    {
        /* Number of bytes incorrect for number of COILS. */
        pch->error = MODBUS_ERR_FC15_03;
        mbs_error_resp_set(pch, MODBUS_ERR_ILLEGAL_DATA_VAL);
    }
    return (true);                                            /* Tell caller that we need to send a response              */
}
#endif
#endif

/*
********************************************************************************
*                                    mbs_fc16_holding_reg_write_multi()
*
* Description : This function is called to write to multiple holding registers.  If the address of the
*               rquest exceeds or is equal to MODBUS_CFG_FP_START_IX, then the command would write to
*               multiple 'floating-point' according to the 'Daniels Flow Meter' extensions.  This means
*               that each register requested is considered as a 32-bit IEEE-754 floating-point format.
*
* Argument(s) : pch       Is a pointer to the Modbus channel's data structure.
*
* Return(s)   : true      If a response needs to be sent
*               false     If not
*
* Caller(s)   : mbs_handler().
*
* Note(s)     : 1) RX command format:             Example:
*                  <slave address>                0x11
*                  <function code>                0x10
*                  <start address HI>             0x00
*                  <start address LO>             0x01
*                  <# registers HI>               0x00
*                  <# registers LO>               0x02
*                  <byte count>                   0x04
*                  <Register value HI>            0x00
*                  <Register value LO>            0x0A
*                  <Register value HI>            0x01
*                  <Register value LO>            0x02
*                  <Error Check (LRC or CRC)>     0x??
*
*               2) TX reply format:               Example:
*                  <slave address>                0x11
*                  <function code>                0x10
*                  <start address HI>             0x00
*                  <start address LO>             0x01
*                  <# registers HI>               0x00
*                  <# registers LO>               0x02
*                  <Error Check (LRC or CRC)>     0x??
********************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN != 0)
#if (MODBUS_CFG_FC16_EN  != 0)
static bool mbs_fc16_holding_reg_write_multi(mb_channel_t *pch)
{
    uint8_t  *prx_data;
    uint16_t err;
    uint16_t   reg;
    uint16_t   reg_val_16;
    uint16_t   nbr_regs;
    uint16_t   nbr_bytes;
    uint8_t   data_size;
#if (MODBUS_CFG_FP_EN != 0)
    uint8_t   i;
    float     reg_val_fp = 0.0;
    uint8_t  *pfp;
#endif


    reg       = MBS_RX_DATA_START;
    nbr_regs  = MBS_RX_DATA_POINTS;
#if (MODBUS_CFG_FP_EN != 0)
    if (reg < MODBUS_CFG_FP_START_IX) {
        if (nbr_regs == 0 || nbr_regs > 125) {                   /* Make sure we don't exceed the allowed limit per request  */
            pch->error = MODBUS_ERR_FC16_04;
            mbs_error_resp_set(pch, MODBUS_ERR_ILLEGAL_DATA_QTY);
            return (true);                                   /* Tell caller that we need to send a response              */
        }
        data_size  = sizeof(uint16_t);
    }
    else
    {
        if (nbr_regs == 0 || nbr_regs > 62) {                    /* Make sure we don't exceed the allowed limit per request  */
            pch->error = MODBUS_ERR_FC16_05;
            mbs_error_resp_set(pch, MODBUS_ERR_ILLEGAL_DATA_QTY);
            return (true);                                   /* Tell caller that we need to send a response              */
        }
        data_size  = sizeof(float);
    }
#else
    if (nbr_regs == 0 || nbr_regs > 125) {                       /* Make sure we don't exceed the allowed limit per request  */
        pch->error = MODBUS_ERR_FC16_04;
        mbs_error_resp_set(pch,
                       MODBUS_ERR_ILLEGAL_DATA_QTY);
        return (true);                                       /* Tell caller that we need to send a response              */
    }
    data_size  = sizeof(uint16_t);
#endif

    prx_data  = &pch->rx_frame_data[6];                            /* Point to number of bytes in request frame                */
    nbr_bytes = (uint16_t)*prx_data ++;
    if ((pch->rx_frame_ndata_bytes - 5) != nbr_bytes) {             /* Compare actual number of bytes to what they say.         */
        pch->error = MODBUS_ERR_FC16_01;
        mbs_error_resp_set(pch,
                       MODBUS_ERR_ILLEGAL_DATA_QTY);
        return (true);
    }
    if ((nbr_bytes / nbr_regs) != (uint16_t)data_size) {
        pch->error = MODBUS_ERR_FC16_02;
        mbs_error_resp_set(pch, MODBUS_ERR_ILLEGAL_DATA_VAL);
        return (true);                                       /* Tell caller that we need to send a response              */
    }
    while (nbr_regs > 0) {
#if (MODBUS_CFG_FP_EN != 0)
        if (reg < MODBUS_CFG_FP_START_IX) {
            reg_val_16  = ((uint16_t)*prx_data++) << 8;        /* Get MSB first.                                           */
            reg_val_16 +=  (uint16_t)*prx_data ++;              /* Add in the LSB.                                          */
            elab_assert(pch->cb.holding_reg_write != NULL);
            pch->cb.holding_reg_write(reg, reg_val_16, &err);
        }
        else
        {
            pfp = (uint8_t *)&reg_val_fp;
            for (i = 0; i < sizeof(float); i++) {
                *pfp++ = *prx_data ++;
            }
            elab_assert(pch->cb.holding_reg_write_fp != NULL);
            pch->cb.holding_reg_write_fp(reg, reg_val_fp, &err);
        }
#else
        reg_val_16  = ((uint16_t)*prx_data++) << 8;            /* Get MSB first.                                           */
        reg_val_16 +=  (uint16_t)*prx_data ++;                  /* Add in the LSB.                                          */
        elab_assert(pch->cb.holding_reg_write != NULL);
        pch->cb.holding_reg_write(reg, reg_val_16, &err);
#endif

        switch (err) {                                           /* See if any errors in writing the data                    */
            case MODBUS_ERR_NONE:                                /* Reply with echoe of command received                     */
                reg ++;
                nbr_regs--;
                break;

            case MODBUS_ERR_RANGE:
            default:
                pch->error = MODBUS_ERR_FC16_03;
                mbs_error_resp_set(pch,
                            MODBUS_ERR_ILLEGAL_DATA_ADDR);
                return (true);                              /* Tell caller that we need to send a response              */
        }
    }
    pch->tx_frame_ndata_bytes = 4;                                  /* Don't echo the whole message back!                       */
    MBS_TX_FRAME_ADDR      = MBS_RX_FRAME_ADDR;                  /* Prepare response packet                                  */
    MBS_TX_FRAME_FC        = MBS_RX_FRAME_FC;
    MBS_TX_DATA_START_H    = MBS_RX_DATA_START_H;
    MBS_TX_DATA_START_L    = MBS_RX_DATA_START_L;
    MBS_TX_DATA_POINTS_H   = MBS_RX_DATA_POINTS_H;
    MBS_TX_DATA_POINTS_L   = MBS_RX_DATA_POINTS_L;
    return (true);                                           /* Tell caller that we need to send a response              */
}
#endif
#endif

/*
********************************************************************************
*                              mbs_rx_task()
*
* Description : Handle either Modbus ASCII or Modbus RTU received packets.
*
* Argument(s) : ch       Specifies the Modbus channel that needs servicing.
*
* Return(s)   : none.
*
* Caller(s)   : mb_rx_task().
*
* Note(s)     : none.
********************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN != 0)
void mbs_rx_task(mb_channel_t *pch)
{
#if (MODBUS_CFG_ASCII_EN != 0)
    if (pch->mode == MODBUS_MODE_ASCII)
    {
        mbs_ascii_task(pch);
    }
#endif

#if (MODBUS_CFG_RTU_EN != 0)
    if (pch->mode == MODBUS_MODE_RTU)
    {
        mbs_rtu_task(pch);
    }
#endif
}
#endif

/*
********************************************************************************
*                              mbs_ascii_task()
*
* Description : Received a packet that should be encoded for Modbus ASCII mode.  Process request.
*
* Argument(s) : pch         Is a pointer to the Modbus channel's data structure.
*
* Return(s)   : none.
*
* Caller(s)   : mbs_rx_task().
*
* Note(s)     : none.
********************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN != 0) && \
    (MODBUS_CFG_ASCII_EN != 0)
static void mbs_ascii_task(mb_channel_t *pch)
{
    if (pch->rx_buff_byte_count >= MODBUS_ASCII_MIN_MSG_SIZE)
    {
        /* Extract received command from .rx_buff[] & move to .rx_frame_data[] */
        bool ok = mb_ascii_rx(pch);
        if (ok == true)
        {
            /* Calculate LRC on received ASCII packet. */
            uint16_t calc_lrc = mb_ascii_rx_calc_lrc(pch);
            /* If sum of all data plus received LRC is not the same. */
            if (calc_lrc == pch->rx_frame_crc)
            {
                /* Execute received command and formulate a response. */
                bool send_reply = mbs_handler(pch);
                if (send_reply == true)
                {
                    mb_ascii_tx(pch);                     /* Send back reply. */
                }
            }
        }
    }
    pch->rx_buff_byte_count = 0;
    pch->p_rx_buff     = &pch->rx_buff[0];
}
#endif

/*
********************************************************************************
*                               mbs_rtu_task()
*
* Description : This function processes a packet received on the Modbus channel assuming that it's an RTU
*               packet.
*
* Argument(s) : pch      Is a pointer to the Modbus channel's data structure.
*
* Return(s)   : none.
*
* Caller(s)   : mbs_rtu_task().
*
* Note(s)     : none.
********************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN != 0) && (MODBUS_CFG_RTU_EN   != 0)
static void mbs_rtu_task(mb_channel_t *pch)
{
    if (pch->rx_buff_byte_count >= MODBUS_RTU_MIN_MSG_SIZE)
    {
        /* Extract received command from .rx_buff[] & move to .rx_frame_data[] */
        bool ok = mb_rtu_rx(pch);
        if (ok == true)
        {
            /* Do our own calculation of the CRC. */
            uint16_t calc_crc = mb_rtu_rx_calc_crc(pch);
            /* If the calculated CRC does not match the CRC received, */
            if (calc_crc == pch->rx_frame_crc)
            {
                /* Execute received command and formulate a response */
                bool send_reply = mbs_handler(pch);
                if (send_reply == true)
                {
                    mb_rtu_tx(pch);                     /* Send back reply. */
                }
            }
        }
    }
    pch->rx_buff_byte_count = 0;
    pch->p_rx_buff = &pch->rx_buff[0];
}
#endif
#endif

/* ----------------------------- end of file -------------------------------- */
