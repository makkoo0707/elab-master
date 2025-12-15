
/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#if 0
#if defined(__linux__) || defined(_WIN32)
/* include ------------------------------------------------------------------ */
#include <stdbool.h>
#include "../edf/driver/linux/driver_uart.h"
#include "../common/elab_export.h"
#include "../edf/driver/simulator/simu_serial.h"
#include "elab/midware/edb/edb.h"

#define PORT_EXPORT_BUFF_SIZE                       (256)
#define PORT_SERIAL_NAME_SIZE                       (64)
#define PORT_PIN_NAME_SIZE                          (16)

typedef struct driver_uart_data
{
    int32_t serial_fd;
    char name_serial[PORT_SERIAL_NAME_SIZE];
    char name_pin[PORT_PIN_NAME_SIZE];
    elab_serial_t device;
    uint32_t baudrate;
} driver_uart_data_t;

static char buff_line[PORT_EXPORT_BUFF_SIZE];
static const char *rs232_table[] =
{
    "RS232_1", "RS232_2", "RS232_3", "RS232_4", 
};
static const char *rs485_table[] =
{
    "RS485_1", "RS485_2", "RS485_3", "RS485_4", 
};

static driver_uart_data_t rs485_table[];
static driver_uart_data_t rs232_table[];

/* public function ---------------------------------------------------------- */
static void _serial_export(void)
{
    /* Simulated RS232 exporting. */
    simu_serial_new("simu_rs232_1", SIMU_SERIAL_MODE_UART, 115200);
    simu_serial_new("simu_rs232_2", SIMU_SERIAL_MODE_UART, 115200);
    simu_serial_new("simu_rs232_3", SIMU_SERIAL_MODE_UART, 115200);
    simu_serial_new("simu_rs232_4", SIMU_SERIAL_MODE_UART, 115200);
    simu_serial_new("simu_rs232_5", SIMU_SERIAL_MODE_UART, 115200);
    simu_serial_new("simu_rs232_6", SIMU_SERIAL_MODE_UART, 115200);
    simu_serial_new("simu_rs232_7", SIMU_SERIAL_MODE_UART, 115200);
    simu_serial_new("simu_rs232_8", SIMU_SERIAL_MODE_UART, 115200);

    /* Simulated RS485 exporting. */
    simu_serial_new("simu_rs485_1", SIMU_SERIAL_MODE_485_S, 115200);
    simu_serial_new("simu_rs485_2", SIMU_SERIAL_MODE_485_S, 115200);
    simu_serial_new("simu_rs485_3", SIMU_SERIAL_MODE_485_S, 115200);
    simu_serial_new("simu_rs485_4", SIMU_SERIAL_MODE_485_S, 115200);
    simu_serial_new("simu_rs485_5", SIMU_SERIAL_MODE_485_S, 115200);
    simu_serial_new("simu_rs485_6", SIMU_SERIAL_MODE_485_S, 115200);
    simu_serial_new("simu_rs485_7", SIMU_SERIAL_MODE_485_S, 115200);
    simu_serial_new("simu_rs485_8", SIMU_SERIAL_MODE_485_S, 115200);

    /* RS232 ports exporting. */
    memset(buff_line, 0, PORT_EXPORT_BUFF_SIZE);
    char *name = NULL;
    for (uint32_t i = 0; i < sizeof(rs232_table) / sizeof(char *); i ++)
    {
        name = rs232_table[i];
        if (edb_str_cmp("Port", name, "simu,master"))
        {
            simu_serial_new(name, SIMU_SERIAL_MODE_485_M, 115200);
        }
        else if (edb_str_cmp("Port", name, "simu,slave"))
        {
            simu_serial_new(name, SIMU_SERIAL_MODE_485_S, 115200);
        }
        else
        {

        }
    }

}

/**
  * @brief  Export pin device board level init and exit function
  */
INIT_EXPORT(_serial_export, 0);

#endif

/* ----------------------------- end of file -------------------------------- */

#endif