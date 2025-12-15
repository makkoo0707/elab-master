#include "oled.h"
#include "elab/edf/normal/elab_spi.h"
#include "elab/edf/normal/elab_pin.h"
#include "elab/os/cmsis_os.h"
#include "elab/common/elab_assert.h"
#include "elab/common/elab_export.h"
#include "export/export.h"

ELAB_TAG("OLED");

static void _write_byte(uint8_t byte, bool cmd);
static void _oled_coordinate(int8_t x, int8_t y, bool status);

static elab_device_t *spi = NULL;
static elab_device_t *oled_dc = NULL;
static elab_device_t *oled_rst = NULL;
int8_t pos_x = 0;
int8_t pos_y = 0;

void oled_init(void)
{
    spi = elab_device_find("spi_oled");
    elab_assert(spi != NULL);

    oled_dc = elab_device_find("pin_oled_dc");
    elab_assert(oled_dc != NULL);
    elab_pin_set_mode(oled_dc, PIN_MODE_OUTPUT_OD);

    oled_rst = elab_device_find("pin_oled_rst");
    elab_assert(oled_rst != NULL);
    elab_pin_set_mode(oled_rst, PIN_MODE_OUTPUT_OD);

    osDelay(100);

    elab_pin_set_status(oled_rst, false);
    osDelay(10);
    elab_pin_set_status(oled_rst, true);

    _write_byte(0xAE, true);
    _write_byte(0x00, true);    //---set low column address
    _write_byte(0x10, true);    //---set high column address
    _write_byte(0x40, true);    //--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
    _write_byte(0x81, true);    //--set contrast control register
    _write_byte(0xCF, true);    // Set SEG Output Current Brightness
    _write_byte(0xA1, true);    //--Set SEG/Column Mapping     0xa0左右反置 0xa1正常
    _write_byte(0xC8, true);    //Set COM/Row Scan Direction   0xc0上下反置 0xc8正常
    _write_byte(0xA6, true);    //--set normal display
    _write_byte(0xA8, true);    //--set multiplex ratio(1 to 64)
    _write_byte(0x3f, true);    //--1/64 duty
    _write_byte(0xD3, true);    //-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
    _write_byte(0x00, true);    //-not offset
    _write_byte(0xd5, true);    //--set display clock divide ratio/oscillator frequency
    _write_byte(0x80, true);    //--set divide ratio, Set Clock as 100 Frames/Sec
    _write_byte(0xD9, true);    //--set pre-charge period
    _write_byte(0xF1, true);    //Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
    _write_byte(0xDA, true);    // --set com pins hardware configuration
    _write_byte(0x12, true);
    _write_byte(0xDB, true);    // --set vcomh
    _write_byte(0x40, true);    // Set VCOM Deselect Level
    _write_byte(0x20, true);    // -Set Page Addressing Mode (0x00/0x01/0x02)
    _write_byte(0x02, true);    //
    _write_byte(0x8D, true);    // --set Charge Pump enable/disable
    _write_byte(0x14, true);    // --set(0x10) disable
    _write_byte(0xA4, true);    // Disable Entire Display On (0xa4/0xa5)
    _write_byte(0xA6, true);    // Disable Inverse Display On (0xa6/a7) 
    _write_byte(0xAF, true);    // --turn on oled panel
    
    oled_clear();
}
INIT_EXPORT(oled_init, EXPORT_LEVEL_OLED);

void oled_open(void)
{
    _write_byte(0X8D, true);    // Set DCDC命令
    _write_byte(0X14, true);    // DCDC ON
    _write_byte(0XAF, true);    // DISPLAY ON
}

void oled_close(void)
{
    _write_byte(0X8D, true);    // Set DCDC命令
    _write_byte(0X10, true);    // DCDC ON
    _write_byte(0XAE, true);    // DISPLAY OFF
}

void oled_clear(void)
{
    for (uint8_t i = 0; i < 8; i ++)
    {  
        _write_byte(0xb0 + i, true);        // 设置页地址（0~7）
        _write_byte(0x00, true);            // 设置显示位置—列低地址
        _write_byte(0x10, true);            // 设置显示位置—列高地址
        for (uint8_t n = 0; n < 128; n ++)
        {
            _write_byte(0, false);
        }
    }
}

void oled_set_value(uint8_t x, uint8_t y, uint8_t value)
{
    _write_byte((0xb0 + y), true);
    _write_byte(((x & 0xf0) >> 4) | 0x10, true);
    _write_byte((x & 0x0f) | 0x01, true);
    _write_byte(value, false);
}

static void _write_byte(uint8_t byte, bool cmd)
{
    elab_pin_set_status(oled_dc, cmd ? false : true);
    elab_spi_send(spi, &byte, 1, 100);
    elab_pin_set_status(oled_dc, true);
}

void oled_game_start(void)
{
    oled_clear();
    pos_x = 0;
    pos_y = 0;
    _oled_coordinate(0, 0, true);
}

void oled_game_stop(void)
{
    oled_clear();
}

void oled_game_execute(uint8_t cmd)
{
    int8_t pos_x_backup = pos_x;
    int8_t pos_y_backup = pos_y;

    switch (cmd)
    {
        case OLED_CMD_UP:
            pos_y += 8;
            pos_y = pos_y > 32 ? 32 : pos_y;
            break;

        case OLED_CMD_DOWN:
            pos_y -= 8;
            pos_y = pos_y < -32 ? -32 : pos_y;
            break;

        case OLED_CMD_LEFT:
            pos_x -= 8;
            pos_x = pos_x < -64 ? -64 : pos_x;
            break;

        case OLED_CMD_RIGHT:
            pos_x += 8;
            pos_x = pos_x > 56 ? 56 : pos_x;
            break;
    }

    if (pos_x_backup != pos_x || pos_y_backup != pos_y)
    {
        _oled_coordinate(pos_x_backup, pos_y_backup, false);
        _oled_coordinate(pos_x, pos_y, true);
    }
}

static void _oled_coordinate(int8_t x, int8_t y, bool status)
{
    x += 64;
    y = 32 - y;

    for (uint32_t i = 0; i < 8; i += 2)
    {
        oled_set_value(x + i, (y / 8), (status ? 0xff : 0x00));
    }
}
