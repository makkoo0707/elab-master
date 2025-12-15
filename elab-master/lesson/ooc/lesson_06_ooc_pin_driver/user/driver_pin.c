
/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include <string.h>
#include "eio_pin.h"
#include "elab_assert.h"
#include "elab_def.h"
#include "elab_export.h"
#include "stm32g0xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

ELAB_TAG("Driver_PIN");

/* public typedef ----------------------------------------------------------- */
typedef struct eio_pin_data
{
    eio_pin_t *device;
    const char *name;
    GPIO_TypeDef *gpio_x;
    uint16_t pin;
} eio_pin_data_t;

/* private function prototype ----------------------------------------------- */
static void _init(eio_pin_t * const me);
static void _set_mode(eio_pin_t * const me, uint8_t mode);
static bool _get_status(eio_pin_t * const me);
static void _set_status(eio_pin_t * const me, bool status);

/* private variables -------------------------------------------------------- */
static eio_pin_t pin_c_08;
static eio_pin_t pin_c_09;

static const eio_pin_ops_t pin_driver_ops =
{
    .init = _init,
    .set_mode = _set_mode,
    .get_status = _get_status,
    .set_status = _set_status,
};

static eio_pin_data_t eio_pin_driver_data[] =
{
    { &pin_c_08, "LED1", GPIOC, GPIO_PIN_8, },
    { &pin_c_09, "LED2", GPIOC, GPIO_PIN_9, },
};

/* public functions --------------------------------------------------------- */
static void eio_pin_dirver_init(void)
{
    for (uint32_t i = 0;
            i < sizeof(eio_pin_driver_data) / sizeof(eio_pin_data_t); i ++)
    {
        /* Enable the clock. */
        if (eio_pin_driver_data[i].gpio_x == GPIOA)
        {
            __HAL_RCC_GPIOA_CLK_ENABLE();
        }
        else if (eio_pin_driver_data[i].gpio_x == GPIOB)
        {
            __HAL_RCC_GPIOB_CLK_ENABLE();
        }
        else if (eio_pin_driver_data[i].gpio_x == GPIOC)
        {
            __HAL_RCC_GPIOC_CLK_ENABLE();
        }
        else if (eio_pin_driver_data[i].gpio_x == GPIOD)
        {
            __HAL_RCC_GPIOD_CLK_ENABLE();
        }

        /* Device registering. */
        eio_pin_register(eio_pin_driver_data[i].device,
                            eio_pin_driver_data[i].name,
                            &pin_driver_ops,
                            &eio_pin_driver_data[i]);
    }
}
INIT_EXPORT(eio_pin_dirver_init, 1);

/* private functions -------------------------------------------------------- */
/**
  * @brief  The PIN driver initialization function.
  * @param  me  PIN device handle.
  * @retval None.
  */
static void _init(eio_pin_t * const me)
{
    eio_pin_data_t *driver_data = (eio_pin_data_t *)me->user_data;

    /* Configure GPIO pin. */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Pin = driver_data->pin;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(driver_data->gpio_x, &GPIO_InitStruct);

    HAL_GPIO_WritePin(driver_data->gpio_x,
                        driver_data->pin,
                        GPIO_PIN_RESET);
}

/**
  * @brief  The PIN driver set_mode function.
  * @param  me  PIN device handle.
  * @retval None.
  */
static void _set_mode(eio_pin_t * const me, uint8_t mode)
{
    elab_assert(me != NULL);
    elab_assert(mode < PIN_MODE_MAX);

    eio_pin_data_t *driver_data = (eio_pin_data_t *)me->user_data;

    /* Configure GPIO pin. */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (mode == PIN_MODE_INPUT)
    {
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
    }
    else if (mode == PIN_MODE_INPUT_PULLUP)
    {
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
    }
    else if (mode == PIN_MODE_INPUT_PULLDOWN)
    {
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    }
    else if (mode == PIN_MODE_OUTPUT_PP)
    {
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
    }
    else if (mode == PIN_MODE_OUTPUT_OD)
    {
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
    }
    GPIO_InitStruct.Pin = driver_data->pin;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(driver_data->gpio_x, &GPIO_InitStruct);
}

/**
  * @brief  The PIN driver set_mode function.
  * @param  me  PIN device handle.
  * @retval GPIO status.
  */
static bool _get_status(eio_pin_t * const me)
{
    elab_assert(me != NULL);

    eio_pin_data_t *driver_data = (eio_pin_data_t *)me->user_data;

    GPIO_PinState status = HAL_GPIO_ReadPin(driver_data->gpio_x, driver_data->pin);

    return (status == GPIO_PIN_SET) ? true : false;
}

/**
  * @brief  The PIN driver set_status function.
  * @param  me      PIN device handle.
  * @param  status  GPIO status.
  * @retval None.
  */
static void _set_status(eio_pin_t * const me, bool status)
{
    elab_assert(me != NULL);

    eio_pin_data_t *driver_data = (eio_pin_data_t *)me->user_data;

    HAL_GPIO_WritePin(driver_data->gpio_x,
                        driver_data->pin,
                        status ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/* ----------------------------- end of file -------------------------------- */
