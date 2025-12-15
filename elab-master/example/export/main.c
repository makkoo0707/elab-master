/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* Includes ------------------------------------------------------------------*/
#include "elab/common/elab_common.h"
#include "elab/common/elab_export.h"

/* public functions --------------------------------------------------------- */
/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    elab_run();
}

/* Private functions ---------------------------------------------------------*/
#include <stdio.h>

static void _export_test_bsp(void)
{
    printf("export bsp testing.\n");
}
INIT_EXPORT(_export_test_bsp, EXPORT_LEVEL_BSP);
EXIT_EXPORT(_export_test_bsp, EXPORT_LEVEL_BSP);

static void _export_test_io_driver(void)
{
    printf("export io driver testing.\n");
}
INIT_EXPORT(_export_test_io_driver, EXPORT_DRVIVER);
EXIT_EXPORT(_export_test_io_driver, EXPORT_DRVIVER);

static void _export_test_component(void)
{
    printf("export component testing.\n");
}
INIT_EXPORT(_export_test_component, EXPORT_MIDWARE);
EXIT_EXPORT(_export_test_component, EXPORT_MIDWARE);

static void _export_test_device(void)
{
    printf("export device testing.\n");
}
INIT_EXPORT(_export_test_device, EXPORT_DEVICE);
EXIT_EXPORT(_export_test_device, EXPORT_DEVICE);

static void _export_test_app(void)
{
    printf("export app testing.\n");
}
INIT_EXPORT(_export_test_app, EXPORT_APP);
EXIT_EXPORT(_export_test_app, EXPORT_APP);

static void _export_test_poll(void)
{
    printf("export poll testing.\n");
}
POLL_EXPORT(_export_test_poll, 1000);

/* ----------------------------- end of file -------------------------------- */
