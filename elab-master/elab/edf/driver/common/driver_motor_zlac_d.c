
/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* include ------------------------------------------------------------------ */
#include <math.h>
#include "driver_motor.h"
#include "elab/midware/edb/edb.h"
#include "elab/common/elab_def.h"
#include "elab/common/elab_assert.h"
#include "elab/common/elab_log.h"

ELAB_TAG("MotorDriverZLAC8015D");

/* private config ----------------------------------------------------------- */
#define MOTOR_SET_MB_TIMES                  (5)
#define MOTOR_SERIAL_NAME_SIZE              (32)

/* private function prototype ----------------------------------------------- */
static elab_err_t _init(elab_motor_t *const me);
static elab_err_t _enable(elab_motor_t *const me, bool status);
static elab_err_t _emg_stop(elab_motor_t *const me);
static bool _ready(elab_motor_t *const me);
static elab_err_t _set_speed(elab_motor_t *const me, float speed);
static elab_err_t _get_speed(elab_motor_t *const me, float *speed);
static elab_err_t _get_error(elab_motor_t *const me, uint32_t *error_out);
static bool _check_motor_valid(char *motor_name);
static uint32_t _get_serial_id(char *name, char *serial_name_out, uint32_t size);
static uint8_t _get_mb_address(char *name);
static uint8_t _get_motor_id(char *name);

/* private variables -------------------------------------------------------- */
static char str_serial_name[MOTOR_SERIAL_NAME_SIZE];
static const elab_motor_ops_t _motor_ops =
{
    .init = _init,
    .enable = _enable,
    .emg_stop = _emg_stop,
    .ready = _ready,
    .get_error = _get_error,
    .set_speed = _set_speed,
    .get_speed = _get_speed,
};

static uint32_t flag_mb_init = 0;

/* public functions --------------------------------------------------------- */
void drv_motor_zlac_d_init(driver_motor_zlac_t *motor,
                            const char *dev_name, const char *drv_name)
{
    elab_assert(_check_motor_valid((char *)drv_name));

    elog_debug("driver_motor_cfg start.");

    bool success = false;
    bool motor_controller_init_end = false;
    uint16_t ret = MODBUS_ERR_NONE;
    memset(str_serial_name, 0, MOTOR_SERIAL_NAME_SIZE);

    /* Motor channel. */
    motor->address = _get_mb_address((char *)drv_name);
    memset(motor->dev_name, 0, sizeof(motor->dev_name));
    strcpy(motor->dev_name, dev_name);
    assert_id(motor->address > 0 && motor->address <= 32, motor->address);
    motor->id = _get_motor_id((char *)drv_name);
    assert_id(motor->id > 0 && motor->id <= 2, motor->id);
    _get_serial_id((char *)drv_name,
                    str_serial_name,
                    MOTOR_SERIAL_NAME_SIZE);

    motor->mb_ch = mb_channel_create(str_serial_name, motor->address,
                                        MODBUS_MASTER, 500, MODBUS_MODE_RTU);
    elab_assert(motor->mb_ch != NULL);

    /* Get base parameter from ini file. */
    uint32_t motor_ratio = edb_get_u32("Base", "motor_ratio");

    if (!motor_controller_init_end)
    {
        /*  Record the initialization status of the motor controller because one
            controller controlling two motors. */
        flag_mb_init |= (1 << (motor->address - 1));

        /* Set the speed mode. */
        for (uint16_t i = 0; i < MOTOR_SET_MB_TIMES; i ++)
        {
            elog_debug("Motor driver set speed mode %u.", motor->id);
            ret = mbm_fc06_holding_reg_write(motor->mb_ch, motor->address,
                                                0x200D, 3);
            if (ret == MODBUS_ERR_NONE)
            {
                success = true;
                break;
            }
            else
            {
                elog_error("mbm_fc06_holding_reg_write error: %u.", ret);
            }
        }
        assert_id(ret == MODBUS_ERR_NONE, ret);
        elog_debug("Motor driver set speed mode end.");

        /* Enable all the motor. */
        for (uint16_t i = 0; i < MOTOR_SET_MB_TIMES; i ++)
        {
            elog_debug("Motor driver enabling %u.", motor->id);
            ret = mbm_fc06_holding_reg_write(motor->mb_ch, motor->address, 0x200E, 8);
            if (ret == MODBUS_ERR_NONE)
            {
                break;
            }
        }
        assert_id(ret == MODBUS_ERR_NONE, ret);
        elog_debug("Motor driver enabling end.");

        /* Set driver bus timeout. */
        for (uint16_t i = 0; i < MOTOR_SET_MB_TIMES; i ++)
        {
            elog_debug("Motor driver enabling %d.", i);
            ret = mbm_fc06_holding_reg_write(motor->mb_ch,
                                                motor->address, 0x2000, 1500);
            if (ret == MODBUS_ERR_NONE)
            {
                break;
            }
        }
        assert_id(ret == MODBUS_ERR_NONE, ret);
        elog_debug("Motor driver bus timeout setting end.");
    }

    /* Set all the motors to the speed of zero. */
    ret = mbm_fc06_holding_reg_write(motor->mb_ch, motor->address,
                                        motor->cmd_speed_reg_addr, 0);
    assert_id(ret == MODBUS_ERR_NONE, ret);
    elog_debug("Motor driver set speed to end.");

    elab_motor_init(ELAB_MOTOR_CAST(motor),
                    motor->dev_name,
                    (elab_motor_ops_t *)&_motor_ops, (void *)motor);
    elab_motor_config_ratio(motor->dev_name, motor_ratio);
    elog_debug("driver_motor_cfg end.");

    motor->ready = true;

exit:
    return;
}

/* private functions -------------------------------------------------------- */
static elab_err_t _init(elab_motor_t *const me)
{
    (void)me;
    return ELAB_OK;
}

static elab_err_t _set_control_word(elab_motor_t *const me, uint16_t value)
{
    driver_motor_zlac_t *data = (driver_motor_zlac_t *)me->super.user_data;
    elab_err_t ret = ELAB_OK;

    uint16_t ret_mb = MODBUS_ERR_NONE;
    ret_mb = mbm_fc06_holding_reg_write(data->mb_ch, data->address, 0x200E, value);
    if (MODBUS_ERR_NONE != ret_mb)
    {
        elog_error("Motor driver ZLAC8015D. modbus error %u.", ret_mb);
        ret = ELAB_ERR_BUS;
    }
    
    return ret;
}

static elab_err_t _enable(elab_motor_t *const me, bool status)
{
    uint16_t value = status ? 8 : 7;
    elab_err_t ret = _set_control_word(me, value);
    if (ELAB_OK != ret)
    {
        elog_error("_enable, modbus error.");
    }
    
    return ret;
}

static elab_err_t _emg_stop(elab_motor_t *const me)
{
    elab_err_t ret = _set_control_word(me, 5);
    if (ELAB_OK != ret)
    {
        elog_error("_enable, modbus error.");
    }
    
    return ret;
}

static bool _ready(elab_motor_t *const me)
{
    driver_motor_zlac_t *data = (driver_motor_zlac_t *)me->super.user_data;
    elab_assert(data != NULL);

    return data->ready;
}

static elab_err_t _set_speed(elab_motor_t *const me, float speed)
{
    driver_motor_zlac_t *data = (driver_motor_zlac_t *)me->super.user_data;
    elab_assert(data != NULL);
    elab_assert(data->address == 1 || data->address == 2);

    elab_err_t ret = ELAB_OK;

    uint16_t ret_mb = MODBUS_ERR_NONE;
    ret_mb = mbm_fc06_holding_reg_write(data->mb_ch,
                                        data->address,
                                        data->cmd_speed_reg_addr,
                                        (uint16_t)((int16_t)speed));
    if (MODBUS_ERR_NONE != ret_mb)
    {
        elog_error("Motor driver ZLAC8015D. _set_speed modbus error %u.", ret_mb);
        ret = ELAB_ERR_BUS;
    }

    return ret;
}

static elab_err_t _get_reg_value(elab_motor_t *const me, uint16_t addr, uint16_t *value)
{
    driver_motor_zlac_t *data = (driver_motor_zlac_t *)me->super.user_data;
    elab_err_t ret = ELAB_OK;

    uint16_t ret_mb = MODBUS_ERR_NONE;
    ret_mb = mbm_fc03_holding_reg_read(data->mb_ch,
                                        data->address,
                                        addr,
                                        value, 1);
    if (MODBUS_ERR_NONE != ret_mb)
    {
        elog_error("Motor driver ZLAC8015D. modbus error %u.", ret_mb);
        ret = ELAB_ERR_BUS;
    }

    return ret;
}

static elab_err_t _get_speed(elab_motor_t *const me, float *speed)
{
    driver_motor_zlac_t *data = (driver_motor_zlac_t *)me->super.user_data;
    uint16_t value = 0;
    elab_err_t ret = _get_reg_value(me, data->act_speed_reg_addr, &value);
    if (ELAB_OK != ret)
    {
        elog_error("Motor driver ZLAC8015D. _get_speed modbus error.");
        ret = ELAB_ERR_BUS;
    }

    *speed = (float)value / 10.0;

    return ret;
}

static elab_err_t _get_error(elab_motor_t *const me, uint32_t *error_out)
{
    driver_motor_zlac_t *data = (driver_motor_zlac_t *)me->super.user_data;
    uint16_t value = 0;
    elab_err_t ret = _get_reg_value(me, data->error_reg_addr, &value);
    if (ELAB_OK != ret)
    {
        elog_error("Motor driver ZLAC8015D. _get_speed modbus error.");
        ret = ELAB_ERR_BUS;
    }

    *error_out = (uint32_t)value;

    return ret;
}

static bool _check_motor_valid(char *name)
{
    bool valid = true;
    uint8_t address = 0;
    char *example = "zlac_d.rs485_1.01.1";
    char *str_head = "zlac_d.rs";
    char *str_addr = &name[strlen("zlac_d.rs485_1.")];
    int32_t error_id = 0;

    if (strlen(example) != strlen(name))
    {
        valid = false;
        error_id = -1;
        goto exit;
    }
    if (strncmp(name, str_head, strlen(str_head)) != 0)
    {
        valid = false;
        error_id = -2;
        goto exit;
    }
    if (strncmp(&name[strlen(str_head)], "485_", 4) != 0 &&
        strncmp(&name[strlen(str_head)], "232_", 4) != 0)
    {
        valid = false;
        error_id = -3;
        goto exit;
    }
    if (name[strlen("zlac_d.rs485_1")] != '.' ||
        name[strlen("zlac_d.rs485_1.01")] != '.')
    {
        valid = false;
        error_id = -4;
        goto exit;
    }
    if (name[strlen("zlac_d.rs485_1.01.")] != '1' &&
        name[strlen("zlac_d.rs485_1.01.")] != '2')
    {
        valid = false;
        error_id = -5;
        goto exit;
    }
    for (uint8_t i = 0; i < 2; i ++)
    {
        address *= 10;
        if (str_addr[i] >= '0' && str_addr[i] <= '9')
        {
            address += (str_addr[i] - '0');
        }
        else
        {
            valid = false;
            error_id = -6;
            goto exit;
        }
    }
    elab_assert(address <= 32 && address > 0);

exit:
    if (error_id != 0)
    {
        printf("Error ID: %d.\n", error_id);
    }
    return valid;
}

static uint32_t _get_serial_id(char *name, char *serial_name_out, uint32_t size)
{
    memset(serial_name_out, 0, size);
    char *str_name = &name[strlen("zlac_d.")];
    uint32_t length = 0;
    for (uint32_t i = 0; i < (strlen(name) - strlen("zlac_d.")); i ++)
    {
        if (str_name[i] != '.')
        {
            length ++;
        }
        else
        {
            break;
        }
    }

    length = length >= (size - 1) ? (size - 1) : length;
    memcpy(serial_name_out, &name[strlen("zlac_d.")], length);

    return length;
}

static uint8_t _get_mb_address(char *name)
{
    uint8_t address = 0;
    char *str_addr = &name[strlen("zlac_d.rs485_1.")];
    for (uint8_t i = 0; i < 2; i ++)
    {
        address *= 10;
        address += (str_addr[i] - '0');
    }

    return address;
}

static uint8_t _get_motor_id(char *name)
{
    return (name[strlen("zlac_d.rs485_1.01.")] - '0');
}
/* ----------------------------- end of file -------------------------------- */
