
/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* include ------------------------------------------------------------------ */
#include <stdlib.h>
#include "shell.h"
#include "elab_assert.h"
#include "elab_export.h"
#include "elab_common.h"
#include "cmsis_os.h"
#include "elab_assert.h"
#include "esh.h"

ELAB_TAG("ShellExport");

/* private defines ---------------------------------------------------------- */
#define ELAB_SHELL_BUFF_SIZE                    (2048)

typedef struct esh_key_info
{
    uint8_t key_id;
    uint8_t count;
    uint8_t value[5];
} esh_key_info_t;

/* private variables -------------------------------------------------------- */
static const osThreadAttr_t thread_sh_attr =
{
    .name = "shell",
    .stack_size = 2048,
    .priority = (osPriority_t)osPriorityBelowNormal,
};

static Shell shell;
static uint8_t buff_shell[ELAB_SHELL_BUFF_SIZE];

static const esh_key_info_t fkey_value_table[] =
{
    { Esh_Null,      0, { 0x00, 0x00, 0x00, 0x00, 0x00 }, },

    { Esh_F1,        3, { 0x1B, 0x4F, 0x50, 0x00, 0x00 }, },
    { Esh_F2,        3, { 0x1B, 0x4F, 0x51, 0x00, 0x00 }, },
    { Esh_F3,        3, { 0x1B, 0x4F, 0x52, 0x00, 0x00 }, },
    { Esh_F4,        3, { 0x1B, 0x4F, 0x53, 0x00, 0x00 }, },

    { Esh_F5,        5, { 0x1B, 0x5B, 0x31, 0x35, 0x7E }, },
    { Esh_F6,        5, { 0x1B, 0x5B, 0x31, 0x37, 0x7E }, },
    { Esh_F7,        5, { 0x1B, 0x5B, 0x31, 0x38, 0x7E }, },
    { Esh_F8,        5, { 0x1B, 0x5B, 0x31, 0x39, 0x7E }, },

    { Esh_F9,        5, { 0x1B, 0x5B, 0x32, 0x30, 0x7E }, },
    { Esh_F10,       5, { 0x1B, 0x5B, 0x32, 0x31, 0x7E }, },
    { Esh_F12,       5, { 0x1B, 0x5B, 0x32, 0x34, 0x7E }, },

    { Esh_Up,        3, { 0x1B, 0x5B, 0x41, 0x00, 0x00 }, },
    { Esh_Down,      3, { 0x1B, 0x5B, 0x42, 0x00, 0x00 }, },
    { Esh_Right,     3, { 0x1B, 0x5B, 0x43, 0x00, 0x00 }, },
    { Esh_Left,      3, { 0x1B, 0x5B, 0x44, 0x00, 0x00 }, },

    { Esh_Home,      4, { 0x1B, 0x5B, 0x31, 0x7E, 0x00 }, },
    { Esh_Insert,    4, { 0x1B, 0x5B, 0x32, 0x7E, 0x00 }, },
    { Esh_Delect,    4, { 0x1B, 0x5B, 0x33, 0x7E, 0x00 }, },
    { Esh_End,       4, { 0x1B, 0x5B, 0x34, 0x7E, 0x00 }, },
    { Esh_PageUp,    4, { 0x1B, 0x5B, 0x35, 0x7E, 0x00 }, },
    { Esh_PageDown,  4, { 0x1B, 0x5B, 0x36, 0x7E, 0x00 }, },
};

/* private function prototype ----------------------------------------------- */
static uint8_t _func_key_parser(uint8_t *key, uint8_t count);

/* public function ---------------------------------------------------------- */
/**
  * @brief  Shell exporting fucntion
  * @param  None
  * @retval None
  */
static void shell_init(void)
{
    esh_init(_func_key_parser);
    
    shell.read = (int16_t (*)(char *, uint16_t))elab_debug_uart_receive;
    shell.write = (int16_t (*)(char *, uint16_t))elab_debug_uart_send;
    shellInit(&shell, buff_shell, ELAB_SHELL_BUFF_SIZE);
    shell.backsapceMode = SHELL_BACKSPACE_MODE_BACK_SPACE_BACK;

    osThreadId_t thread = osThreadNew(shellTask, (void*)&shell, &thread_sh_attr);
    elab_assert(thread != NULL);
}

/**
  * @brief  Export pin device board level init and exit function
  */
INIT_IO_DRIVER_EXPORT(shell_init);

/* private function --------------------------------------------------------- */

static uint8_t _func_key_parser(uint8_t *key, uint8_t count)
{
    uint8_t _fkey_id = Esh_Null;

    for (uint32_t i = 0;
            i < sizeof(fkey_value_table) / sizeof(esh_key_info_t);
            i ++)
    {
        bool matched = true;
        for (uint32_t k = 0; k < fkey_value_table[i].count; k ++)
        {
            if (fkey_value_table[i].value[k] != key[k])
            {
                printf("fkey_value_table[i].value[k]: %u, value[k]: %u.\n",
                        fkey_value_table[i].value[k], key[k]);
                matched = false;
                break;
            }
        }

        if (matched)
        {
            _fkey_id = fkey_value_table[i].key_id;
        }
    }

    return Esh_Null;
}

/* ----------------------------- end of file -------------------------------- */
