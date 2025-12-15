mkdir build

arm-linux-gnueabihf-gcc -std=gnu99 -g \
main.c \
../../elab/common/*.c \
../../elab/export/*.c \
../../elab/test/test_elog.c \
../../elab/test/test_preemption_linux.c \
../../elab/3rd/Shell/*.c \
../../elab/os/posix/cmsis_os.c \
../../elab/port/posix/elab_port.c \
-I ../.. \
-I . \
-o build/shell \
-l pthread
