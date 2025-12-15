
/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include <string.h>
#include "drv_pwm.h"
#include "drv_util.h"
#include "elab/common/elab_assert.h"
#include "elab/common/elab_def.h"
#include "elab/common/elab_export.h"
#include "stm32g0xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

ELAB_TAG("DriverPin");

/* private typedef ---------------------------------------------------------- */
typedef struct elab_pwm_driver_data
{
    const char *pin_name;
    GPIO_TypeDef *gpio_x;
    uint16_t pin;
    uint32_t alternate;
    TIM_HandleTypeDef *htim;
    uint32_t channel;
} elab_pwm_driver_data_t;

/* private function prototype ----------------------------------------------- */
static void _set_duty(elab_pwm_t * const me, uint8_t duty_ratio);

static void _hardware_init(elab_pwm_driver_data_t *driver_data);
static elab_pwm_driver_data_t *_get_driver_data(const char *name);

/* private variables -------------------------------------------------------- */
static const elab_pwm_ops_t pin_driver_ops =
{
    .set_duty = _set_duty,
};

static bool timer1_init_end = false;
static bool timer15_init_end = false;
static TIM_HandleTypeDef htim1;
static TIM_HandleTypeDef htim15;

static const elab_pwm_driver_data_t pwm_driver_data[] =
{
    {
        "C.08", GPIOC, GPIO_PIN_8, GPIO_AF2_TIM1, &htim1, TIM_CHANNEL_1,
    },
    {
        "C.09", GPIOC, GPIO_PIN_9, GPIO_AF2_TIM1, &htim1, TIM_CHANNEL_2,
    },
    {
        "C.01", GPIOC, GPIO_PIN_1, GPIO_AF2_TIM15, &htim15, TIM_CHANNEL_1,
    },
};

/* public functions --------------------------------------------------------- */
void elab_driver_pwm_init(elab_pwm_driver_t *me,
                            const char *name, const char *pin_name)
{
    elab_assert(me != NULL);
    elab_assert(name != NULL);
    assert_name(check_pin_name_valid(pin_name), pin_name);

    elab_pwm_driver_data_t *driver_data = _get_driver_data(pin_name);
    elab_assert(driver_data != NULL);

    _hardware_init(driver_data);
    
    me->pin_name = pin_name;

    elab_pwm_register(&me->device, name, &pin_driver_ops, me);
}

/* private functions -------------------------------------------------------- */
/**
  * @brief  The PIN driver initialization function.
  * @param  me  PIN device handle.
  * @retval None.
  */
static void _set_duty(elab_pwm_t * const me, uint8_t duty_ratio)
{
    elab_assert(me != NULL);
    elab_pwm_driver_t *driver = (elab_pwm_driver_t *)me->super.user_data;
    elab_pwm_driver_data_t *driver_data = _get_driver_data(driver->pin_name);
    elab_assert(driver_data != NULL);

    HAL_TIM_PWM_Stop(driver_data->htim, driver_data->channel);
    
    TIM_OC_InitTypeDef sConfigOC = {0};
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = duty_ratio * 10;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    HAL_TIM_PWM_ConfigChannel(driver_data->htim, &sConfigOC, driver_data->channel);

    HAL_TIM_PWM_Start(driver_data->htim, driver_data->channel);
}

static elab_pwm_driver_data_t *_get_driver_data(const char *name)
{
    elab_pwm_driver_data_t *driver_data = NULL;

    for (uint32_t i = 0;
            i < sizeof(pwm_driver_data) / sizeof(elab_pwm_driver_data_t); i ++)
    {
        if (strcmp(name, pwm_driver_data[i].pin_name) == 0)
        {
            driver_data = (elab_pwm_driver_data_t *)&pwm_driver_data[i];
            break;
        }
    }

    return driver_data;
}

static void _hardware_init(elab_pwm_driver_data_t *driver_data)
{
    gpio_clock_enable(driver_data->pin_name);

    if (driver_data->alternate == GPIO_AF2_TIM1)
    {
        __HAL_RCC_TIM1_CLK_ENABLE();

        if (!timer1_init_end)
        {
            timer1_init_end = true;

            htim1.Instance = TIM1;
            htim1.Init.Prescaler = 63;
            htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
            htim1.Init.Period = 999;
            htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
            htim1.Init.RepetitionCounter = 0;
            htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
            HAL_TIM_PWM_Init(&htim1);
        }
    }
    if (driver_data->alternate == GPIO_AF2_TIM15)
    {
        __HAL_RCC_TIM15_CLK_ENABLE();

        if (!timer15_init_end)
        {
            timer15_init_end = true;

            htim15.Instance = TIM15;
            htim15.Init.Prescaler = 63;
            htim15.Init.CounterMode = TIM_COUNTERMODE_UP;
            htim15.Init.Period = 999;
            htim15.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
            htim15.Init.RepetitionCounter = 0;
            htim15.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
            HAL_TIM_PWM_Init(&htim15);
        }
    }

    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};
    TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    HAL_TIMEx_MasterConfigSynchronization(driver_data->htim, &sMasterConfig);

    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    HAL_TIM_PWM_ConfigChannel(driver_data->htim, &sConfigOC, driver_data->channel);

    sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
    sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
    sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
    sBreakDeadTimeConfig.DeadTime = 0;
    sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
    sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
    sBreakDeadTimeConfig.BreakFilter = 0;
    sBreakDeadTimeConfig.BreakAFMode = TIM_BREAK_AFMODE_INPUT;
    sBreakDeadTimeConfig.Break2State = TIM_BREAK2_DISABLE;
    sBreakDeadTimeConfig.Break2Polarity = TIM_BREAK2POLARITY_HIGH;
    sBreakDeadTimeConfig.Break2Filter = 0;
    sBreakDeadTimeConfig.Break2AFMode = TIM_BREAK_AFMODE_INPUT;
    sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
    HAL_TIMEx_ConfigBreakDeadTime(driver_data->htim, &sBreakDeadTimeConfig);

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = driver_data->pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = driver_data->alternate;
    HAL_GPIO_Init(driver_data->gpio_x, &GPIO_InitStruct);
}

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
