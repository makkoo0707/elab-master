
/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "elab/3rd/Shell/shell.h"
#include "elab/common/elab_log.h"
#include "elab/edf/normal/elab_spi.h"

ELAB_TAG("SpiTest");

#ifdef __cplusplus
extern "C" {
#endif

/* private functions -------------------------------------------------------- */
/**
  * @brief Testing function for base vw function.
  * @retval None
  */
static int32_t test_spi(int32_t argc, char *argv[])
{
    (void)argc;
    (void)argv;

    int32_t ret = 0;
    elab_err_t ret_spi = ELAB_OK;
    static uint8_t buff_tx[32];
    static uint8_t buff_rx[32];
    uint32_t size = 32;

    elab_device_t *spi = elab_device_find("spi");
    if (spi == NULL)
    {
        elog_error("spi device not found.");
        ret = -1;
        goto exit;
    }

    ret_spi = elab_spi_xfer(spi, buff_tx, buff_rx, size, 100);
    if (ret_spi != ELAB_OK)
    {
        elog_error("elab_spi_xfer error %d.", (int32_t)ret_spi);
        ret = -2;
        goto exit;
    }

exit:
    return ret;
}

/**
  * @brief  Test shell command export
  */
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                    test_spi,
                    test_spi,
                    spi testing function);

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
