 
/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "elab/common/elab_assert.h"
#include "elab/3rd/Shell/shell.h"
#include "elab/edf/user/elab_led.h"
#include "elab/edf/user/elab_button.h"
#include "elab/edf/normal/elab_adc.h"
#include "elab/edf/normal/elab_pwm.h"
#include "elab/edf/normal/elab_i2c.h"
#include "elab/edf/normal/elab_spi.h"
#include "device/oled.h"

#ifdef __cplusplus
extern "C" {
#endif

ELAB_TAG("TestSelf");

/* private config ----------------------------------------------------------- */
#define TEST_SELF_BUFF_SIZE                     (128)
#define TEST_SELF_EEPROM_DATA_SIZE              (8)

/* private typedef ---------------------------------------------------------- */

/* private function prototype ----------------------------------------------- */
static void _button_cb(elab_button_t *const me, uint8_t event_id);

/* private variables -------------------------------------------------------- */
static bool button_start_en = false;
static bool button_stop_en = false;
static bool button_up_en = false;
static bool button_down_en = false;
static bool button_right_en = false;
static bool button_left_en = false;

static osSemaphoreId_t sem = NULL;
static uint8_t buff_tx[TEST_SELF_BUFF_SIZE];
static uint8_t buff_rx[TEST_SELF_BUFF_SIZE];

/* private functions -------------------------------------------------------- */
/**
  * @brief OLED closing testing function.
  * @retval None
  */
static int32_t test_self(int32_t argc, char *argv[])
{
    int32_t ret = 0;
    elab_device_t *dev = NULL;
    float voltage = 0.0;
    uint16_t flash_id;
    
    if (argc != 1)
    {
        ret = -1;
        goto exit;
    }

    sem = osSemaphoreNew(1, 0, NULL);
    elab_assert(sem != NULL);

    elab_button_set_event_callback(elab_device_find("button_start"), _button_cb);
    elab_button_set_event_callback(elab_device_find("button_stop"), _button_cb);
    elab_button_set_event_callback(elab_device_find("button_right"), _button_cb);
    elab_button_set_event_callback(elab_device_find("button_left"), _button_cb);
    elab_button_set_event_callback(elab_device_find("button_up"), _button_cb);
    elab_button_set_event_callback(elab_device_find("button_down"), _button_cb);

    /* Make all LEDs on-off in different frequency. */
    elab_led_toggle(elab_device_find("led1"), 100);
    elab_led_toggle(elab_device_find("led2"), 200);
    elab_led_toggle(elab_device_find("led3"), 500);
    elab_led_toggle(elab_device_find("led4"), 1000);

    /* PWM DAC output and measure the output volage. */
    printf("Start the PWM setting and voltage measuring:\r\n");
    for (uint32_t i = 1; i <= 100; i ++)
    {
        elab_pwm_set_duty(elab_device_find("pwm_dac"), i);
        voltage = elab_adc_get_value(elab_device_find("adc_pwm"));
        printf("%.3f ", voltage);
        if (i % 10 == 0)
        {
            printf("\r\n");
        }
        osDelay(10);
    }
    elab_pwm_set_duty(elab_device_find("pwm_dac"), 0);
    osDelay(200);

    /* EEPROM testing */
    dev = elab_device_find("i2c_eeprom");
    for (uint8_t i = 0; i < TEST_SELF_EEPROM_DATA_SIZE; i ++)
    {
        buff_tx[i] = (i + 2);
    }
    memset(buff_rx, 0, TEST_SELF_EEPROM_DATA_SIZE);
    elab_i2c_write_memory(dev, 0, buff_tx, TEST_SELF_EEPROM_DATA_SIZE, 100);
    osDelay(200);
    elab_i2c_read_memory(dev, 0, buff_rx, TEST_SELF_EEPROM_DATA_SIZE, 100);
    
    if (memcmp(buff_tx, buff_rx, TEST_SELF_EEPROM_DATA_SIZE) == 0)
    {
        printf("EEPROM testing passed.\r\n");
    }
    for (uint32_t i = 0; i < TEST_SELF_EEPROM_DATA_SIZE; i ++)
    {
        printf("0x%02x ", buff_rx[i]);
    }
    printf("\r\n");

    /* SPI-Flash testing */
    buff_tx[0] = 0x90;                  /* Read Manufacturer / Device ID */
    buff_tx[1] = 0x00;                  /* Data address */
    dev = elab_device_find("spi_flash");
    elab_spi_xfer(dev, buff_tx, buff_rx, 6, 100);

    flash_id = (buff_rx[5] << 8) + buff_rx[4];
    if (flash_id == 0xef14)
    {
        printf("Flash W25Q16 read ID: 0x%04x.\r\n", flash_id);
    }
    else
    {
        elog_error("Flash testing fails.");
    }

    /* Button testing. */
    printf("Buttons testing starts. --------------------------------\r\n");
    /* Start button */
    button_start_en = true;
    printf("Please press the start button!\r\n");
    osSemaphoreAcquire(sem, osWaitForever);
    button_start_en = false;

    /* Up button */
    button_up_en = true;
    printf("Please press the up button!\r\n");
    osSemaphoreAcquire(sem, osWaitForever);
    button_up_en = false;

    /* Down button */
    button_down_en = true;
    printf("Please press the down button!\r\n");
    osSemaphoreAcquire(sem, osWaitForever);
    button_down_en = false;

    /* Left button */
    button_left_en = true;
    printf("Please press the left button!\r\n");
    osSemaphoreAcquire(sem, osWaitForever);
    button_left_en = false;

    /* Right button */
    button_right_en = true;
    printf("Please press the Right button!\r\n");
    osSemaphoreAcquire(sem, osWaitForever);
    button_right_en = false;

    /* Stop button */
    button_stop_en = true;
    printf("Please press the stop button!\r\n");
    osSemaphoreAcquire(sem, osWaitForever);
    button_stop_en = false;
    
    elab_button_set_event_callback(elab_device_find("button_start"), NULL);
    elab_button_set_event_callback(elab_device_find("button_stop"), NULL);
    elab_button_set_event_callback(elab_device_find("button_right"), NULL);
    elab_button_set_event_callback(elab_device_find("button_left"), NULL);
    elab_button_set_event_callback(elab_device_find("button_up"), NULL);
    elab_button_set_event_callback(elab_device_find("button_down"), NULL);

exit:
    return ret;
}

/**
  * @brief  Test shell command export
  */
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                    test_self,
                    test_self,
                    Golden board self-test);

static void _button_cb(elab_button_t *const me, uint8_t event_id)
{
    elab_device_t *dev = ELAB_DEVICE_CAST(me);

    if (event_id == ELAB_BUTTON_EVT_CLICK)
    {
        if (elab_device_of_name(dev, "button_start") && button_start_en)
        {
            osSemaphoreRelease(sem);
        }
        else if (elab_device_of_name(dev, "button_stop") && button_stop_en)
        {
            osSemaphoreRelease(sem);
        }
        else if (elab_device_of_name(dev, "button_up") && button_up_en)
        {
            osSemaphoreRelease(sem);
        }
        else if (elab_device_of_name(dev, "button_down") && button_down_en)
        {
            osSemaphoreRelease(sem);
        }
        else if (elab_device_of_name(dev, "button_right") && button_right_en)
        {
            osSemaphoreRelease(sem);
        }
        else if (elab_device_of_name(dev, "button_left") && button_left_en)
        {
            osSemaphoreRelease(sem);
        }
    }
}

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
