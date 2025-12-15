
/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include <string.h>
#include "drv_adc.h"
#include "drv_util.h"
#include "elab/edf/normal/elab_adc.h"
#include "elab/common/elab_assert.h"
#include "elab/common/elab_def.h"
#include "elab/common/elab_export.h"
#include "stm32g0xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

ELAB_TAG("DriverADC");

/* public typedef ----------------------------------------------------------- */
typedef struct elab_adc_data
{
    const char *pin_name;
    uint32_t channel;
} elab_adc_data_t;

/* private function prototype ----------------------------------------------- */
static uint32_t _get_value(elab_adc_t * const me);
static void _hw_adc1_init(void);

/* private variables -------------------------------------------------------- */
static const elab_adc_ops_t _driver_ops =
{
    .get_value = _get_value,
};

static bool hw_adc1_init_end = false;
static ADC_HandleTypeDef hadc1;

static const elab_adc_data_t adc_data[] =
{
    { "A.00", ADC_CHANNEL_0 },
    { "A.01", ADC_CHANNEL_1 },
};

/* public functions --------------------------------------------------------- */
void elab_driver_adc_init(elab_adc_driver_t *me,
                            const char *name, const char *pin_name)
{
    elab_assert(me != NULL);
    elab_assert(name != NULL);
    elab_assert(pin_name != NULL);
    elab_assert(check_pin_name_valid(pin_name));

    me->pin_name = pin_name;
    elab_adc_register(&me->device, name, &_driver_ops, me);

    /* Enable the clock. */
    gpio_clock_enable(pin_name);

    /* Configure GPIO pin. */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Pin = get_pin_from_name(pin_name);
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(get_port_from_name(pin_name), &GPIO_InitStruct);

    if (!hw_adc1_init_end)
    {
        hw_adc1_init_end = true;
        _hw_adc1_init();
    }
}

static void _hw_adc1_init(void)
{
    HAL_StatusTypeDef ret = HAL_OK;
    
    __HAL_RCC_ADC_CLK_ENABLE();
    
    /** Configure the global features of the ADC (Clock, Resolution, Data 
     *  Alignment and number of conversion.
     */
    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc1.Init.EOCSelection = ADC_EOC_SEQ_CONV;
    hadc1.Init.LowPowerAutoWait = DISABLE;
    hadc1.Init.LowPowerAutoPowerOff = DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.NbrOfConversion = 1;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc1.Init.DMAContinuousRequests = DISABLE;
    hadc1.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
    hadc1.Init.SamplingTimeCommon1 = ADC_SAMPLETIME_1CYCLE_5;
    hadc1.Init.SamplingTimeCommon2 = ADC_SAMPLETIME_1CYCLE_5;
    hadc1.Init.OversamplingMode = DISABLE;
    hadc1.Init.TriggerFrequencyMode = ADC_TRIGGER_FREQ_HIGH;
    ret = HAL_ADC_Init(&hadc1);
    elab_assert(ret == HAL_OK);

    HAL_ADCEx_Calibration_Start(&hadc1);
}

/* private functions -------------------------------------------------------- */
/**
  * @brief  The ADC driver get_value function.
  * @param  me  PIN device handle.
  * @retval ADC value.
  */
static uint32_t _get_value(elab_adc_t * const me)
{
    elab_assert(me != NULL);
    elab_adc_driver_t *driver = (elab_adc_driver_t *)me->super.user_data;
    uint32_t channel = UINT32_MAX;
    for (uint32_t i = 0; i < (sizeof(adc_data) / sizeof(elab_adc_data_t)); i ++)
    {
        if (strcmp(adc_data[i].pin_name, driver->pin_name) == 0)
        {
            channel = adc_data[i].channel;
            break;
        }
    }
    elab_assert(channel != UINT32_MAX);

    HAL_StatusTypeDef ret = HAL_OK;

    /** Configure Regular Channel */
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = channel;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_2;
    ret = HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    elab_assert(ret == HAL_OK);

    ret = HAL_ADC_Start(&hadc1);
    elab_assert(ret == HAL_OK);
    ret = HAL_ADC_PollForConversion(&hadc1, 100);
    elab_assert(ret == HAL_OK);
    uint32_t value = HAL_ADC_GetValue(&hadc1);
    ret = HAL_ADC_Stop(&hadc1);
    elab_assert(ret == HAL_OK);

    return value;
}

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
