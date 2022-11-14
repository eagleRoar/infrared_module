/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-09-19     Administrator       the first version
 */
#ifndef APPLICATIONS_UART_MODULE_H_
#define APPLICATIONS_UART_MODULE_H_

#include "Gpio.h"
#include "Uart.h"

enum
{
    REGISTER_NULL = 0x00,
    REGISTER_SUCESS,
    REGISTER_FAIL
};


u8 getMasterEvent(void);
void setMasterEvent(u8 event);
void sendRegisterToMaster(rt_device_t , u8 *);
rt_err_t analyzeMasterData(u8 *, rt_size_t);
u16 getRegister(u16);
void replyDataToMaster(u8 , u8 *, u16 , u8);
void setRegister(u16, u16);
void setDataToFlah(u32, u8 *, size_t);
void getDataFromFlash(u32, u8 *, size_t);
#endif /* APPLICATIONS_UART_MODULE_H_ */
