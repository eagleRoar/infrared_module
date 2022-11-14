/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-09-25     12154       the first version
 */
#ifndef APPLICATIONS_UART_INFRARED_H_
#define APPLICATIONS_UART_INFRARED_H_

#include "Gpio.h"

typedef     void (*Call_Back)(u8);
/* 事件消息结构 */
struct infrared_event_s
{
    u8          event;
    Call_Back   call_back;
};

enum
{
    INFRARED_EVENT_MATCH_OK = 0x01,
    INFRARED_EVENT_MATCH_FAIL,
    INFRARED_EVENT_SAVE_OK,
    INFRARED_EVENT_SAVE_FAIL,
};

typedef     struct infrared_event_s            infrared_event_t;

infrared_event_t getInfraredEvent(void);
void setinfraredEvent(infrared_event_t);
void sendMatchDataToCodeData(rt_device_t);
void sendSaveCommandToCodeData(rt_device_t);
u8 codeDataRecvResult(u8 *recv);
void sendMatchOnTest(rt_device_t);
void sendMatchOffTest(rt_device_t);
void sendInLearnOnMode(rt_device_t);
void sendInLearnOffMode(rt_device_t);
void sendlearnOn(rt_device_t);
void sendlearnOff(rt_device_t);
void sendMatchOver(rt_device_t);
void sendLearnOnOver(rt_device_t);
void sendLearnOffOver(rt_device_t);
void sendCtrlCommandToCodeData(rt_device_t);
#endif /* APPLICATIONS_UART_INFRARED_H_ */
