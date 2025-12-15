mkdir build

gcc -std=gnu99 -g \
main.c \
../../elab/common/*.c \
../../elab/export/*.c \
../../elab/elib/*.c \
../../elab/edf/*.c \
../../elab/edf/user/elab_button.c \
../../elab/edf/driver/simulator/*.c \
../../elab/midware/modbus/*.c \
../../elab/midware/esig_captor/*.c \
../../elab/edf/normal/*.c \
../../elab/unit_test/edf/*.c \
../../elab/unit_test/os/*.c \
../../elab/unit_test/elib/*.c \
../../elab/unit_test/midware/*.c \
../../elab/test/test_elog.c \
../../elab/3rd/Shell/*.c \
../../elab/3rd/mqtt/common/*.c \
../../elab/3rd/mqtt/mqtt/*.c \
../../elab/3rd/mqtt/mqttclient/*.c \
../../elab/3rd/mqtt/network/*.c \
../../elab/3rd/mqtt/platform/*.c \
../../elab/3rd/qpc/src/qf/*.c \
../../elab/3rd/qpc/ports/posix/qf_port.c \
../../elab/3rd/qpc/qpc_export.c \
../../elab/3rd/Unity/*.c \
../../elab/os/posix/cmsis_os.c \
-I ../.. \
-I . \
-o build/shell \
-l pthread
