/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef MODBUS_DEF_H
#define MODBUS_DEF_H

/*
********************************************************************************
*                         GLOBAL MODBUS CONSTANTS
********************************************************************************
*/
#define MODBUS_MODE_ASCII                          1
#define MODBUS_MODE_RTU                            0

#define MODBUS_SLAVE                               0
#define MODBUS_MASTER                              1

/*
********************************************************************************
*                         CONSTANTS
********************************************************************************
*/

#define MODBUS_FC01_COIL_RD                        1   /* COIL Status. */
#define MODBUS_FC02_DI_RD                          2   /* Read Discrete Input. */
#define MODBUS_FC03_HOLDING_REG_RD                 3   /* Holding registers. */
#define MODBUS_FC04_IN_REG_RD                      4   /* Read Only registers. */
#define MODBUS_FC05_COIL_WR                        5   /* Set a single COIL value. */
#define MODBUS_FC06_HOLDING_REG_WR                 6   /* Holding registers. */
#define MODBUS_FC15_COIL_WR_MULTIPLE              15   /* Set multiple COIL values. */
#define MODBUS_FC16_HOLDING_REG_WR_MULTIPLE       16   /* Holding registers */

/*
********************************************************************************
*                               ERROR CODES
********************************************************************************
*/

#define MODBUS_ERR_NONE                            0

#define MODBUS_ERR_ILLEGAL_FC                      1
#define MODBUS_ERR_ILLEGAL_DATA_ADDR               2
#define MODBUS_ERR_ILLEGAL_DATA_QTY                3
#define MODBUS_ERR_ILLEGAL_DATA_VAL                4

#define MODBUS_ERR_FC01_01                       101
#define MODBUS_ERR_FC01_02                       102
#define MODBUS_ERR_FC01_03                       103

#define MODBUS_ERR_FC02_01                       201
#define MODBUS_ERR_FC02_02                       202

#define MODBUS_ERR_FC03_01                       301
#define MODBUS_ERR_FC03_02                       302
#define MODBUS_ERR_FC03_03                       303
#define MODBUS_ERR_FC03_04                       304

#define MODBUS_ERR_FC04_01                       401
#define MODBUS_ERR_FC04_02                       402
#define MODBUS_ERR_FC04_03                       403
#define MODBUS_ERR_FC04_04                       404

#define MODBUS_ERR_FC05_01                       501
#define MODBUS_ERR_FC05_02                       502

#define MODBUS_ERR_FC06_01                       601

#define MODBUS_ERR_FC15_01                      1501
#define MODBUS_ERR_FC15_02                      1502
#define MODBUS_ERR_FC15_03                      1503

#define MODBUS_ERR_FC16_01                      1601
#define MODBUS_ERR_FC16_02                      1602
#define MODBUS_ERR_FC16_03                      1603
#define MODBUS_ERR_FC16_04                      1604
#define MODBUS_ERR_FC16_05                      1605

#define MODBUS_ERR_TIMED_OUT                    3000
#define MODBUS_ERR_RANGE                        4000

#define MODBUS_ERR_COIL_ADDR                    5000
#define MODBUS_ERR_COIL_WR                      5001
#define MODBUS_ERR_SLAVE_ADDR                   5002
#define MODBUS_ERR_FC                           5003
#define MODBUS_ERR_BYTE_COUNT                   5004
#define MODBUS_ERR_COIL_QTY                     5005
#define MODBUS_ERR_REG_ADDR                     5006
#define MODBUS_ERR_NBR_REG                      5007
#define MODBUS_ERR_SUB_FNCT                     5008
#define MODBUS_ERR_DIAG                         5009
#define MODBUS_ERR_WR                           5010

#define MODBUS_ERR_RX                           6000

#endif

/* ----------------------------- end of file -------------------------------- */
