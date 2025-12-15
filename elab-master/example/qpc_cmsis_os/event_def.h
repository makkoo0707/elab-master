/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef __EVENT_DEF_H__
#define __EVENT_DEF_H__

/* includes ----------------------------------------------------------------- */
#include "elab/3rd/qpc/include/qpc.h"

/* public typedef ----------------------------------------------------------- */
enum
{
    Q_QPC_TEST_SIG = Q_USER_SIG,
    
    Q_MAX_PUB_SIG,
    Q_MAX_SIG,                        /* the last signal (keep always last) */
};

#endif

/* ----------------------------- end of file -------------------------------- */
