/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include "elab_common.h"
#include "elib_queue.h"
#include "stm32g0xx_hal.h"

#define ELAB_DEBUG_UART_BUFFER_TX               (256)
#define ELAB_DEBUG_UART_BUFFER_RX               (16)

UART_HandleTypeDef huart4;

static elib_queue_t queue_rx;
static uint8_t buffer_rx[ELAB_DEBUG_UART_BUFFER_RX];
static elib_queue_t queue_tx;
static uint8_t buffer_tx[ELAB_DEBUG_UART_BUFFER_TX];
static uint8_t byte_recv;

/* public functions --------------------------------------------------------- */
/**
  * @brief  Initialize the elab debug uart.
  * @param  buffer  this pointer
  * @retval Free size.
  */
void elab_debug_uart_init(uint32_t baudrate)
{
    huart4.Instance = USART4;
    huart4.Init.BaudRate = baudrate;
    huart4.Init.WordLength = UART_WORDLENGTH_8B;
    huart4.Init.StopBits = UART_STOPBITS_1;
    huart4.Init.Parity = UART_PARITY_NONE;
    huart4.Init.Mode = UART_MODE_TX_RX;
    huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart4.Init.OverSampling = UART_OVERSAMPLING_16;
    huart4.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart4.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    huart4.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

    HAL_UART_Init(&huart4);
    HAL_UART_Receive_IT(&huart4, &byte_recv, 1);
    
    elib_queue_init(&queue_rx, buffer_rx, ELAB_DEBUG_UART_BUFFER_RX);
    elib_queue_init(&queue_tx, buffer_tx, ELAB_DEBUG_UART_BUFFER_TX);
}

/**
  * @brief  Send data to the debug uart.
  * @param  buffer  this pointer
  * @retval Free size.
  */
int16_t elab_debug_uart_send(void *buffer, uint16_t size)
{
    int16_t ret = 0;
    uint8_t byte = 0;

    HAL_NVIC_DisableIRQ(USART3_4_IRQn);
    if (elib_queue_is_empty(&queue_tx))
    {
        ret = elib_queue_push(&queue_tx, buffer, size);
        if (elib_queue_pull(&queue_tx, &byte, 1) == 1)
        {
            HAL_UART_Transmit_IT(&huart4, &byte, 1);
        }
    }
    else
    {
        ret = elib_queue_push(&queue_tx, buffer, size);
    }
    HAL_NVIC_EnableIRQ(USART3_4_IRQn);

    return ret;
}

/**
  * @brief  Initialize the elab debug uart.
  * @param  buffer  this pointer
  * @retval Free size.
  */
int16_t elab_debug_uart_receive(void *buffer, uint16_t size)
{
    int16_t ret = 0;

    HAL_NVIC_DisableIRQ(USART3_4_IRQn);
    ret = elib_queue_pull_pop(&queue_rx, buffer, size);
    HAL_NVIC_EnableIRQ(USART3_4_IRQn);

    return ret;
}

/**
  * @brief  Clear buffer of the elab debug uart.
  * @param  buffer  this pointer
  * @retval Free size.
  */
void elab_debug_uart_buffer_clear(void)
{
    HAL_NVIC_DisableIRQ(USART3_4_IRQn);

    elib_queue_clear(&queue_rx);
    elib_queue_clear(&queue_tx);

    HAL_NVIC_EnableIRQ(USART3_4_IRQn);
}

/* private functions -------------------------------------------------------- */
/**
  * @brief  The weak UART tx callback function in HAL library.
  * @param  uartHandle  UART handle.
  * @retval None.
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle)
{
    uint8_t byte = 0;

    if (UartHandle->Instance == USART4)
    {
        elib_queue_pop(&queue_tx, 1);
        if (elib_queue_pull(&queue_tx, &byte, 1))
        {
            HAL_UART_Transmit_IT(&huart4, &byte, 1);
        }
    }
}

/**
  * @brief  The weak UART rx callback function in HAL library.
  * @param  uartHandle  UART handle.
  * @retval None.
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
    if (UartHandle->Instance == USART4)
    {
        HAL_UART_Receive_IT(&huart4, &byte_recv, 1);
        elib_queue_push(&queue_rx, &byte_recv, 1);
    }
}

/**
  * @brief  The weak UART initialization function in HAL library.
  * @param  uartHandle  UART handle.
  * @retval None
  */
void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (uartHandle->Instance == USART4)
    {
        /* USART4 clock enable */
        __HAL_RCC_USART4_CLK_ENABLE();
        __HAL_RCC_GPIOC_CLK_ENABLE();

        /** USART4 GPIO Configuration
            PC11     ------> USART4_RX
            PC10     ------> USART4_TX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF1_USART4;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        /* USART4 interrupt Init */
        HAL_NVIC_SetPriority(USART3_4_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(USART3_4_IRQn);
    }
}

/* ----------------------------- end of file -------------------------------- */
