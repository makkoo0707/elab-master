
/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "../3rd/Shell/shell.h"
#include "../common/elab_log.h"

ELAB_TAG("eLogTest");

#ifdef __cplusplus
extern "C" {
#endif

/* private functions -------------------------------------------------------- */
/**
  * @brief Testing function for eLog module.
  * @retval None
  */
static int32_t test_elog(int32_t argc, char *argv[])
{
    (void)argc;
    (void)argv;
    
    char ch = 'A';
    char *str = "String Test";
    uint32_t value_u32 = 12345;
    int32_t value_s32 = -12345;
    float value_float = 1.2345;

    printf("elog printf test. %s, %u, %d, %f, %c.\r\n",
            str, value_u32, value_s32, value_float, ch);
    elog_debug("elog debug level test. %s, %u, %d, %f, %c.",
                str, value_u32, value_s32, value_float, ch);
    elog_info("elog info level test. %s, %u, %d, %f, %c.",
                str, value_u32, value_s32, value_float, ch);
    elog_warn("elog warn level test. %s, %u, %d, %f, %c.",
                str, value_u32, value_s32, value_float, ch);
    elog_error("elog error level test. %s, %u, %d, %f, %c.",
                str, value_u32, value_s32, value_float, ch);

    return 0;
}

/**
  * @brief  Test shell command export
  */
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                    test_elog,
                    test_elog,
                    eLog testing function);

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
