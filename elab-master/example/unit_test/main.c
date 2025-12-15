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
    elab_debug_uart_init(115200);
    elab_run();
}

/* ----------------------------- end of file -------------------------------- */
