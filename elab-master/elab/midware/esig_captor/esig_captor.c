
/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include "esig_captor.h"
#include "event_def.h"
#include "../../common/elab_common.h"
#include "../../common/elab_assert.h"

Q_DEFINE_THIS_MODULE("ESigCaptor")
ELAB_TAG("ESigCaptor");

#ifdef __cplusplus
extern "C" {
#endif

/* private state function prototypes -----------------------------------------*/
static QState _state_initial(esig_captor_t * const me, void const * const par);
static QState _state_work(esig_captor_t * const me, QEvt const * const e);

/* public functions --------------------------------------------------------- */
void esig_captor_init(esig_captor_t *const me, uint32_t capacity,
                        uint32_t esig_min, uint32_t esig_max)
{
    me->esig_max = esig_max;
    me->esig_min = esig_min;
    me->sem = osSemaphoreNew(1, 0, NULL);
    elab_assert(me->sem != NULL);

    void *memory = elab_malloc(sizeof(QSignal) * capacity);
    elab_assert(memory != NULL);

    elib_queue_init(&me->q, memory, capacity * sizeof(QSignal));
    QActive_ctor(&me->super, Q_STATE_CAST(&_state_initial));

    me->e_queue = elab_malloc(sizeof(QEvt *) * capacity);
    elab_assert(me->e_queue != NULL);

    QACTIVE_START(&me->super,
                    osPriorityRealtime,                  /* QP priority */
                    (QEvt const **)me->e_queue,
                    capacity,                       /* evt queue */
                    (void *)me->stack, 1024U,       /* no per-thread stack */
                    (QEvt *)0);                     /* no initialization event */
}

void esig_captor_deinit(esig_captor_t *const me)
{
    osSemaphoreDelete(me->sem);
    elab_free(me->q.buffer);
    QActive_stop(&me->super);
}

uint32_t esig_captor_get_event_count(esig_captor_t *const me)
{
    uint32_t size_total = (me->q.capacity - me->q.size_free);
    elab_assert((size_total % sizeof(QSignal)) == 0);

    return (size_total / sizeof(QSignal));
}

uint32_t esig_captor_pop(esig_captor_t *const me)
{
    QSignal e_sig = Q_NULL_SIG;

    if (!elib_queue_is_empty(&me->q))
    {
        elib_queue_pull_pop(&me->q, &e_sig, sizeof(QSignal));
    }

    return (uint32_t)e_sig;
}

void esig_captor_wait(esig_captor_t *const me, uint32_t e_sig)
{
    /* TODO */
}

uint32_t esig_captor_sub(esig_captor_t *const me, uint32_t e_sig)
{
    elab_assert(e_sig >= me->esig_min && e_sig <= me->esig_max);

    QActive_subscribe(&me->super, e_sig);
}

uint32_t esig_captor_unsub(esig_captor_t *const me, uint32_t e_sig)
{
    elab_assert(e_sig >= me->esig_min && e_sig <= me->esig_max);

    QActive_unsubscribe(&me->super, e_sig);
}

/* private state function ----------------------------------------------------*/
static QState _state_initial(esig_captor_t * const me, void const * const par)
{
    (void)par;
    (void)me;

    return Q_TRAN(&_state_work);
}

static QState _state_work(esig_captor_t * const me, QEvt const * const e)
{
    QState _status = Q_SUPER(&QHsm_top);

    if (e->sig >= me->esig_min && e->sig <= me->esig_max)
    {
        if (!elib_queue_is_full(&me->q))
        {
            elib_queue_push(&me->q, (void *)&e->sig, sizeof(QSignal));
        }
        _status = Q_HANDLED();
    }

    return _status;
}

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
