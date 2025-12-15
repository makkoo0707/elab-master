/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef __ELAB_SPI_H
#define __ELAB_SPI_H

/* includes ----------------------------------------------------------------- */
#include <stdint.h>
#include "../elab_device.h"
#include "../../os/cmsis_os.h"

#ifdef __cplusplus
extern "C" {
#endif

/* public macros -------------------------------------------------------------*/
enum elab_spi_mode
{
    ELAB_SPI_MODE_0 = 0,                        /* CPOL = 0, CPHA = 0 */
    ELAB_SPI_MODE_1,                            /* CPOL = 0, CPHA = 1 */
    ELAB_SPI_MODE_2,                            /* CPOL = 1, CPHA = 0 */
    ELAB_SPI_MODE_3,                            /* CPOL = 1, CPHA = 1 */

    ELAB_SPI_MODE_MAX
};

/* Public types ------------------------------------------------------------- */
/**
 * SPI message structure
 */
typedef struct elab_spi_msg
{
    const void *buff_send;
    void *buff_recv;
    uint32_t size;
} elab_spi_msg_t;

/**
 * SPI configuration structure
 */
typedef struct elab_spi_config
{
    uint8_t mode;
    uint8_t data_width;
    uint32_t max_hz;
} elab_spi_config_t;

/**
 * SPI Virtual BUS, one device must connected to a virtual BUS
 */
struct elab_spi_ops;
typedef struct elab_spi_bus
{
    elab_device_t super;

    uint8_t mode;
    const struct elab_spi_bus_ops *ops;

    osMutexId_t mutex;
    osSemaphoreId_t sem;
    elab_spi_config_t config_owner;
} elab_spi_bus_t;

typedef struct elab_spi_device
{
    elab_device_t super;

    elab_spi_bus_t *bus;
    elab_device_t *pin_cs;
    elab_spi_config_t config;
    void *user_data;
} elab_spi_t;

/**
 * SPI bus operators
 */
typedef struct elab_spi_bus_ops
{
    elab_err_t (* config)(elab_spi_t *const me, elab_spi_config_t *config);
    elab_err_t (* xfer)(elab_spi_t * const me, elab_spi_msg_t *message);
} elab_spi_bus_ops_t;

#define ELAB_SPI_CAST(_dev)                 ((elab_spi_t *)_dev)
#define ELAB_SPI_BUS_CAST(_dev)             ((elab_spi_bus_t *)_dev)

/* Public functions --------------------------------------------------------- */
void elab_spi_bus_register(elab_spi_bus_t *bus,
                            const char *name, const elab_spi_bus_ops_t *ops,
                            void *user_data);
void elab_spi_register(elab_spi_t *device, const char *name,
                            const char *bus_name,
                            const char *pin_name_cs,
                            elab_spi_config_t config);
void elab_spi_bus_xfer_end(elab_spi_bus_t *bus);

/* Before doing anything with the SPI device, you first need to call this
 * function to configure the bus. */
elab_err_t elab_spi_send_recv(elab_device_t *me,
                                const void *buff_send, uint32_t size_send,
                                void *buff_recv, uint32_t size_recv,
                                uint32_t timeout);
elab_err_t elab_spi_send_twice(elab_device_t *me,
                                const void *buff1, uint32_t size1,
                                const void *buff2, uint32_t size2,
                                uint32_t timeout);
elab_err_t elab_spi_xfer(elab_device_t *me,
                            const void *buff_send, void *buff_recv,
                            uint32_t size, uint32_t timeout);
elab_err_t elab_spi_xfer_msg(elab_device_t *me, elab_spi_msg_t *msg, uint32_t num,
                                uint32_t timeout);
elab_err_t elab_spi_recv(elab_device_t *me,
                            void *buff, uint32_t size, uint32_t timeout);
elab_err_t elab_spi_send(elab_device_t *me,
                            const void *buffer, uint32_t size, uint32_t timeout);

#ifdef __cplusplus
}
#endif

#endif /* __ELAB_SPI_H */

/* ----------------------------- end of file -------------------------------- */
