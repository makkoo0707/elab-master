/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef ESIG_CAPTOR_H
#define ESIG_CAPTOR_H

/* include ------------------------------------------------------------------ */
#include "../../elib/elib_queue.h"
#include "../../3rd/qpc/include/qpc.h"
#include "../../os/cmsis_os.h"

#ifdef __cplusplus
extern "C" {
#endif

/* public typedef ----------------------------------------------------------- */
typedef struct esig_captor
{
    QActive super;

    osSemaphoreId_t sem;
    elib_queue_t q;
    uint32_t esig_max;
    uint32_t esig_min;

    uint8_t stack[1024];
    QEvt **e_queue;
} esig_captor_t;

/* public function ---------------------------------------------------------- */
void esig_captor_init(esig_captor_t *const me, uint32_t capacity,
                        uint32_t esig_min, uint32_t esig_max);
void esig_captor_deinit(esig_captor_t *const me);

/* Event signal captor class functions */
uint32_t esig_captor_get_event_count(esig_captor_t *const me);
uint32_t esig_captor_pop(esig_captor_t *const me);
uint32_t esig_captor_sub(esig_captor_t *const me, uint32_t e_sig);
uint32_t esig_captor_unsub(esig_captor_t *const me, uint32_t e_sig);
/* TODO */
void esig_captor_wait(esig_captor_t *const me, uint32_t e_sig);

#ifdef __cplusplus
}
#endif     

#endif  /* ESIG_CAPTOR_H */

/* ----------------------------- end of file -------------------------------- */
