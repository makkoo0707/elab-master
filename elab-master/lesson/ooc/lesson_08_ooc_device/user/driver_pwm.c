
/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include "eio_pwm.h"
#include "stm32g0xx_hal.h"
#include "elab_export.h"

#ifdef __cplusplus
extern "C" {
#endif

/* public typedef ----------------------------------------------------------- */
typedef struct eio_pwm_data
{
    eio_pwm_t *device;
    const char *name;
    GPIO_TypeDef *gpio_x;
    uint16_t pin;
    uint32_t alternate;
    TIM_HandleTypeDef *htim;
    uint32_t channel;
} eio_pwm_data_t;

/* private function prototype ----------------------------------------------- */
static void _init(eio_pwm_t * const me);
static void _set_duty(eio_pwm_t * const me, uint8_t duty_ratio);

static void _timer_pwm_init(eio_pwm_data_t *data);
static void _gpio_pwm_init(eio_pwm_data_t *data);

/* private variables -------------------------------------------------------- */
static eio_pwm_t pwm_led3;
static eio_pwm_t pwm_led4;
static eio_pwm_t pwm_adc_in;

static TIM_HandleTypeDef htim1;
static TIM_HandleTypeDef htim15;

static eio_pwm_data_t pwm_driver_data[] =
{
    {
        &pwm_led3, "pwmled3",
        GPIOC, GPIO_PIN_8, GPIO_AF2_TIM1, &htim1, TIM_CHANNEL_1,
    },
    {
        &pwm_led4, "pwmled4",
        GPIOC, GPIO_PIN_9, GPIO_AF2_TIM1, &htim1, TIM_CHANNEL_2,
    },
    {
        &pwm_adc_in, "adc_in",
        GPIOC, GPIO_PIN_1, GPIO_AF2_TIM15, &htim15, TIM_CHANNEL_1,
    },
};

static const eio_pwm_ops_t pwm_driver_ops =
{
    .init = _init,
    .set_duty = _set_duty,
};

/* public functions --------------------------------------------------------- */
static void driver_pwm_init(void)
{
    __HAL_RCC_TIM1_CLK_ENABLE();
    __HAL_RCC_TIM15_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    htim1.Instance = TIM1;
    htim1.Init.Prescaler = 63;
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.Period = 999;
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0;
    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_PWM_Init(&htim1);

    htim15.Instance = TIM15;
    htim15.Init.Prescaler = 63;
    htim15.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim15.Init.Period = 999;
    htim15.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim15.Init.RepetitionCounter = 0;
    htim15.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_PWM_Init(&htim15);

    for (uint32_t i = 0;
            i < sizeof(pwm_driver_data) / sizeof(eio_pwm_data_t); i ++)
    {
        /* Device registering. */
        eio_pwm_register(pwm_driver_data[i].device,
                            pwm_driver_data[i].name,
                            &pwm_driver_ops,
                            &pwm_driver_data[i]);
    }
}
INIT_EXPORT(driver_pwm_init, 1);

/* private functions -------------------------------------------------------- */
static void _init(eio_pwm_t * const me)
{
    eio_pwm_data_t *data = me->super.attr.user_data;

    
    _timer_pwm_init(data);
    _gpio_pwm_init(data);

    HAL_TIM_PWM_Start(data->htim, data->channel);
}

static void _set_duty(eio_pwm_t * const me, uint8_t duty)
{
    eio_pwm_data_t *data = me->super.attr.user_data;

    HAL_TIM_PWM_Stop(data->htim, data->channel);
    
    TIM_OC_InitTypeDef sConfigOC = {0};
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = duty * 10;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    HAL_TIM_PWM_ConfigChannel(data->htim, &sConfigOC, data->channel);

    HAL_TIM_PWM_Start(data->htim, data->channel);
}

static void _timer_pwm_init(eio_pwm_data_t *data)
{
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};
    TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    HAL_TIMEx_MasterConfigSynchronization(data->htim, &sMasterConfig);

    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    HAL_TIM_PWM_ConfigChannel(data->htim, &sConfigOC, data->channel);

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
    HAL_TIMEx_ConfigBreakDeadTime(data->htim, &sBreakDeadTimeConfig);
}

static void _gpio_pwm_init(eio_pwm_data_t *data)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = data->pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = data->alternate;
    HAL_GPIO_Init(data->gpio_x, &GPIO_InitStruct);
}

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
