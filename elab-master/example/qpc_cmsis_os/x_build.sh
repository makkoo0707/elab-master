mkdir build

gcc -std=gnu99 -g \
*.c \
../../elab/common/*.c \
../../elab/os/posix/cmsis_os.c \
../../elab/3rd/qpc/src/qf/*.c \
../../elab/3rd/qpc/ports/posix/qf_port.c \
../../elab/3rd/qpc/qpc_export.c \
-I ../.. \
-I . \
-o build/qpc_cmsis_os \
-l pthread
