/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include "mb.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

uint16_t MB_RTU_Freq = 1000;

bool MB_CoilRd(uint16_t coil, uint16_t *perr)
{
    *perr = MODBUS_ERR_NONE;
    return (0x55 & (1 << (coil - 1000))) ? true : false;
}

void MB_CoilWr(uint16_t coil, bool coil_val, uint16_t *perr)
{

}

bool MB_DIRd(uint16_t di, uint16_t *perr)
{
    return false;
}

uint16_t MB_InRegRd(uint16_t reg, uint16_t *perr)
{

}

float MB_InRegRdFP(uint16_t reg, uint16_t *perr)
{

}

uint16_t MB_HoldingRegRd(uint16_t reg, uint16_t *perr)
{

}

float MB_HoldingRegRdFP(uint16_t reg, uint16_t *perr)
{

}

void MB_HoldingRegWr(uint16_t reg, uint16_t reg_val_16, uint16_t *perr)
{

}

void MB_HoldingRegWrFP(uint16_t reg, float reg_val_fp, uint16_t *perr)
{

}

uint16_t elab_mb_file_read(uint16_t file_nbr,
                            uint16_t record_nbr,
                            uint16_t ix,
                            uint8_t record_len,
                            uint16_t *perr)
{
    
}

void elab_mb_file_write(uint16_t file_nbr, uint16_t record_nbr,
                        uint16_t ix, uint8_t record_len,
                        uint16_t value, uint16_t *perr)
{
    
}

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
