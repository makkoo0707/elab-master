/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include "mb_def.h"
#include "mb.h"
#include "mb_bsp.h"
#include "elab_device.h"
#include "elab_serial.h"
#include "elab_pin.h"
#include "cmsis_os.h"
#include "elab_def.h"
#include "elab_rs485.h"
#include "elab_assert.h"
#include "elab_common.h"
#include "elab_log.h"

ELAB_TAG("ModbusBsp");

/* Private define ------------------------------------------------------------*/
#define MODBUS_SERAIL_RX_BUFSZ                  (256)
#define MODBUS_SERAIL_TX_BUFSZ                  (256)

/* Private typedef -----------------------------------------------------------*/
typedef struct mb_dev_data
{
    elab_mb_channel_t *pch;
    void *dev_handle;
    elab_device_t *serial;
    osThreadId_t thread_rx;
} mb_dev_data_t;

/* Exported variables --------------------------------------------------------*/
ELAB_WEAK mb_dev_info_t mb_dev_info[MODBUS_CFG_MAX_CH];

/* Private variables ---------------------------------------------------------*/
static mb_dev_data_t mb_dev_data[MODBUS_CFG_MAX_CH] =
{
    { NULL, NULL, NULL, NULL },
};

static const osThreadAttr_t thread_rx_attr =
{
    .name = "ModbusSerial",
    .attr_bits = osThreadDetached, 
    .priority = osPriorityRealtime,
    .stack_size = 2048,
};

/* Private function prototypes -----------------------------------------------*/
static void thread_func_mb_rx(void *parameter);
static elab_device_t *mb_port_get_serial(elab_mb_channel_t *pch);
static uint32_t mb_port_write(elab_mb_channel_t *pch,
                                const void *pbuf, uint32_t size);
static uint32_t mb_port_read(elab_mb_channel_t *pch, void *pbuf, uint32_t size);

/* Exported functions --------------------------------------------------------*/
/**
 * @brief  This function is called to terminate Modbus communications. All Modbus
 *         channels are close.
 * @retval None.
 */
void MB_CommExit(void)
{
    uint8_t ch;
    elab_mb_channel_t *pch;

    for (ch = 0; ch < MODBUS_CFG_MAX_CH; ch++)
    {
        pch = &modbus_channel_table[ch];
        MB_CommTxIntDis(pch);
        MB_CommRxIntDis(pch);
    }
}

/**
 * @brief  This function initializes the serial port to the desired baud rate
 *         and the UART is configured for N, 8, 1 (No parity, 8 bits, 1 stop).
 * @param  pch        is a pointer to the Modbus channel
 * @param  port_nbr   is the desired serial port number. This argument allows
 *                    you to assign a specific serial port to a specific Modbus
 *                    channel.
 * @param  baud       is the desired baud rate for the serial port.
 * @param  parity     is the desired parity and can be either:
 *                          MODBUS_PARITY_NONE
 *                          MODBUS_PARITY_ODD
 *                          MODBUS_PARITY_EVEN
 * @param  bits       specifies the number of bit and can be either 7 or 8.
 * @param  stops      specifies the number of stop bits and can either be 1 or 2
 * @retval None.
 */
void MB_CommPortCfg(elab_mb_channel_t *pch,
                    uint8_t port_nbr,
                    uint32_t baud,
                    uint8_t bits,
                    uint8_t parity,
                    uint8_t stops)
{
    (void)port_nbr;

    elab_serial_attr_t uart_attr =
    {
        baud,
        (uint32_t)MODBUS_SERAIL_RX_BUFSZ,
        (uint32_t)MODBUS_SERAIL_TX_BUFSZ,
        bits,
        (stops - 1),                 /* STOP_BITS_1 is 0. */
        parity,
        ELAB_SERIAL_MODE_FULL_DUPLEX,
        0,
    };
    
    elab_err_t ret = ELAB_OK;

    /* Find the serial device from its name. */
    mb_dev_data[pch->PortNbr].serial = mb_port_get_serial(pch);
    if (NULL == mb_dev_data[pch->PortNbr].serial)
    {
        ret = ELAB_ERROR;
        elog_error("Modbus %u finding serial device %s fails.",
                        pch->PortNbr,
                        mb_dev_info[pch->PortNbr].serial_name);
    }

    /* Close the serial device. */
    if (MB_DEV_TYPE_UART != mb_dev_info[pch->PortNbr].type)
    {
        ret = elab_device_close(mb_dev_data[pch->PortNbr].serial);
        if (ELAB_OK != ret)
        {
            elog_error("Modbus serial port closing fails. Error ID: %d.",
                        (int32_t)ret);
            goto exit;
        }
    }

    /* Serial port configuration. */
    elab_serial_set_attr(ELAB_SERAIL_CAST(mb_dev_data[pch->PortNbr].serial),
                            &uart_attr);

    /* Open the serial device. */
    ret = elab_device_open(mb_dev_data[pch->PortNbr].serial);
    if (ELAB_OK != ret)
    {
        elog_error("Modbus serial port opening fails. Error ID: %d.",
                     (int32_t)ret);
        goto exit;
    }

    /* Start a thread for receiving data from the serial port. */
    mb_dev_data[pch->PortNbr].thread_rx = 
        osThreadNew(thread_func_mb_rx, pch, &thread_rx_attr);
    if (NULL == mb_dev_data[pch->PortNbr].thread_rx)
    {
        ret = ELAB_ERROR;
        elog_error("Modbus thread for receiving data initialization fails.");
        goto exit;
    }

exit:
    elab_assert(ELAB_OK == ret);
}

/**
 * @brief  This function disables Rx interrupts.
 * @param  pch        is a pointer to the Modbus channel.
 * @retval None.
 */
void MB_CommRxIntDis(elab_mb_channel_t *pch)
{
    (void)pch;
}

/**
 * @brief  This function enables Rx interrupts.
 * @param  pch        is a pointer to the Modbus channel.
 * @retval None.
 */
void MB_CommRxIntEn(elab_mb_channel_t *pch)
{
    (void)pch;
}

/**
 * @brief  This function is called to obtain the next byte to send from the
 *         transmit buffer. When all bytes in the reply have been sent, transmit
 *         interrupts are disabled and the receiver is enabled to accept the
 *         next Modbus request.
 * @param  pch  is a pointer to the Modbus channel.
 * @param  c    is the byte to send to the serial port
 * @retval None.
 */
void MB_CommTx1(elab_mb_channel_t *pch, uint8_t c)
{
    /* Simulate the MCU ISR. Find back the first sent byte. */
    pch->TxBufByteCtr++;
    pch->TxBufPtr--;
    
    /* Send data into the serial port. */
    mb_port_write(pch, pch->TxBufPtr, pch->TxBufByteCtr);
    pch->TxCtr = pch->TxBufByteCtr;
    pch->TxBufByteCtr = 0;

    /* If there is nothing to do end transmission. */
    MB_TxByte(pch);
}

/**
 * @brief  This function disables Tx interrupts.
 * @param  pch  is a pointer to the Modbus channel.
 * @retval None.
 */
void MB_CommTxIntDis(elab_mb_channel_t *pch)
{
    (void)pch;
}

/**
 * @brief  This function enables Tx interrupts.
 * @param  pch  is a pointer to the Modbus channel.
 * @retval None.
 */
void MB_CommTxIntEn(elab_mb_channel_t *pch)
{
    (void)pch;
}

/* Private functions ---------------------------------------------------------*/
/**
 * @brief  Read data from the serial port in blocking mode.
 * @param  argument  Thread argument.
 * @retval None.
 */
static void thread_func_mb_rx(void *parameter)
{
    elab_mb_channel_t *pch = (elab_mb_channel_t *)parameter;
    uint8_t c;

    while (1)
    {
        c = 0;
        uint32_t ret = mb_port_read(pch, &c, 1);
        if (ret != 0)
        {
            MB_RxByte(pch, c);
        }
    }
}

/**
 * @brief  Get serial device handle from modbus channel handle.
 * @param  pch      Modbus channel handle.
 * @retval serial handle.
 */
static elab_device_t *mb_port_get_serial(elab_mb_channel_t *pch)
{
    elab_err_t ret = ELAB_OK;
    uint8_t nbr = pch->PortNbr;
    elab_device_t *serial = NULL;

    mb_dev_data[nbr].pch = pch;

    /* Initialize the physical ports. */
    if (MB_DEV_TYPE_UART == mb_dev_info[nbr].type)
    {
        /* Find the serial device from its name. */
        serial = elab_device_find(mb_dev_info[nbr].serial_name);
    }
    else if (MB_DEV_TYPE_RS485 == mb_dev_info[nbr].type)
    {
        mb_dev_data[nbr].dev_handle = elab_malloc(sizeof(rs485_t));
        if (NULL == mb_dev_data[nbr].dev_handle)
        {
            elog_error("Modbus %u device memory applying fails.",
                            nbr);
            goto exit;
        }

        elab_err_t ret =
            rs485_init((rs485_t *)mb_dev_data[nbr].dev_handle,
                        mb_dev_info[nbr].serial_name,
                        mb_dev_info[nbr].pin_tx_en,
                        mb_dev_info[nbr].tx_en_high_active,
                        NULL);
        if (ret != ELAB_OK)
        {
            elog_error("Modbus %u rs485 intialization fails.", nbr);
            goto exit;
        }

        serial = rs485_get_serial((rs485_t *)mb_dev_data[nbr].dev_handle);
    }
    else if (MB_DEV_TYPE_RS485 == mb_dev_info[nbr].type)
    {
        mb_dev_data[nbr].dev_handle = elab_malloc(sizeof(rs485_t));
        if (NULL == mb_dev_data[nbr].dev_handle)
        {
            elog_error("Modbus %u device memory applying fails.",
                            nbr);
            goto exit;
        }

        elab_err_t ret =
            rs485_init((rs485_t *)mb_dev_data[nbr].dev_handle,
                        mb_dev_info[nbr].serial_name,
                        mb_dev_info[nbr].pin_tx_en,
                        mb_dev_info[nbr].pin_rx_en,
                        NULL);
        if (ret != ELAB_OK)
        {
            elog_error("Modbus %u rs485 intialization fails.", nbr);
            goto exit;
        }

        serial = rs485_get_serial((rs485_t *)mb_dev_data[nbr].dev_handle);
    }

exit:
    if (NULL == serial)
    {
        ret = ELAB_ERROR;
        elog_error("Modbus %u finding serial device %s fails.",
                        nbr,
                        mb_dev_info[nbr].serial_name);
    }

    elab_assert(ELAB_OK == ret);

    return serial;
}

/**
 * @brief  Modbus serial device data sending function.
 * @param  pch  Modbus channel handle.
 * @param  pbuf The pointer of buffer
 * @param  size Expected write length
 * @retval Actual write length
 */
static uint32_t mb_port_write(elab_mb_channel_t *pch, const void *pbuf, uint32_t size)
{
    uint32_t ret = 0;

    uint8_t nbr = pch->PortNbr;
    if (MB_DEV_TYPE_UART == mb_dev_info[nbr].type ||
        MB_DEV_TYPE_RS422 == mb_dev_info[nbr].type)
    {
        elab_device_t *serial = mb_dev_data[nbr].serial;
        ret = elab_device_write(serial, 0, pbuf, size);
    }
    else
    {
        rs485_t *rs485 = (rs485_t *)mb_dev_data[nbr].dev_handle;
        ret = rs485_write(rs485, pbuf, size);
    }

    return ret;
}

/**
  * @brief  Modbus serial device data reading function.
  * @param  pch Modbus channel handle
  * @param  pbuf The pointer of buffer
  * @param  size Expected read length
  * @retval Auctual read length
  */
static uint32_t mb_port_read(elab_mb_channel_t *pch, void *pbuf, uint32_t size)
{
    uint32_t ret = 0;

    uint8_t nbr = pch->PortNbr;
    if (MB_DEV_TYPE_UART == mb_dev_info[nbr].type ||
        MB_DEV_TYPE_RS422 == mb_dev_info[nbr].type)
    {
        elab_device_t *serial = mb_dev_data[nbr].serial;
        ret = elab_device_read(serial, 0, pbuf, size);
    }
    else
    {
        rs485_t *rs485 = (rs485_t *)mb_dev_data[nbr].dev_handle;
        ret = rs485_read(rs485, pbuf, size);
    }

    return ret;
}

/* ----------------------------- end of file -------------------------------- */
