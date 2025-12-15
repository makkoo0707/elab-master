
#if defined (_WIN32) || defined(__linux__)

#include "mqtt_platform.h"
#include "stdlib.h"

void *platform_memory_alloc(size_t size)
{
    return malloc(size);
}

void *platform_memory_calloc(size_t num, size_t size)
{
    return calloc(num, size);
}

void platform_memory_free(void *ptr)
{
    free(ptr);
}

#endif
