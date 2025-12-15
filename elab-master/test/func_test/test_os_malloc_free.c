/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdio.h>
#include "elab/common/elab_common.h"
#include "elab/os/cmsis_os.h"
#include "elab/3rd/Shell/shell.h"
#include "elab/common/elab_log.h"

ELAB_TAG("Test_OS_Malloc_Free");

/* private functions ---------------------------------------------------------*/
/**
  * @brief  Malloc & free testing.
  * @param  None
  * @retval None
  */
static int32_t test_malloc_free(int32_t argc, char *argv[])
{
    if (argc != 2)
    {
        elog_error("test_malloc_free Wrong argc. Actual: %d, expected: 2.",
                    argc);
        return -1;
    }

    int32_t malloc_size = atoi(argv[1]);
    if (malloc_size <= 0)
    {
        elog_error("test_malloc_free Wrong malloc size %d.\n", malloc_size);
        return -2;
    }

    void *memory = elab_malloc(malloc_size);
    if (memory == NULL)
    {
        elog_error("Malloc size %d bytes fails.\n", malloc_size);
        return -3;
    }

    elab_free(memory);

    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                    test_malloc_free,
                    test_malloc_free,
                    testing for elab malloc & free functions);

/* ----------------------------- end of file -------------------------------- */
