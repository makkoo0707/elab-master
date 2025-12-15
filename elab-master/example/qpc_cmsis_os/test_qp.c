/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "event_def.h"
#include "elab/3rd/qpc/include/qpc.h"
#include "elab/os/cmsis_os.h"
#include "elab/common/elab_export.h"

Q_DEFINE_THIS_FILE

/* private variables ---------------------------------------------------------*/
typedef struct ao_led
{
    QActive super;
    QTimeEvt timeEvt;
    bool status;
} ao_led_t;

/* private state function prototypes -----------------------------------------*/
static QState _state_initial(ao_led_t * const me, void const * const par);
static QState _state_on(ao_led_t * const me, QEvt const * const e);
static QState _state_off(ao_led_t * const me, QEvt const * const e);

/* private variables ---------------------------------------------------------*/
static ao_led_t led;

/* exported function ---------------------------------------------------------*/
static void ao_led_init(void)
{
    ao_led_t *me = (ao_led_t *)&led;
    QActive_ctor(&me->super, Q_STATE_CAST(&_state_initial));
    QTimeEvt_ctorX(&me->timeEvt, &me->super, Q_QPC_TEST_SIG, 0U);

    static uint8_t stack_led[1024];
    static QEvt const * led_evet_queue[32];

    QACTIVE_START(&me->super,
                  osPriorityNormal,                         /* QP priority */
                  led_evet_queue, Q_DIM(led_evet_queue),    /* evt queue */
                  (void *)stack_led, 1024U,                 /* no per-thread stack */
                  (QEvt *)0);                               /* no initialization event */
}
INIT_EXPORT(ao_led_init, EXPORT_APP);

/* private state function ----------------------------------------------------*/
static QState _state_initial(ao_led_t * const me, void const * const par)
{
    (void)par;
    
    QTimeEvt_ctorX(&me->timeEvt, &me->super, Q_QPC_TEST_SIG, 0U);
    QTimeEvt_armX(&me->timeEvt, 1U, 500);

    return Q_TRAN(&_state_off);
}

static QState _state_on(ao_led_t * const me, QEvt const * const e)
{
    QState _status;

    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            me->status = true;
            printf("Led on!\n");
            _status = Q_HANDLED();
            break;
        }

        case Q_QPC_TEST_SIG:
        {
            _status = Q_TRAN(&_state_off);
            break;
        }

        default:
        {
            _status = Q_SUPER(&QHsm_top);
            break;
        }
    }

    return _status;
}

static QState _state_off(ao_led_t * const me, QEvt const * const e)
{
    QState _status;

    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            me->status = false;
            printf("Led off!\n");
            _status = Q_HANDLED();
            break;
        }

        case Q_QPC_TEST_SIG:
        {
            _status = Q_TRAN(&_state_on);
            break;
        }

        default: {
            _status = Q_SUPER(&QHsm_top);
            break;
        }
    }

    return _status;
}

/* ----------------------------- end of file -------------------------------- */
