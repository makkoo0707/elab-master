mkdir build

gcc -std=gnu99 -g \
main.c \
elab_port.c \
../../eLab/common/elab_log.c \
../../eLab/common/elab_assert.c \
../../eLab/elib/elib_hash_table.c \
../../eLab/elib/elib_hash.c \
../../eLab/RTOS/CMSIS_OS_POSIX/cmsis_os.c \
../../eLab/midware/esh/esh.c \
../../eLab/edf/user/elab_rs485.c \
../../eLab/edf/elab_device.c \
../../eLab/edf/elab_device_null.c \
../../eLab/edf/normal/elab_serial.c \
../../eLab/edf/normal/elab_rtc.c \
../../eLab/edf/normal/elab_watchdog.c \
../../eLab/edf/normal/elab_spi.c \
../../eLab/edf/normal/elab_pin.c \
../../eLab/edf/normal/elab_i2c.c \
../../eLab/edf/driver/simulator/simu_driver_serial.c \
../../eLab/edf/driver/simulator/simu_driver_pin.c \
-I ../../eLab/RTOS/CMSIS_OS_POSIX \
-I ../../eLab/common \
-I ../../eLab/elib \
-I ../../eLab/edf/normal \
-I ../../eLab/edf/user \
-I ../../eLab/edf \
-I ../../eLab/edf/driver/simulator \
-I ../../eLab/midware/esh \
-I . \
-o build/simu_serial_test \
-l pthread
