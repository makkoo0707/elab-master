
/* include ------------------------------------------------------------------ */
#include "elab/common/elab_assert.h"
#include "simu_mbs.h"

ELAB_TAG("SimuMbs");

/* private functions ---------------------------------------------------------*/
static bool _coil_read(uint16_t coil, uint16_t *perr);
static void _coil_write(uint16_t coil, bool coil_val, uint16_t *perr);
static bool _di_read(uint16_t di, uint16_t *perr);
static uint16_t _in_reg_read(uint16_t reg, uint16_t *perr);
static float _in_reg_read_fp(uint16_t reg, uint16_t *perr);
static uint16_t _holding_reg_read(uint16_t reg, uint16_t *perr);
static float _holding_reg_read_fp(uint16_t reg, uint16_t *perr);
static void _holding_reg_write(uint16_t reg, uint16_t reg_val_16, uint16_t *perr);
static void _holding_reg_write_fp(uint16_t reg, float reg_val_fp, uint16_t *perr);

/* private variables -------------------------------------------------------- */
static mb_channel_cb_t _cb_light =
{
    .coil_read              = _coil_read,
    .coil_write             = _coil_write,
    .di_read                = _di_read,
    .in_reg_read            = _in_reg_read,
    .in_reg_read_fp         = _in_reg_read_fp,
    .holding_reg_read       = _holding_reg_read,
    .holding_reg_read_fp    = _holding_reg_read_fp,
    .holding_reg_write      = _holding_reg_write,
    .holding_reg_write_fp   = _holding_reg_write_fp,
};

/* public functions ----------------------------------------------------------*/
void drv_simu_mbs_init(driver_simu_mbs_t *simu_mbs,
                        const char *dev_name, const char *drv_name)
{
    elab_assert(strlen(dev_name) < ELAB_NAME_SIZE);
    elab_assert(strlen(drv_name) < ELAB_NAME_SIZE);

    memset(simu_mbs->dev_name, 0, ELAB_NAME_SIZE);
    memset(simu_mbs->drv_name, 0, ELAB_NAME_SIZE);
    strcpy(simu_mbs->dev_name, dev_name);
    strcpy(simu_mbs->drv_name, drv_name);

    uint8_t node_addr = drv_name[strlen(drv_name) - 1] - '0';

    simu_mbs->ch = mb_channel_create(drv_name,
                                        node_addr,
                                        MODBUS_SLAVE, 500, MODBUS_MODE_RTU);
    elab_assert(simu_mbs->ch != NULL);
    mb_slave_write_enable(simu_mbs->ch, true);
    mb_slave_set_cb(simu_mbs->ch, &_cb_light);
}

/* private functions ---------------------------------------------------------*/
static bool _coil_read(uint16_t coil, uint16_t *perr)
{
    (void)coil;
    
    *perr = MODBUS_ERR_NONE;

    return false;
}

static void _coil_write(uint16_t coil, bool coil_val, uint16_t *perr)
{
    (void)coil;
    (void)coil_val;
    
    *perr = MODBUS_ERR_NONE;
}

static bool _di_read(uint16_t di, uint16_t *perr)
{
    (void)di;
    
    *perr = MODBUS_ERR_NONE;

    return false;
}

static uint16_t _in_reg_read(uint16_t reg, uint16_t *perr)
{
    (void)reg;
    
    *perr = MODBUS_ERR_NONE;

    return 0;
}

static float _in_reg_read_fp(uint16_t reg, uint16_t *perr)
{
    (void)reg;
    
    *perr = MODBUS_ERR_NONE;

    return 0.0;
}

static uint16_t _holding_reg_read(uint16_t reg, uint16_t *perr)
{
    (void)reg;
    
    *perr = MODBUS_ERR_NONE;

    return 0;
}

static float _holding_reg_read_fp(uint16_t reg, uint16_t *perr)
{
    (void)reg;
    
    *perr = MODBUS_ERR_NONE;

    return 0.0;
}

static void _holding_reg_write(uint16_t reg, uint16_t reg_val_16, uint16_t *perr)
{
    (void)reg;
    (void)reg_val_16;

    printf("reg: %u, value: 0x%04x.\n", reg, reg_val_16);

    *perr = MODBUS_ERR_NONE;
}

static void _holding_reg_write_fp(uint16_t reg, float reg_val_fp, uint16_t *perr)
{
    (void)reg;
    (void)reg_val_fp;
    
    *perr = MODBUS_ERR_NONE;
}

/* ----------------------------- end of file -------------------------------- */
