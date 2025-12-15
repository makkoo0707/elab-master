
/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include "elab/common/elab_export.h"
#include "elab/edf/normal/elab_spi.h"
#include "export.h"

/* private variables -------------------------------------------------------- */
static elab_spi_t spi_oled;
static elab_spi_t spi_flash;

/* public functions --------------------------------------------------------- */
static void driver_spi_export(void)
{
    elab_spi_config_t config =
    {
        .mode = ELAB_SPI_MODE_0,
        .data_width = 8,
        .max_hz = 1000000,
    };
    elab_spi_register(&spi_oled, "spi_oled", "SPI1", "pin_oled_cs", config);
    elab_spi_register(&spi_flash, "spi_flash", "SPI1", "pin_flash_cs", config);
}
INIT_EXPORT(driver_spi_export, EXPORT_LEVEL_SPI);

/* ----------------------------- end of file -------------------------------- */
