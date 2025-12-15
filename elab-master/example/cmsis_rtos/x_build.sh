mkdir build

gcc -std=gnu99 -g \
*.c \
../../elab/common/*.c \
../../elab/os/posix/cmsis_os.c \
-I ../.. \
-I . \
-o build/cmsis_os \
-l pthread
