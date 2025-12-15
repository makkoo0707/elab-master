
/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include "../driver/drv_pin.h"
#include "elab/common/elab_export.h"
#include "export.h"

/* private variables -------------------------------------------------------- */
static elab_pin_driver_t pin_mcu_led1_c08;
static elab_pin_driver_t pin_mcu_led2_c09;
static elab_pin_driver_t pin_mcu_oled_cs_a02;
static elab_pin_driver_t pin_mcu_oled_dc_a03;
static elab_pin_driver_t pin_mcu_oled_rst_a04;
static elab_pin_driver_t pin_mcu_flash_cs_d02;
static elab_pin_driver_t pin_mcu_button_stop_a15;

static elab_pin_driver_t pin_i2c1_38_2_led3;
static elab_pin_driver_t pin_i2c1_38_3_led4;
static elab_pin_driver_t pin_i2c1_38_4_button;

/* public functions --------------------------------------------------------- */
static void driver_pin_mcu_export(void)
{
    /* PIN devices on MCU. */
    elab_driver_pin_init(&pin_mcu_led1_c08, "pin_led1", "C.08");
    elab_pin_set_mode(&pin_mcu_led1_c08.device.super, PIN_MODE_OUTPUT_PP);

    elab_driver_pin_init(&pin_mcu_led2_c09, "pin_led2", "C.09");
    elab_pin_set_mode(&pin_mcu_led2_c09.device.super, PIN_MODE_OUTPUT_PP);

    elab_driver_pin_init(&pin_mcu_oled_cs_a02, "pin_oled_cs", "A.02");
    elab_pin_set_mode(&pin_mcu_oled_cs_a02.device.super, PIN_MODE_OUTPUT_PP);

    elab_driver_pin_init(&pin_mcu_oled_dc_a03, "pin_oled_dc", "A.03");
    elab_pin_set_mode(&pin_mcu_oled_dc_a03.device.super, PIN_MODE_OUTPUT_PP);

    elab_driver_pin_init(&pin_mcu_oled_rst_a04, "pin_oled_rst", "A.04");
    elab_pin_set_mode(&pin_mcu_oled_rst_a04.device.super, PIN_MODE_OUTPUT_PP);

    elab_driver_pin_init(&pin_mcu_flash_cs_d02, "pin_flash_cs", "D.02");
    elab_pin_set_mode(&pin_mcu_flash_cs_d02.device.super, PIN_MODE_OUTPUT_PP);

    elab_driver_pin_init(&pin_mcu_button_stop_a15, "pin_button_stop", "A.15");
    elab_pin_set_mode(&pin_mcu_button_stop_a15.device.super, PIN_MODE_INPUT);
    
}
INIT_EXPORT(driver_pin_mcu_export, EXPORT_LEVEL_PIN_MCU);

static void driver_pin_i2c_export(void)
{
    /* PIN devices based on I2C. */
    elab_driver_pin_i2c_init(&pin_i2c1_38_2_led3, "pin_led3", "i2c_exio.02");
    elab_pin_set_mode(&pin_i2c1_38_2_led3.device.super, PIN_MODE_OUTPUT_PP);

    elab_driver_pin_i2c_init(&pin_i2c1_38_3_led4, "pin_led4", "i2c_exio.03");
    elab_pin_set_mode(&pin_i2c1_38_3_led4.device.super, PIN_MODE_OUTPUT_PP);

    elab_driver_pin_i2c_init(&pin_i2c1_38_4_button, "pin_button_start", "i2c_exio.04");
    elab_pin_set_mode(&pin_i2c1_38_4_button.device.super, PIN_MODE_INPUT);
}
INIT_EXPORT(driver_pin_i2c_export, EXPORT_LEVEL_PIN_I2C);

/* ----------------------------- end of file -------------------------------- */
