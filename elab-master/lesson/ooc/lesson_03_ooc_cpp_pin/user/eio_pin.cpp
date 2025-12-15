
/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include <string.h>
#include "eio_pin.h"
#include "elab_assert.h"

ELAB_TAG("EIO_PIN");

/* private variables -------------------------------------------------------- */
static const GPIO_TypeDef *gpio_table[] =
{
    GPIOA, GPIOB, GPIOC, GPIOD,
};

/* private function prototype ----------------------------------------------- */
static bool _check_pin_name_valid(const char *name);
static void _translate_pin_name(const char *name, eio_pin_data_t *data);

/* public functions --------------------------------------------------------- */
/**
  * @brief  EIO pin initialization.
  * @param  name    pin's name, just like "A.01" or "B.14" and so on.
  * @param  mode    pin's mode.
  * @retval None
  */
void eio_pin_t::init(const char *name, enum pin_mode mode)
{
    bool valid = _check_pin_name_valid(name);
    elab_assert(valid);

    _translate_pin_name(name, &this->data_);
    this->mode_ = mode;

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
    else if (mode == PIN_MODE_OUTPUT)
    {
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
    }
    else if (mode == PIN_MODE_OUTPUT_OD)
    {
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
    }

    GPIO_InitStruct.Pin = data_.pin;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(data_.gpio_x, &GPIO_InitStruct);

    get_status();
}

/**
  * @brief  EIO pin's status turning on function.
  * @param  status  the input pin status.
  * @retval None.
  */
void eio_pin_t::set_status(bool status)
{
    elab_assert(mode_ == PIN_MODE_OUTPUT || mode_ == PIN_MODE_OUTPUT_OD);
    
    if (status != status_)
    {
        HAL_GPIO_WritePin(data_.gpio_x,
                            data_.pin,
                            status ? GPIO_PIN_SET : GPIO_PIN_RESET);

        get_status();
        elab_assert(status_ == status);
    }
}

/**
  * @brief  EIO pin's status getting function.
  * @retval The pin's status.
  */
bool eio_pin_t::get_status(void)
{
    GPIO_PinState status = HAL_GPIO_ReadPin(data_.gpio_x, data_.pin);
    status_ = (status == GPIO_PIN_SET) ? true : false;

    return status_;
}

/* private functions -------------------------------------------------------- */
/**
  * @brief  Check the given pin name is valid or not.
  * @param  name    The given pin name.
  * @retval None.
  */
static bool _check_pin_name_valid(const char *name)
{
    bool ret = true;
    uint8_t pin_number;

    if (!(strlen(name) == 4 && (name[1] == '.')))
    {
        ret = false;
        goto exit;
    }
    
    if ((name[0] < 'A' || name[0] > 'D') ||
            (name[2] < '0' || name[2] > '1') ||
            (name[3] < '0' || name[3] > '9'))
    {
        ret = false;
        goto exit;
    }

    pin_number = (name[2] - '0') * 10 + (name[3] - '0');
    if (pin_number >= 16)
    {
        ret = false;
        goto exit;
    }

exit:
    return ret;
}

/**
  * @brief  Translate the pin name to MCU relate pin data structure.
  * @param  name    The given pin name.
  * @param  data    The pin data output.
  * @retval None.
  */
// A.14 C.02
static void _translate_pin_name(const char *name, eio_pin_data_t *data)
{
    data->gpio_x = (GPIO_TypeDef *)gpio_table[name[0] - 'A'];
    data->pin = (1 << ((uint8_t)((name[2] - '0') * 10 + (name[3] - '0'))));

    if (name[0] == 'A')
    {
        __HAL_RCC_GPIOA_CLK_ENABLE();
    }
    else if (name[0] == 'B')
    {
        __HAL_RCC_GPIOB_CLK_ENABLE();
    }
    else if (name[0] == 'C')
    {
        __HAL_RCC_GPIOC_CLK_ENABLE();
    }
    else if (name[0] == 'D')
    {
        __HAL_RCC_GPIOD_CLK_ENABLE();
    }
}

/* ----------------------------- end of file -------------------------------- */
