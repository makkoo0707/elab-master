
/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* Includes ------------------------------------------------------------------*/
#include "modbus.h"

/* Private config ------------------------------------------------------------*/
/* CRC-16 Generation Polynomial value. */
#define MODBUS_CRC16_POLY                      0xA001

/*
********************************************************************************
*                            mb_ascii_bin_to_hex()
*
* Description : Converts a byte into two ASCII characters into the given buffer.
*
* Argument(s) : value      The byte of data to be converted.
*               pbuf       A pointer to the buffer to store the ASCII chars.
*
* Return(s)   : The buffer pointer which has been updated to point to the next 
*               char in the buffer.
*
* Caller(s)   : mb_ascii_tx().
*
* Note(s)     : (1) The function ONLY converts the byte to ASCII and DOES NOT 
*                   null terminate the string.
********************************************************************************
*/

#if (MODBUS_CFG_ASCII_EN != 0)
uint8_t *mb_ascii_bin_to_hex (uint8_t  value, uint8_t *pbuf)
{
    uint8_t nibble = (value >> 4) & 0x0F;           /* Upper Nibble */
    if (nibble <= 9)
    {
        *pbuf ++ = (uint8_t)(nibble + '0');
    }
    else
    {
        *pbuf ++ = (uint8_t)(nibble - 10 + 'A');
    }

    nibble = value & 0x0F;                          /* Lower Nibble */
    if (nibble <= 9)
    {
        *pbuf ++ = (uint8_t)(nibble + '0');
    }
    else
    {
        *pbuf ++ = (uint8_t)(nibble - 10 + 'A');
    }
    return (pbuf);
}
#endif

 
/*
********************************************************************************
*                      mb_ascii_hex_to_bin()
*
* Description : Converts the first two ASCII hex characters in the buffer into one byte.
*
* Argument(s) : phex     Pointer to the buffer that contains the two ascii chars.
*
* Return(s)   : value of the two ASCII HEX digits pointed to by 'phex'.
*
* Caller(s)   : mb_ascii_rx_byte(),
*               mb_ascii_rx(),
*               mb_ascii_rx_calc_lrc(),
*               mb_ascii_tx_calc_lrc().
*
* Note(s)     : none.
********************************************************************************
*/

#if (MODBUS_CFG_ASCII_EN != 0)
uint8_t mb_ascii_hex_to_bin (uint8_t *phex)
{
    uint8_t value;
    uint8_t high;
    uint8_t low;

    high = *phex;                                   /* Get upper nibble. */
    phex++;
    low  = *phex;                                   /* Get lower nibble. */
    if (high <= '9')                                /* Upper Nibble. */
    {
        value  = (uint8_t)(high - '0');
    }
    else if (high <= 'F')
    {
        value  = (uint8_t)(high - 'A' + 10);
    }
    else
    {
        value  = (uint8_t)(high - 'a' + 10);
    }
    value <<= 4;

    if (low <= '9')                                 /* Lower Nibble. */
    {
        value += (uint8_t)(low - '0');
    }
    else if (low <= 'F')
    {
        value += (uint8_t)(low - 'A' + 10);
    }
    else
    {
        value += (uint8_t)(low - 'a' + 10);
    }
    return (value);
}
#endif

 
/*
********************************************************************************
*                          mb_ascii_rx_calc_lrc()
*
* Description : The function calculates an 8-bit Longitudinal Redundancy Check on a MODBUS_FRAME
*               structure.
*
* Argument(s) : none.
*
* Return(s)   : The calculated LRC value.
*
* Caller(s)   : mbs_ascii_task().
*
* Note(s)     : (1) The LRC is calculated on the ADDR, FC and Data fields, not the ':', CR/LF and LRC
*                   placed in the message by the sender.  We thus need to subtract 5 'ASCII' characters
*                   from the received message to exclude these.
********************************************************************************
*/

#if (MODBUS_CFG_ASCII_EN != 0)
uint8_t  mb_ascii_rx_calc_lrc (mb_channel_t  *pch)
{
    uint8_t   lrc;
    uint16_t   len;
    uint8_t  *pblock;


    len    = (pch->rx_buff_byte_count - 5) / 2 ;        /* LRC to include Addr + FC + Data                    */
    pblock = (uint8_t *)&pch->rx_buff[1];
    lrc    = 0;
    while (len-- > 0) {                          /* For each byte of data in the data block...         */
        lrc    += mb_ascii_hex_to_bin(pblock);     /* Add the data byte to LRC, increment data pointer.  */
        pblock += 2;
    }

    lrc = ~lrc + 1;                              /* Two complement the binary sum                      */
    return (lrc);                                /* Return LRC for all data in block.                  */
}
#endif

 
/*
********************************************************************************
*                           mb_ascii_tx_calc_lrc()
*
* Description : The function calculates an 8-bit Longitudinal Redundancy Check on a MODBUS_FRAME
*               structure.
*
* Argument(s) : none.
*
* Return(s)   : The calculated LRC value.
*
* Caller(s)   : mb_ascii_tx().
*
* Note(s)     : (1) The LRC is calculated on the ADDR, FC and Data fields, not the ':' which was inserted
*                   in the tx_buff[].  Thus we subtract 1 ASCII character from the LRC.
*
*               (2) The LRC and CR/LF bytes are not YET in the .rx_buff[].
********************************************************************************
*/

#if (MODBUS_CFG_ASCII_EN != 0)
uint8_t  mb_ascii_tx_calc_lrc (mb_channel_t  *pch, uint16_t tx_bytes)
{
    uint8_t     lrc;
    uint16_t     len;
    uint8_t    *pblock;


    len    = (tx_bytes - 1) / 2;                 /* LRC to include Addr + FC + Data (exclude ':')      */
    pblock = (uint8_t *)&pch->tx_buff[1];
    lrc    = 0;
    while (len-- > 0) {                          /* For each byte of data in the data block...         */
        lrc    += mb_ascii_hex_to_bin(pblock);     /* Add the data byte to LRC, increment data pointer.  */
        pblock += 2;
    }
    lrc = ~lrc + 1;                              /* Two complement the binary sum                      */
    return (lrc);                                /* Return LRC for all data in block.                  */
}
#endif

 
/*
********************************************************************************
*                           mb_rtu_rx_calc_crc()
*
* Description : The polynomial is a CRC-16 found for 'MBS_rx_frame_ndata_bytes' number of characters
*               starting at 'MBS_RxFrameAddr'.
*
* Argument(s) : none.
*
* Return(s)   : An unsigned 16-bit value representing the CRC-16 of the data.
*
* Caller(s)   : mbs_rtu_task().
*
* Note(s)     : none.
********************************************************************************
*/

#if (MODBUS_CFG_RTU_EN != 0)
uint16_t mb_rtu_rx_calc_crc(mb_channel_t *pch)
{
    /* Starting address of where the CRC data starts. */
    uint8_t *pblock = (uint8_t *)&pch->rx_frame_data[0];
    /* Include the address and function code in the CRC. */
    uint16_t length = pch->rx_frame_ndata_bytes + 2;
    /* Initialize CRC to all ones. */
    uint16_t crc = 0xFFFF;
    /* Account for each byte of data. */
    while (length > 0)
    {
        length --;
        crc ^= (uint16_t)*pblock++;
        uint8_t shiftctr = 8;
        do
        {
            /* Determine if the shift out of rightmost bit is 1. */
            bool flag = (crc & 0x0001) ? true : false;
            /* Shift CRC to the right one bit. */
            crc >>= 1;
            /* If (bit shifted out of rightmost bit was a 1). */
            if (flag == true)
            {
                /* Exclusive OR the CRC with the generating polynomial. */
                crc ^= MODBUS_CRC16_POLY;
            }
            shiftctr --;
        } while (shiftctr > 0);
    }
    pch->rx_frame_crc_calc = crc;

    return (crc);
}
#endif

 
/*
********************************************************************************
*                          mb_rtu_tx_calc_crc()
*
* Description : The polynomial is a CRC-16 found for 'MBS_tx_frame_ndata_bytes' number of characters
*               starting at 'MBS_TxFrameAddr'.
*
* Argument(s) : none.
*
* Return(s)   : An unsigned 16-bit value representing the CRC-16 of the data.
*
* Caller(s)   : mb_rtu_tx().
*
* Note*(s)    : none.
********************************************************************************
*/

#if (MODBUS_CFG_RTU_EN != 0)
uint16_t mb_rtu_tx_calc_crc(mb_channel_t *pch)
{
    /* Starting address of where the CRC data starts. */
    uint8_t *pblock = (uint8_t *)&pch->tx_frame_data[0];
    /* Include the address and function code in the CRC. */
    uint16_t length = pch->tx_frame_ndata_bytes + 2;
    /* Initialize CRC to all ones. */
    uint16_t crc = 0xFFFF;
    /* Account for each byte of data. */
    while (length > 0)
    {
        length --;
        crc ^= (uint16_t)*pblock++;
        uint8_t shiftctr = 8;
        do
        {
            /* Determine if the shift out of rightmost bit is 1. */
            bool flag = (crc & 0x0001) ? true : false; 
            /* Shift CRC to the right one bit. */
            crc >>= 1;
            /* If (bit shifted out of rightmost bit was a 1). */
            if (flag == true)
            {
                /* Exclusive OR the CRC with the generating polynomial. */
                crc ^= MODBUS_CRC16_POLY;
            }
            shiftctr--;
        } while (shiftctr > 0);
    }

    return (crc);
}
#endif

/* ----------------------------- end of file -------------------------------- */
