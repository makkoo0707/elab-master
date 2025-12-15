/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef DRIVER_UART_H
#define DRIVER_UART_H

/* include ------------------------------------------------------------------ */
#include "../../elab_device.h"
#include "../../normal/elab_serial.h"

/* private typedef ---------------------------------------------------------- */
typedef struct driver_uart
{
    elab_serial_t device;
    
    const char *dev_name;
    uint32_t baudrate;
    int32_t serial_fd;
    const char *drv_name;
    char serial_name[ELAB_NAME_SIZE];
} driver_uart_t;

/* public function ---------------------------------------------------------- */
void driver_serial_init(driver_uart_t *me,
                        const char *dev_name,
                        const char *drv_name);

#endif /* DRIVER_UART_H */

/* ----------------------------- end of file -------------------------------- */
