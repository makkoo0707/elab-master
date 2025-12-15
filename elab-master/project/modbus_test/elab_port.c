/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include <stdlib.h>
#include "elab_common.h"
#include "cmsis_os.h"

/* public functions ----------------------------------------------------------*/
uint32_t elab_time_ms(void)
{
    return osKernelGetTickCount();
}

void *elab_malloc(uint32_t size)
{
    return malloc(size);
}

void elab_free(void *memory)
{
    return free(memory);
}

/* ----------------------------- end of file -------------------------------- */
