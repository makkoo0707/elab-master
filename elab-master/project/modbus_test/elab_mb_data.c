#include "mb.h"
#include <stdio.h>

uint16_t MB_RTU_Freq = 1000;

bool MB_CoilRd(uint16_t coil, uint16_t *perr)
{
    *perr = MODBUS_ERR_NONE;
    return (0x54 & (1 << (coil - 1000))) ? true : false;
}

void MB_CoilWr(uint16_t coil, bool coil_val, uint16_t *perr)
{
    *perr = MODBUS_ERR_NONE;
    printf("coil %u value: %d.\n", coil, coil_val);
}

bool MB_DIRd(uint16_t di, uint16_t *perr)
{
    *perr = MODBUS_ERR_NONE;

    return (0x86 & (1 << (di - 1000))) ? true : false;
}

uint16_t MB_InRegRd(uint16_t reg, uint16_t *perr)
{
    *perr = MODBUS_ERR_NONE;

    return (reg + 300);
}

float MB_InRegRdFP(uint16_t reg, uint16_t *perr)
{

}

uint16_t MB_HoldingRegRd(uint16_t reg, uint16_t *perr)
{
    *perr = MODBUS_ERR_NONE;

    return (reg + 128);
}

float MB_HoldingRegRdFP(uint16_t reg, uint16_t *perr)
{
    *perr = MODBUS_ERR_NONE;

    return ((float)reg + 0.666);
}

void MB_HoldingRegWr(uint16_t reg, uint16_t reg_val_16, uint16_t *perr)
{
    *perr = 3000;
    printf("HoldingRegWr %u value: %u.\n", reg, reg_val_16);
}

void MB_HoldingRegWrFP(uint16_t reg, float reg_val_fp, uint16_t *perr)
{
    *perr = MODBUS_ERR_NONE;
    printf("MB_HoldingRegWrFP %u value: %f.\n", reg, reg_val_fp);
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
