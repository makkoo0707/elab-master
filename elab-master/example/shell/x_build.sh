mkdir build

gcc -std=gnu99 -g \
main.c \
../../elab/common/*.c \
../../elab/export/*.c \
../../elab/test/test_elog.c \
../../elab/3rd/Shell/*.c \
../../elab/os/posix/cmsis_os.c \
-I ../.. \
-I . \
-o build/shell \
-l pthread
