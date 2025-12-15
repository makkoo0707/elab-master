mkdir build

arm-linux-gnueabihf-gcc -std=gnu99 -g \
main.c \
elab_port.c \
modbus_export.c \
elab_mb_timer.c \
elab_mb_data.c \
elab_mb_bsp.c \
elab_mb_cfg.c \
../../eLab/common/elab_log.c \
../../eLab/elib/elib_hash_table.c \
../../eLab/elib/elib_hash.c \
../../eLab/RTOS/CMSIS_OS_POSIX/cmsis_os.c \
../../eLab/3rd/modbus/mb_os.c \
../../eLab/3rd/modbus/mb_util.c \
../../eLab/3rd/modbus/mb.c \
../../eLab/3rd/modbus/mbm_core.c \
../../eLab/3rd/modbus/mbs_core.c \
../../eLab/edev/elab_rs485.c \
../../plib/elab_iof/elab_iof.c \
../../plib/elab_iof/elab_serial.c \
../../plib/elab_iof/elab_serial_null.c \
../../plib/elab_iof/elab_rtc.c \
../../plib/elab_iof/elab_watchdog.c \
../../plib/elab_iof/elab_utils.c \
../../plib/elab_iof/elab_spi_core.c \
../../plib/elab_iof/elab_spi_dev.c \
../../plib/elab_iof/elab_pin.c \
../../plib/elab_iof/elab_i2c_core.c \
../../plib/elab_iof/elab_i2c_dev.c \
../../plib/simulator/simu_io_driver_serial.c \
../../plib/simulator/simu_io_driver_pin.c \
-I ../../eLab/RTOS/CMSIS_OS_POSIX \
-I ../../eLab/common \
-I ../../eLab/edev \
-I ../../eLab/elib \
-I ../../plib/elab_iof \
-I ../../eLab/3rd/modbus \
-I ../../plib/simulator \
-I . \
-o build/modbus_test \
-l pthread
