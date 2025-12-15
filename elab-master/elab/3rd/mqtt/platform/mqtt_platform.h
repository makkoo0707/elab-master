
#ifndef __PLATFORM_H_
#define __PLATFORM_H_

// memory ----------------------------------------------------------------------
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#if defined(_WIN32) || defined(__linux__)
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#else
#include "../../../../osal/cmsis_rtos/cmsis_os.h"
#endif

void *platform_memory_alloc(size_t size);
void *platform_memory_calloc(size_t num, size_t size);
void platform_memory_free(void *ptr);

// mutex -----------------------------------------------------------------------
typedef struct platform_mutex {
    pthread_mutex_t mutex;
} platform_mutex_t;

int platform_mutex_init(platform_mutex_t* m);
int platform_mutex_lock(platform_mutex_t* m);
int platform_mutex_trylock(platform_mutex_t* m);
int platform_mutex_unlock(platform_mutex_t* m);
int platform_mutex_destroy(platform_mutex_t* m);

// thread ----------------------------------------------------------------------
typedef struct platform_thread {
    pthread_t thread;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} platform_thread_t;

platform_thread_t *platform_thread_init( const char *name,
                                        void (*entry)(void *),
                                        void * const param,
                                        unsigned int stack_size,
                                        unsigned int priority,
                                        unsigned int tick);
void platform_thread_startup(platform_thread_t* thread);
void platform_thread_stop(platform_thread_t* thread);
void platform_thread_start(platform_thread_t* thread);
void platform_thread_destroy(platform_thread_t* thread);

// timer -----------------------------------------------------------------------
typedef struct platform_timer {
    struct timeval time;
} platform_timer_t;

void platform_timer_init(platform_timer_t* timer);
void platform_timer_cutdown(platform_timer_t* timer, unsigned int timeout);
char platform_timer_is_expired(platform_timer_t* timer);
int platform_timer_remain(platform_timer_t* timer);
unsigned long platform_timer_now(void);
void platform_timer_usleep(unsigned long usec);

// net -------------------------------------------------------------------------
#define PLATFORM_NET_PROTO_TCP  0 /**< The TCP transport protocol */
#define PLATFORM_NET_PROTO_UDP  1 /**< The UDP transport protocol */

int platform_net_socket_connect(const char *host, const char *port, int proto);
int platform_net_socket_recv(int fd, void *buf, size_t len, int flags);
int platform_net_socket_recv_timeout(int fd, unsigned char *buf, int len, int timeout);
int platform_net_socket_write(int fd, void *buf, size_t len);
int platform_net_socket_write_timeout(int fd, unsigned char *buf, int len, int timeout);
int platform_net_socket_close(int fd);
int platform_net_socket_set_block(int fd);
int platform_net_socket_set_nonblock(int fd);
int platform_net_socket_setsockopt(int fd, int level, int optname, const void *optval, uint32_t optlen);

#endif
