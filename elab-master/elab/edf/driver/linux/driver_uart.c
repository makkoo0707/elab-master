/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "driver_uart.h"
#include "../../../common/elab_log.h"
#include "../../normal/elab_serial.h"
#include "../../normal/elab_pin.h"
#include "../../../common/elab_assert.h"

ELAB_TAG("Bsp-Serial-Linux");

/* private function prototype ----------------------------------------------- */
static elab_err_t _enable(elab_serial_t *serial, bool status);
static int32_t _read(elab_serial_t *serial, void *buffer, uint32_t size);
static int32_t _write(elab_serial_t *serial, const void *buffer, uint32_t size);
static void _set_tx(elab_serial_t *serial, bool status);
static elab_err_t _config(elab_serial_t *serial, elab_serial_config_t *config);
static bool _checkout_name_valid(char *name);
static uint32_t _get_baudrate_from_device(char *name);

/* private variables -------------------------------------------------------- */
static elab_serial_ops_t _serial_ops =
{
    .enable = _enable,
    .read = _read,
    .write = _write,
    .set_tx = _set_tx,
    .config = _config,
};

/* public function ---------------------------------------------------------- */
void driver_serial_init(driver_uart_t *me,
                        const char *dev_name,
                        const char *drv_name)
{
    elab_assert(_checkout_name_valid((char *)drv_name));

    me->baudrate = _get_baudrate_from_device((char *)drv_name);
    me->dev_name = dev_name;
    me->drv_name = drv_name;
    me->serial_fd = INT32_MIN;

    elab_serial_attr_t _attr = (elab_serial_attr_t)ELAB_SERIAL_ATTR_DEFAULT;
    _attr.baud_rate = me->baudrate;
    elab_serial_register(&me->device, me->dev_name, &_serial_ops, &_attr, (void *)me);
    _enable(&me->device, true);
    _config(&me->device, (elab_serial_config_t *)&_attr);
    _enable(&me->device, false);
}

/* private functions -------------------------------------------------------- */
static elab_err_t _enable(elab_serial_t *serial, bool status)
{
    driver_uart_t *driver = (driver_uart_t *)serial->super.user_data;
    elab_err_t ret = ELAB_OK;

    if (status)
    {
        if (driver->serial_fd == INT32_MIN)
        {
            /* Open the serial port. */
            driver->serial_fd = open(driver->serial_name, O_RDWR | O_NOCTTY);
            if (driver->serial_fd != INT32_MIN && driver->serial_fd < 0)
            {
                elog_error("Serial port %s opening fails.", driver->serial_name);
                goto exit;
            }
        }
    }
    else
    {
        assert(driver->serial_fd != INT32_MIN);

        int32_t ret = close(driver->serial_fd);
        if (ret != 0)
        {
            elog_error("Serial port %s closing fails.", driver->serial_name);
            goto exit;
        }

        driver->serial_fd = INT32_MIN;
    }

exit:
    return ret;
}

static int32_t _read(elab_serial_t *serial, void *buffer, uint32_t size)
{
    driver_uart_t *driver = (driver_uart_t *)serial->super.user_data;

    int32_t ret = size;
    int32_t ret_r = read(driver->serial_fd, buffer, size);
    if (ret_r < 0)
    {
        ret = ELAB_ERROR;
    }
    else
    {
        ret = ret_r;
    }

    return ret;
}

static int32_t _write(elab_serial_t *serial, const void *buffer, uint32_t size)
{
    driver_uart_t *driver = (driver_uart_t *)serial->super.user_data;
    int32_t ret = size;
    int32_t ret_w = write(driver->serial_fd, buffer, size);
    /* Wait the data transmitted completely. */
#if defined(__x86_64__) || defined(__i386__)
    tcdrain(driver->serial_fd);
#endif
    if (ret_w < 0)
    {
        ret = ELAB_ERROR;
    }
    else
    {
        ret = ret_w;
    }

    return ret;
}

static void _set_tx(elab_serial_t *serial, bool status)
{
    (void)serial;
    (void)status;
}

static elab_err_t _config(elab_serial_t *serial, elab_serial_config_t *config)
{
    driver_uart_t *driver = (driver_uart_t *)serial->super.user_data;
    elab_err_t ret = ELAB_OK;
    int ret_set = 0;

    /* Test if the serial port is a terminal-device. */
    if (0 == isatty(STDIN_FILENO))
    {
        elog_error("isatty(STDIN_FILENO) fails.");
        ret = ELAB_ERROR;
        goto exit;
    }

    /* Config the serial port. */
    static const uint32_t speed_arr[] =
    {
        B1152000, B460800, B115200, B19200, B9600, B4800, B2400, B1200, B300
    };
    static const uint32_t name_arr[] =
    {
        1152000, 460800, 115200, 19200, 9600, 4800, 2400, 1200, 300
    };

    struct termios options;
    ret_set = tcgetattr(driver->serial_fd, &options);
    if (ret_set != 0)
    {
        elog_error("tcgetattr(serial_fd, &options) fails. ret_get: %d.", ret_set);
        ret = ELAB_ERROR;
        goto exit;
    }

    // set the input and output baudrate of the serial port
    bool baudrate_existent = false;
    for (uint8_t i = 0;  i < sizeof(speed_arr) / sizeof(int); i ++)
    {
        if (config->baud_rate == name_arr[i])
        {
            cfsetispeed(&options, speed_arr[i]);
            cfsetospeed(&options, speed_arr[i]);
            baudrate_existent = true;
        }
    }
    assert(baudrate_existent);

    /* control mode */
    options.c_cflag |= CLOCAL;
    options.c_cflag |= CREAD;
    /* no data stream control */
    options.c_cflag &= ~CRTSCTS;
    /* data bits */
    options.c_cflag &= ~CSIZE;
    options.c_iflag &= ~(ICRNL | IXON);
    switch (config->data_bits)
    {
        case ELAB_SERIAL_DATA_BITS_5: options.c_cflag |= CS5; break;
        case ELAB_SERIAL_DATA_BITS_6: options.c_cflag |= CS6; break;
        case ELAB_SERIAL_DATA_BITS_7: options.c_cflag |= CS7; break;
        case ELAB_SERIAL_DATA_BITS_8: options.c_cflag |= CS8; break;
        default:
            assert(false);
    }

    /* parity bits */
    switch (config->parity)
    {
        case ELAB_SERIAL_PARITY_NONE:
            options.c_cflag &= ~PARENB;
            options.c_iflag &= ~INPCK;
            break;

        case ELAB_SERIAL_PARITY_ODD:
            options.c_cflag |= (PARODD | PARENB);
            options.c_iflag |= INPCK;
            break;

        case ELAB_SERIAL_PARITY_EVEN:
            options.c_cflag |= PARENB;
            options.c_cflag &= ~PARODD;
            options.c_iflag |= INPCK;
            break;

        default:
            assert(false);
    }
    /* stop bits */
    switch (config->stop_bits)
    {
        case ELAB_SERIAL_STOP_BITS_1:
            options.c_cflag &= ~CSTOPB;
            break;

        case ELAB_SERIAL_STOP_BITS_2:
            options.c_cflag |= CSTOPB;
            break;

        default:
            assert(false);
    }

    options.c_oflag &= ~OPOST;
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_cc[VTIME] = 1;
    options.c_cc[VMIN] = 1;

    tcflush(driver->serial_fd, TCIFLUSH);

    /* config */
    ret_set = tcsetattr(driver->serial_fd, TCSANOW, &options);
    if (ret_set != 0)
    {
        elog_error("tcsetattr(serial_fd, &options) fails. ret_set: %d.", ret_set);
        ret = ELAB_ERROR;
        goto exit;
    }

exit:
    return ret;
}

static uint32_t _get_baudrate_from_device(char *name)
{
    char *ch_baudrate = strstr(name, ".");
    ch_baudrate ++;
    return (uint32_t)atoi(ch_baudrate);
}

static bool _checkout_name_valid(char *name)
{
    static const uint32_t name_arr[] =
    {
        1152000, 460800, 115200, 19200, 9600, 4800, 2400, 1200, 300
    };
    char *ch_baudrate = strstr(name, ".");
    ch_baudrate ++;
    int32_t baudrate = atoi(ch_baudrate);
    bool existent = false;
    for (uint32_t i = 0; i < sizeof(name_arr) / sizeof(name_arr); i ++)
    {
        if (name_arr[i] == baudrate)
        {
            existent = true;
            break;
        }
    }

    return existent;
}

/* ----------------------------- end of file -------------------------------- */
