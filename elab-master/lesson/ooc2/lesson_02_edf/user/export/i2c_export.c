
#include "elab/common/elab_export.h"
#include "elab/edf/normal/elab_i2c.h"
#include "export.h"

static elab_i2c_t i2c_exio;
static elab_i2c_t i2c_eeprom;

static void driver_i2c_export(void)
{
    static const elab_i2c_config_t exio_i2c_config =
    {
        .addr_10bit = false,
        .addr = 0x38,
        .clock = 400000,
    };
    elab_i2c_register(&i2c_exio, "i2c_exio", "I2C1", exio_i2c_config);

    static const elab_i2c_config_t eeprom_i2c_config =
    {
        .addr_10bit = false,
        .addr = 0xA0,
        .clock = 400000,
    };
    elab_i2c_register(&i2c_eeprom, "i2c_eeprom", "I2C1", eeprom_i2c_config);
}
INIT_EXPORT(driver_i2c_export, EXPORT_LEVEL_I2C);
