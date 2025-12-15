/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include "../../common/elab_assert.h"
#include "../../common/elab_common.h"
#include "modbus.h"

ELAB_TAG("Modbus");

/* Exported function prototypes ----------------------------------------------*/
#if (MODBUS_CFG_ASCII_EN != 0)
uint8_t *mb_ascii_bin_to_hex(uint8_t value, uint8_t *pbuf);
uint8_t mb_ascii_hex_to_bin(uint8_t *phex);
uint8_t mb_ascii_tx_calc_lrc(mb_channel_t *pch, uint16_t tx_bytes);
void mb_ascii_rx_byte(mb_channel_t *pch, uint8_t rx_byte);
bool mb_ascii_rx(mb_channel_t *pch);
void mb_ascii_tx(mb_channel_t *pch);
#endif

#if (MODBUS_CFG_RTU_EN != 0)
uint16_t mb_rtu_calc_crc(mb_channel_t *pch);
uint16_t mb_rtu_tx_calc_crc(mb_channel_t *pch);
uint16_t mb_rtu_rx_calc_crc(mb_channel_t *pch);
void mb_rtu_rx_byte(mb_channel_t *pch, uint8_t rx_byte);
bool mb_rtu_rx(mb_channel_t *pch);
void mb_rtu_tx(mb_channel_t *pch);
#endif

#if (MODBUS_CFG_SLAVE_EN != 0)
void mbs_rx_task(mb_channel_t *pch);
#endif

void entry_slave_rx(void *paras);

/* Private variables ---------------------------------------------------------*/
static const osMutexAttr_t mutex_attr_mbm =
{
    "mutex_modbus",
    osMutexRecursive | osMutexPrioInherit,
    NULL,
    0U 
};

#if (MODBUS_CFG_RTU_EN != 0)
static const osThreadAttr_t mb_thread_rx_attr =
{
    .name = "mb_thread_rx",
    .attr_bits = osThreadDetached,
    .priority = osPriorityRealtime,
    .stack_size = 2048,
};
#endif

/* Exported functions --------------------------------------------------------*/
/**
  * @brief  Initialize each of the Modbus channels in your system.
  * @param  serial          The input serial device name.
  * @param  node_addr       Modbus slave node address.
  * @param  master_slave    MODBUS_MASTER or MODBUS_SLAVE.
  * @param  rx_timeout      Modbus slave rx timeout.
  * @param  modbus_mode     MODBUS_MODE_ASCII or MODBUS_MODE_RTU.
  * @retval Modbus channel handle.
  */
mb_channel_t *mb_channel_create(const char *serial,
                                uint8_t node_addr, uint8_t master_slave,
                                uint16_t rx_timeout, uint8_t modbus_mode)
{
    elab_assert(serial != NULL);
    elab_assert(master_slave == MODBUS_MASTER || master_slave == MODBUS_SLAVE);
    elab_assert(modbus_mode == MODBUS_MODE_ASCII || modbus_mode == MODBUS_MODE_RTU);
    if (master_slave == MODBUS_SLAVE)
    {
        elab_assert(rx_timeout > 0 && rx_timeout < 1000);
    }

    mb_channel_t *pch = elab_malloc(sizeof(mb_channel_t));
    elab_assert(pch != NULL);
    memset(pch, 0, sizeof(mb_channel_t));

    pch->m_or_s = master_slave;
    pch->mode = modbus_mode;
    pch->write_en = false;
    pch->node_addr = node_addr;
    pch->thread_slave = NULL;
    pch->mutex = NULL;
    pch->p_rx_buff = pch->rx_buff;
    pch->p_tx_buff = pch->tx_buff;

    pch->serial = elab_device_find(serial);
    elab_assert(pch->serial != NULL);

    /* Open the serial device. */
    elab_err_t ret = elab_device_open(pch->serial);
    elab_assert(ret == ELAB_OK);

#if (MODBUS_CFG_MASTER_EN != 0)
    if (pch->m_or_s == MODBUS_MASTER)
    {
        pch->mutex = osMutexNew(&mutex_attr_mbm);
        elab_assert(pch->mutex != NULL);
    }
    pch->rx_timeout = rx_timeout;
    pch->size_expect = 0;
#endif

#if (MODBUS_CFG_SLAVE_EN != 0)
    if (pch->m_or_s == MODBUS_SLAVE)
    {
        pch->thread_slave = osThreadNew(entry_slave_rx, pch, &mb_thread_rx_attr);
        elab_assert(pch->thread_slave != NULL);
    }

    /* Freq * 5 char * 10 bits/char * 1 / baud_rate */
    elab_serial_attr_t config = elab_serial_get_attr(pch->serial);
    uint16_t cnts = ((uint32_t)1000 * 5L * 10L) / config.baud_rate;
    if (cnts <= 1)
    {
        cnts = 5;
    }
    pch->rtu_timeout = cnts;

    pch->cb.coil_read = NULL;
    pch->cb.coil_write = NULL;
    pch->cb.di_read = NULL;
    pch->cb.in_reg_read = NULL;
    pch->cb.in_reg_read_fp = NULL;
    pch->cb.holding_reg_read = NULL;
    pch->cb.holding_reg_read_fp = NULL;
    pch->cb.holding_reg_write = NULL;
    pch->cb.holding_reg_write_fp = NULL;
#endif

    return pch;
}

/**
 * @brief  Set modbus slave channel write enabling.
 * @param  pch      Modbus channel handle.
 * @param  status   Enabling status.
 * @retval None.
 */
#if (MODBUS_CFG_SLAVE_EN != 0)
void mb_slave_write_enable(mb_channel_t *pch, bool status)
{
    elab_assert(pch != NULL);
    pch->write_en = status;
}
#endif

/**
 * @brief  Destroy modbus channel.
 * @param  pch      Modbus channel handle.
 * @retval None.
 */
void mb_channel_destroy(mb_channel_t *pch)
{
    osStatus_t ret_os = osOK;
    elab_err_t ret = ELAB_OK;

#if (MODBUS_CFG_SLAVE_EN != 0)
    if (pch->m_or_s == MODBUS_SLAVE)
    {
        ret_os = osThreadTerminate(pch->thread_slave);
        elab_assert(ret_os == osOK);
    }
#endif

    /* Close the serial device. */
    ret = elab_device_close(pch->serial);
    elab_assert(ret == ELAB_OK);

#if (MODBUS_CFG_MASTER_EN != 0)
    if (pch->m_or_s == MODBUS_MASTER)
    {
        ret_os = osMutexDelete(pch->mutex);
        elab_assert(ret_os == osOK);
    }
#endif

    elab_free(pch);
}

/**
 * @brief  Set modbus slave callback.
 * @param  pch      Modbus channel handle.
 * @param  cb       Modbus slave callback.
 * @retval None.
 */
#if (MODBUS_CFG_SLAVE_EN != 0)
void mb_slave_set_cb(mb_channel_t *pch, mb_channel_cb_t *cb)
{
    elab_assert(pch->m_or_s == MODBUS_SLAVE);

    pch->cb.coil_read = cb->coil_read;
    pch->cb.coil_write = cb->coil_write;
    pch->cb.di_read = cb->di_read;
    pch->cb.in_reg_read = cb->in_reg_read;
    pch->cb.in_reg_read_fp = cb->in_reg_read_fp;
    pch->cb.holding_reg_read = cb->holding_reg_read;
    pch->cb.holding_reg_read_fp = cb->holding_reg_read_fp;
    pch->cb.holding_reg_write = cb->holding_reg_write;
    pch->cb.holding_reg_write_fp = cb->holding_reg_write_fp;
}
#endif

/**
 * @brief  A byte has been received from a serial port.  We just store it in the 
 *         buffer for processingwhen a complete packet has been received.
 * @param  pch      Modbus channel handle.
 * @param  rx_byte  Is the byte received.
 * @retval None.
 */
void mb_rx_byte(mb_channel_t *pch, uint8_t rx_byte)
{
#if (MODBUS_CFG_ASCII_EN != 0)
    if (pch->mode == MODBUS_MODE_ASCII)
    {
        mb_ascii_rx_byte(pch, rx_byte & 0x7F);
        return;
    }
#endif

#if (MODBUS_CFG_RTU_EN != 0)
    if (pch->mode == MODBUS_MODE_RTU)
    {
        mb_rtu_rx_byte(pch, rx_byte);
        return;
    }
#endif

    elab_assert(false);
}

/**
 * @brief  A byte has been received from a serial port.  We just store it in the 
 *         buffer for processing when a complete packet has been received.
 * @param  pch      Modbus channel handle.
 * @param  rx_byte  Is the byte received.
 * @retval None.
 */
#if (MODBUS_CFG_ASCII_EN != 0)
void mb_ascii_rx_byte(mb_channel_t *pch, uint8_t rx_byte)
{
    uint8_t node_addr;
    uint8_t *phex;

    /* Increment the number of bytes received. */
    pch->rx_count ++;
    /* Is it the start of frame character? */
    if (rx_byte == ':')
    {
        /* Yes, Restart a new frame. */
        pch->p_rx_buff = &pch->rx_buff[0];
        pch->rx_buff_byte_count = 0;
    }
    /* No, add received byte to buffer. */
    if (pch->rx_buff_byte_count < MODBUS_CFG_BUF_SIZE)
    {
        /* Increment byte counter to see if we have Rx activity. */
        *pch->p_rx_buff ++ = rx_byte;
        pch->rx_buff_byte_count ++;
    }
    /* See if we received a complete ASCII frame. */
    if (rx_byte == MODBUS_ASCII_END_FRAME_CHAR2)
    {
        phex = &pch->rx_buff[1];
        node_addr = mb_ascii_hex_to_bin(phex);
        /* Is the address for us or a 'broadcast'? */
        if ((node_addr == pch->node_addr) || (node_addr == 0))
        {
            mbs_rx_task(pch);           /* Yes, Let task handle reply */
        }
        else
        {
            /* No,  Wipe out anything, we have to re-synchronize. */
            pch->p_rx_buff = &pch->rx_buff[0];
            pch->rx_buff_byte_count = 0;
        }
    }
}
#endif

/**
 * @brief  Parses and converts an ASCII style message into a Modbus frame. A 
 *         check is performed to verify that the Modbus packet is valid.
 * @param  pch      Modbus channel handle.
 * @retval None.
 */
#if (MODBUS_CFG_ASCII_EN != 0)
bool mb_ascii_rx(mb_channel_t *pch)
{
    uint8_t *pmsg;
    uint8_t *prx_data;
    uint16_t rx_size;

    pmsg      = &pch->rx_buff[0];
    rx_size   =  pch->rx_buff_byte_count;
    prx_data  = &pch->rx_frame_data[0];
    
    /* Message should have an ODD nbr of bytes. */
    if ((rx_size & 0x01) &&
        /* Check if message is long enough. */
        (rx_size > MODBUS_ASCII_MIN_MSG_SIZE) &&
        /* Check the first char. */
        (pmsg[0] == MODBUS_ASCII_START_FRAME_CHAR) &&
        /* Check the last two. */
        (pmsg[rx_size - 2] == MODBUS_ASCII_END_FRAME_CHAR1)  &&
        (pmsg[rx_size - 1] == MODBUS_ASCII_END_FRAME_CHAR2))
    {
        rx_size -= 3;                   /* Take away for the ':', CR, and LF */
        pmsg ++;                        /* Point past the ':' to the address. */
        pch->rx_frame_ndata_bytes = 0;  /* Get the data from the message. */
        while (rx_size > 2)
        {
            *prx_data++  = mb_ascii_hex_to_bin(pmsg);
            pmsg += 2;
            rx_size -= 2;
            /* Increment the number of Modbus packets received. */
            pch->rx_frame_ndata_bytes ++;
        }
        /* Subtract the Address and function code. */
        pch->rx_frame_ndata_bytes -= 2;
        /* Extract the message's LRC. */
        pch->rx_frame_crc = (uint16_t)mb_ascii_hex_to_bin(pmsg); 

        return (true);
    }
    else
    {
        return (false);
    }
}
#endif

/**
 * @brief  The format of the message is ASCII.  The actual information is taken 
 *         from the given MODBUS frame.
 * @param  pch      Modbus channel handle.
 * @retval None.
 */
#if (MODBUS_CFG_ASCII_EN != 0)
void mb_ascii_tx(mb_channel_t *pch)
{
    elab_err_t ret = ELAB_OK;

    uint8_t  *ptx_data;
    uint8_t  *pbuf;
    uint16_t   i;
    uint16_t   tx_bytes;
    uint8_t   lrc;

    ptx_data = &pch->tx_frame_data[0];
    pbuf     = &pch->tx_buff[0];
    *pbuf++  = MODBUS_ASCII_START_FRAME_CHAR;
    pbuf     = mb_ascii_bin_to_hex(*ptx_data++, pbuf);
    pbuf     = mb_ascii_bin_to_hex(*ptx_data++, pbuf);
    
    /* Transmit the actual data. */
    tx_bytes = 5;
    i = (uint8_t)pch->tx_frame_ndata_bytes;
    while (i > 0)
    {
        pbuf = mb_ascii_bin_to_hex(*ptx_data ++, pbuf);
        tx_bytes += 2;
        i --;
    }
    /* Compute outbound packet LRC. */
    lrc = mb_ascii_tx_calc_lrc(pch, tx_bytes);
    /* Add the LRC checksum in the packet. */
    pbuf = mb_ascii_bin_to_hex(lrc, pbuf);
    *pbuf++ = MODBUS_ASCII_END_FRAME_CHAR1;
    *pbuf++ = MODBUS_ASCII_END_FRAME_CHAR2;
    tx_bytes += 4;
    /* Save the computed LRC into the channel. */
    pch->tx_frame_crc = (uint16_t)lrc;
    /* Update the total number of bytes to send. */
    pch->tx_buff_byte_count = tx_bytes;

    /* Send it out the communication driver. */
    ret = elab_serial_write(pch->serial, pch->tx_buff, pch->tx_buff_byte_count);
    elab_assert(ret == pch->tx_buff_byte_count);
}
#endif

/**
 * @brief  A byte has been received from a serial port.  We just store it in 
 *         the buffer for processing when a complete packet has been received.
 * @param  pch      Modbus channel handle.
 * @param  rx_byte  Is the byte received.
 * @retval None.
 */
#if (MODBUS_CFG_RTU_EN != 0)
void mb_rtu_rx_byte(mb_channel_t *pch, uint8_t rx_byte)
{
    /* No, add received byte to buffer. */
    if (pch->rx_buff_byte_count < MODBUS_CFG_BUF_SIZE)
    {
        /* Increment the number of bytes received. */
        pch->rx_count ++;
        *pch->p_rx_buff++ = rx_byte;
        /* Increment byte counter to see if we have Rx activity. */
        pch->rx_buff_byte_count++;
    }
}
#endif

/**
 * @brief  Parses a Modbus RTU packet and processes the request if the packet 
 *         is valid.
 * @param  pch      Modbus channel handle.
 * @retval None.
 */
#if (MODBUS_CFG_RTU_EN != 0)
bool mb_rtu_rx(mb_channel_t *pch)
{
    /* Is the message long enough? */
    if (pch->rx_buff_byte_count >= MODBUS_RTU_MIN_MSG_SIZE &&
        pch->rx_buff_byte_count <= MODBUS_CFG_BUF_SIZE)
    {
        memcpy(pch->rx_frame_data, pch->rx_buff, (pch->rx_buff_byte_count - 2));
        pch->rx_frame_ndata_bytes = pch->rx_buff_byte_count - 4;
        /* Transfer the CRC over.  It's LSB first, then MSB. */
        pch->rx_frame_crc  = (uint16_t)pch->rx_buff[pch->rx_buff_byte_count - 2];
        pch->rx_frame_crc += (uint16_t)pch->rx_buff[pch->rx_buff_byte_count - 1] << 8;

        return (true);
    }

    return (false);
}
#endif

/**
 * @brief  A MODBUS message is formed into a buffer and sent to the appropriate 
 *         communication port. The actual reply is taken from the given MODBUS 
 *         Frame.
 * @param  pch      Modbus channel handle.
 * @retval None.
 */
#if (MODBUS_CFG_RTU_EN != 0)
void mb_rtu_tx(mb_channel_t *pch)
{
    elab_err_t ret = ELAB_OK;

    /* Include the actual data in the buffer. */
    pch->tx_buff_byte_count = (uint8_t)pch->tx_frame_ndata_bytes + 2;
    memcpy(pch->tx_buff, pch->tx_frame_data, pch->tx_buff_byte_count);
    /* Save the calculated CRC in the channel. */
    pch->tx_frame_crc = mb_rtu_tx_calc_crc(pch);
    /* Add in the CRC checksum.  Low byte first! */
    pch->tx_buff[pch->tx_buff_byte_count ++] = (uint8_t)(pch->tx_frame_crc & 0x00FF);
    pch->tx_buff[pch->tx_buff_byte_count ++] = (uint8_t)(pch->tx_frame_crc >> 8);

    /* Send it out the communication driver. */
    ret = elab_serial_write(pch->serial, pch->tx_buff, pch->tx_buff_byte_count);
    elab_assert(ret == pch->tx_buff_byte_count);

#if 0
    printf("Tx %u.\n", pch->tx_buff_byte_count);
    for (uint32_t i = 0; i < pch->tx_buff_byte_count; i ++)
    {
        printf("0x%02x ", pch->tx_buff[i]);
        if ((i + 1) % 16 == 0)
        {
            printf("\n");
        }
    }
    printf("\n");
#endif
}
#endif

/**
 * @brief  This function waits for a response from a slave.
 * @param  pch     specifies the Modbus channel data structure to wait on.
 * @retval Error code.
 */
#if (MODBUS_CFG_MASTER_EN != 0)
uint16_t mbm_rx_blocking(mb_channel_t *pch)
{
    elab_err_t ret = ELAB_OK;
    uint16_t error_id = MODBUS_ERR_NONE;

    ret = elab_serial_read(pch->serial,
                            pch->rx_buff, pch->size_expect, pch->rx_timeout);
    if (ret == ELAB_ERR_TIMEOUT || (ret >= 0 && ret < pch->size_expect))
    {
        error_id = MODBUS_ERR_TIMED_OUT;
    }
    else if (ret < ELAB_OK)
    {
        error_id = MODBUS_ERR_RX;
    }
    else
    {
        pch->rx_buff_byte_count = ret;
    }

    return error_id;
}
#endif

/* ----------------------------- end of file -------------------------------- */
