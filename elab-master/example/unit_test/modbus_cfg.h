/*
*********************************************************************************************************
*                                              uC/Modbus
*                                       The Embedded Modbus Stack
*
*                    Copyright 2003-2020 Silicon Laboratories Inc. www.silabs.com
*
*                                 SPDX-License-Identifier: APACHE-2.0
*
*               This software is subject to an open source license and is distributed by
*                Silicon Laboratories Inc. pursuant to the terms of the Apache License,
*                    Version 2.0 available at www.apache.org/licenses/LICENSE-2.0.
*
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                      MODBUS CONFIGURATION
*
* Filename : mb_cfg.h
* Version  : V2.14.00
*********************************************************************************************************
* Note(s)  : (1) This file contains configuration constants for uC/Modbus
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                  MODBUS MODULES CONFIGURATION
*********************************************************************************************************
*/

#define MODBUS_CFG_SLAVE_EN             1
#define MODBUS_CFG_MASTER_EN            1
#define MODBUS_CFG_ASCII_EN             1          /* Modbus ASCII is supported when 1         */
#define MODBUS_CFG_RTU_EN               1          /* Modbus RTU   is supported when 1         */

#define MODBUS_CFG_BUF_SIZE             (1024)        /* Maximum outgoing message size. */
#define MODBUS_NODE_ADDR_MAX            (32)

/*
*********************************************************************************************************
*                                   MODBUS FUNCTIONS CONFIGURATION
*********************************************************************************************************
*/

#define MODBUS_CFG_FC01_EN              1          /* Enable or Disable support for Modbus functions     */
#define MODBUS_CFG_FC02_EN              1
#define MODBUS_CFG_FC03_EN              1
#define MODBUS_CFG_FC04_EN              1
#define MODBUS_CFG_FC05_EN              1
#define MODBUS_CFG_FC06_EN              1
#define MODBUS_CFG_FC15_EN              1
#define MODBUS_CFG_FC16_EN              1


/*
*********************************************************************************************************
*                                  MODBUS FLOATING POINT SUPPORT
*********************************************************************************************************
*/

#define MODBUS_CFG_FP_EN                1            /* Enable Floating-Point support.                     */

#if     (MODBUS_CFG_FP_EN != 0)
#define MODBUS_CFG_FP_START_IX                  10000           /* Start address of Floating-Point registers          */
#else
#define MODBUS_CFG_FP_START_IX                  50000           /* Floating point is disabled, set start of FP very high */
#endif



