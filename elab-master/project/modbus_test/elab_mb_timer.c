
/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include <stdlib.h>
#include "mb.h"
#include "cmsis_os.h"
#include "elab_common.h"
#include "elab_assert.h"

ELAB_TAG("eLabMbTimer");

/* private variables -------------------------------------------------------- */
static const osThreadAttr_t thread_timer_rtu =
{
    .name = "ModbusSerial",
    .attr_bits = osThreadDetached, 
    .priority = osPriorityRealtime,
    .stack_size = 2048,
};

/* public function ---------------------------------------------------------- */
void elab_mb_rtu_timer_init(void)
{
    osThreadId_t thread =
    osThreadNew((osThreadFunc_t)MB_RTU_TmrISR_Handler, NULL, &thread_timer_rtu);
    elab_assert(thread != NULL);
}

void elab_mb_rtu_timer_exit(void)
{

}

void MB_RTU_TmrISR_Handler(void)
{
    uint32_t time = elab_time_ms();

    while (1)
    {
        uint32_t time_current = elab_time_ms();
        uint32_t diff = 0;
        if (time_current >= time)
        {
            diff = time_current - time;
        }
        else
        {
            diff = UINT32_MAX - time_current + time + 1;
        }
        
        for (uint32_t i = 0; i < diff; i ++)
        {
            MB_RTU_TmrUpdate();
        }

        time = time_current;
    }
}

/* ----------------------------- end of file -------------------------------- */
