/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include "mb_bsp.h"
#include "mb.h"

/* public variables --------------------------------------------------------- */
mb_dev_info_t mb_dev_info[MODBUS_CFG_MAX_CH] =
{
    { MB_DEV_TYPE_RS485, "uart1", "P1.1", NULL, true, false },
    { MB_DEV_TYPE_RS485, "uart2", "P1.2", NULL, true, false },
};

/* ----------------------------- end of file -------------------------------- */
