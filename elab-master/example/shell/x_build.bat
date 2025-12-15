md build

gcc -std=c99 -g ^
*.c ^
..\..\elab\export\*.c ^
..\..\elab\common\*.c ^
..\..\elab\test\test_elog.c ^
..\..\elab\os\win32\cmsis_os_cb.c ^
..\..\elab\os\win32\cmsis_os.c ^
..\..\elab\3rd\FreeRTOS\*.c ^
..\..\elab\3rd\FreeRTOS\portable\MemMang\heap_4.c ^
..\..\elab\3rd\FreeRTOS\portable\mingw\port.c ^
..\..\elab\3rd\Shell\*.c ^
-I ..\.. ^
-I . ^
-o build\shell ^
-l winmm