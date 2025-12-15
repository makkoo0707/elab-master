md build

gcc -std=c99 -g ^
*.c ^
..\..\elab\common\*.c ^
..\..\elab\os\win32\cmsis_os_cb.c ^
..\..\elab\os\win32\cmsis_os.c ^
..\..\elab\3rd\FreeRTOS\*.c ^
..\..\elab\3rd\FreeRTOS\portable\MemMang\heap_4.c ^
..\..\elab\3rd\FreeRTOS\portable\mingw\port.c ^
..\..\elab\3rd\qpc\src\qf\*.c ^
..\..\elab\3rd\qpc\ports\freertos\qf_port.c ^
..\..\elab\3rd\qpc\qpc_export.c ^
-I ..\.. ^
-I . ^
-o build\qpc_cmsis_os ^
-l winmm
