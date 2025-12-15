
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

ELAB_TAG("IniTest");

#ifdef __cplusplus
extern "C" {
#endif

/* The testing file (config/config.ini) content is as the following:

; last modified 1 April 2001 by John Doe
[owner]
name = John Doe
organization = Acme Widgets Inc.

[database]
; use IP address in case network name resolution is not working
server = 192.0.2.62
port = 143
file = "payroll.dat"

*/

#if 0
/* private functions -------------------------------------------------------- */
/**
  * @brief Testing function for eLog module.
  * @retval None
  */
static int32_t test_ini(int32_t argc, char *argv[])
{
    (void)argc;
    (void)argv;

    ini_t *config = ini_load("para/config_test.ini");

    const char *name = ini_get(config, "owner", "name");
    if (name)
    {
        printf("name: %s\n", name);
    }
    
    int port = 80;

    ini_sget(config, "database", "port", "%d", &port);
    printf("port: %d.\n", port);

    ini_free(config);

    return 0;
}

/**
  * @brief  Test shell command export
  */
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                    test_ini,
                    test_ini,
                    ini library testing function);

#endif

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
