/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include <stdint.h>
#include <stdio.h>
#include "cmsis_os.h"
#include "simu_driver_serial.h"
#include "simu_driver_pin.h"
#include "elab_device.h"
#include "elab_pin.h"
#include "elab_assert.h"
#include "elab_log.h"

ELAB_TAG("main");

/* private variables ---------------------------------------------------------*/
/**
 * @brief  The thread attribute for testing.
 */
static const osThreadAttr_t thread_attr_mb = 
{
    .name = "ThreadTest",
    .attr_bits = osThreadDetached,
    .priority = osPriorityNormal,
    .stack_size = 2048,
};

/* public functions --------------------------------------------------------- */
/**
  * @brief  The main function.
  */
int main(int32_t argc, char **argv)
{
    elab_err_t ret = ELAB_OK;

    simu_serial_new("serial_test");
    elab_device_t *serial = elab_device_find("serial_test");
    elab_assert(serial != NULL);
    elab_device_open(serial);

    const char *str_test = "Hello serial!";
    char buff_rx[256];
    memset(buff_rx, 0, 256);
    simu_serial_make_rx_data("serial_test", (void *)str_test, strlen(str_test));

    int32_t ret_read = elab_device_read(serial, 0, buff_rx, strlen(str_test));
    elog_debug("Serail rx: %u %s.", ret_read, buff_rx);

    simu_io_new("P2.1", false);
    elab_device_t *pin_id = elab_device_find("P2.1");
    elab_pin_set_mode(pin_id, PIN_MODE_INPUT);

    elog_debug("PIN %s status: %u.", "P2.1", elab_pin_get_status(pin_id));
    simu_in_set_status("P2.1", true);
    elog_debug("PIN %s status: %u.", "P2.1", elab_pin_get_status(pin_id));
    simu_in_set_status("P2.1", false);
    elog_debug("PIN %s status: %u.", "P2.1", elab_pin_get_status(pin_id));

    return 0;
}

/* ----------------------------- end of file -------------------------------- */
