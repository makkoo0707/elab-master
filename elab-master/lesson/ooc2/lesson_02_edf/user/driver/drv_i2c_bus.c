
/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include <string.h>
#include "drv_util.h"
#include "elab/common/elab_assert.h"
#include "elab/common/elab_def.h"
#include "elab/common/elab_export.h"
#include "elab/edf/normal/elab_i2c.h"
#include "stm32g0xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

ELAB_TAG("DriverI2C");

/* private function prototype ----------------------------------------------- */
static int32_t _xfer(elab_i2c_bus_t *, uint16_t addr, elab_i2c_msg_t msg);
static elab_err_t _config(elab_i2c_bus_t *, elab_i2c_bus_config_t *config);

/* private variables -------------------------------------------------------- */
static const elab_i2c_bus_ops_t pin_driver_ops =
{
    .xfer = _xfer,
    .config = _config,
};

static I2C_HandleTypeDef hi2c1;
static elab_i2c_bus_t i2c_bus_1;

/* public functions --------------------------------------------------------- */
static void driver_i2c_bus_init(void)
{
    HAL_StatusTypeDef status = HAL_OK;

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_I2C1_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF6_I2C1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    hi2c1.Instance = I2C1;
    hi2c1.Init.Timing = 0x00400B27;
    hi2c1.Init.OwnAddress1 = 0x3f;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    status = HAL_I2C_Init(&hi2c1);
    elab_assert(status == HAL_OK);

    /** Configure analogue and digital filter 
     */
    status = HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE);
    elab_assert(status == HAL_OK);
    status = HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0);
    elab_assert(status == HAL_OK);

    HAL_I2CEx_EnableFastModePlus(I2C_FASTMODEPLUS_I2C1);
    HAL_NVIC_SetPriority(I2C1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(I2C1_IRQn);

    elab_i2c_bus_register(&i2c_bus_1, "I2C1", &pin_driver_ops, &hi2c1);
}
INIT_EXPORT(driver_i2c_bus_init, EXPORT_LEVEL_BSP);

/* private functions -------------------------------------------------------- */
/**
  * @brief  The PIN driver initialization function.
  * @param  me  PIN device handle.
  * @retval None.
  */
static int32_t _xfer(elab_i2c_bus_t *me, uint16_t addr, elab_i2c_msg_t msg)
{
    I2C_HandleTypeDef *i2c = (I2C_HandleTypeDef *)me->super.user_data;
    HAL_StatusTypeDef status = HAL_OK;

    if (msg.write)
    {
        status = HAL_I2C_Master_Transmit_IT(i2c, addr, msg.buffer, msg.len);
    }
    else
    {
        status = HAL_I2C_Master_Receive_IT(i2c, addr, msg.buffer, msg.len);
    }
    (void)status;

    return (int32_t)msg.len;
}

static elab_err_t _config(elab_i2c_bus_t *me, elab_i2c_bus_config_t *config)
{
    (void)me;
    (void)config;

    /* TODO */

    return ELAB_OK;
}

void I2C1_IRQHandler(void)
{
    HAL_I2C_EV_IRQHandler(&hi2c1);
    HAL_I2C_ER_IRQHandler(&hi2c1);
}

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    elab_i2c_xfer_end(&i2c_bus_1);
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    elab_i2c_xfer_end(&i2c_bus_1);
}

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
