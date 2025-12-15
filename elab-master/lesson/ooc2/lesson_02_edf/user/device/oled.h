/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef OLED_H
#define OLED_H

/* includes ----------------------------------------------------------------- */
#include <stdint.h>

/* public defines ----------------------------------------------------------- */
enum
{
    OLED_CMD_UP = 0,
    OLED_CMD_DOWN,
    OLED_CMD_LEFT,
    OLED_CMD_RIGHT,
};

/* public functions --------------------------------------------------------- */
void oled_open(void);
void oled_close(void);
void oled_clear(void);
void oled_set_value(uint8_t x, uint8_t y, uint8_t value);

void oled_game_start(void);
void oled_game_stop(void);
void oled_game_execute(uint8_t cmd);

#endif

/* ----------------------------- end of file -------------------------------- */
