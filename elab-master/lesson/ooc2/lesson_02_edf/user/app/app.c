
/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include "device/oled.h"
#include "elab/3rd/qpc/include/qpc.h"
#include "elab/os/cmsis_os.h"
#include "elab/common/elab_export.h"
#include "elab/common/elab_assert.h"
#include "elab/edf/elab_device.h"
#include "elab/edf/user/elab_button.h"
#include "config/event_def.h"

ELAB_TAG("App");
Q_DEFINE_THIS_MODULE("App")

#ifdef __cplusplus
extern "C" {
#endif

/* private state function prototypes -----------------------------------------*/
static QState _state_initial(QActive * const me, void const * const par);
static QState _state_idle(QActive * const me, QEvt const * const e);
static QState _state_work(QActive * const me, QEvt const * const e);

/* Private variables ---------------------------------------------------------*/
static QActive oled_game;

static const char *str_info[] =
{
    "EVT_BUTTON_UP.",
    "EVT_BUTTON_DOWN.",
    "EVT_BUTTON_LEFT.",
    "EVT_BUTTON_RIGHT.",
    "EVT_BUTTON_START.",
    "EVT_BUTTON_STOP.",
};

/* public functions --------------------------------------------------------- */
static void oled_game_init(void)
{
    static uint8_t stack[1024];                             /* stack. */
    static QEvt const *e_queue[8];                          /* event queue. */

    QActive_ctor(&oled_game, Q_STATE_CAST(&_state_initial));
    QACTIVE_START(&oled_game,
                    osPriorityLow,                  /* QP priority */
                    e_queue, Q_DIM(e_queue),        /* evt queue */
                    (void *)stack, 1024U,           /* no per-thread stack */
                    (QEvt *)0);                     /* no initialization event */

}
INIT_EXPORT(oled_game_init, EXPORT_APP);

/* private state function ----------------------------------------------------*/
static QState _state_initial(QActive * const me, void const * const par)
{
    (void)par;
    (void)me;

    elab_device_t *dev = NULL;

    QActive_subscribe(me, EVT_BUTTON_START);
    QActive_subscribe(me, EVT_BUTTON_STOP);
    QActive_subscribe(me, EVT_BUTTON_UP);
    QActive_subscribe(me, EVT_BUTTON_DOWN);
    QActive_subscribe(me, EVT_BUTTON_LEFT);
    QActive_subscribe(me, EVT_BUTTON_RIGHT);

    /* Button start click event. */
    dev = elab_device_find("button_start");
    elab_assert(dev != NULL);
    elab_button_set_event_signal(dev, ELAB_BUTTON_EVT_CLICK, EVT_BUTTON_START);

    static const char * const button_talbe[] =
    {
        "button_start",
        "button_stop",
        "button_up",
        "button_down",
        "button_right",
        "button_left",
    };
    static const uint32_t event_talbe[] =
    {
        EVT_BUTTON_START,
        EVT_BUTTON_STOP,
        EVT_BUTTON_UP,
        EVT_BUTTON_DOWN,
        EVT_BUTTON_RIGHT,
        EVT_BUTTON_LEFT,
    };
    elab_assert((sizeof(button_talbe) / sizeof(const char *)) ==
                (sizeof(event_talbe) / sizeof(uint32_t)));
    
    for (uint32_t i = 0; i < (sizeof(event_talbe) / sizeof(uint32_t)); i ++)
    {
        dev = elab_device_find(button_talbe[i]);
        elab_assert(dev != NULL);
        elab_button_set_event_signal(dev, ELAB_BUTTON_EVT_CLICK, event_talbe[i]);
    }

    return Q_TRAN(&_state_work);
}

static QState _state_idle(QActive * const me, QEvt const * const e)
{
    QState ret = Q_SUPER(&QHsm_top);

    switch (e->sig)
    {
    case EVT_BUTTON_START:
        elog_debug(str_info[e->sig - EVT_BUTTON_UP]);
        ret = Q_TRAN(&_state_work);
        break;

    case EVT_BUTTON_STOP:
    case EVT_BUTTON_UP:
    case EVT_BUTTON_DOWN:
    case EVT_BUTTON_LEFT:
    case EVT_BUTTON_RIGHT:
        elog_debug(str_info[e->sig - EVT_BUTTON_UP]);
        ret = Q_HANDLED();
        break;
    }
    
    return ret;
}

static QState _state_work(QActive * const me, QEvt const * const e)
{
    QState ret = Q_SUPER(&QHsm_top);

    switch (e->sig)
    {
    case EVT_BUTTON_START:
        elog_debug("EVT_BUTTON_START.");
        oled_game_start();
        ret = Q_HANDLED();
        break;

    case EVT_BUTTON_STOP:
        elog_debug("EVT_BUTTON_STOP.");
        oled_game_stop();
        ret = Q_TRAN(&_state_idle);
        break;

    case EVT_BUTTON_UP:
        elog_debug("EVT_BUTTON_UP.");
        oled_game_execute(OLED_CMD_UP);
        ret = Q_HANDLED();
        break;

    case EVT_BUTTON_DOWN:
        elog_debug("EVT_BUTTON_DOWN.");
        oled_game_execute(OLED_CMD_DOWN);
        ret = Q_HANDLED();
        break;

    case EVT_BUTTON_LEFT:
        elog_debug("EVT_BUTTON_LEFT.");
        oled_game_execute(OLED_CMD_LEFT);
        ret = Q_HANDLED();
        break;

    case EVT_BUTTON_RIGHT:
        elog_debug("EVT_BUTTON_RIGHT.");
        oled_game_execute(OLED_CMD_RIGHT);
        ret = Q_HANDLED();
        break;
    }

    return ret;
}

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
