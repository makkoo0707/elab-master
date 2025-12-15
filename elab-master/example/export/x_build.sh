mkdir build

gcc -std=gnu99 -g \
main.c \
../../elab/common/*.c \
../../elab/os/posix/cmsis_os.c \
-I ../.. \
-I . \
-o build/export \
-l pthread
