/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-10-25     Administrator       the first version
 */
#ifndef APPLICATIONS_INC_REAL_TIME_H_
#define APPLICATIONS_INC_REAL_TIME_H_

#include "Gpio.h"

struct system_time
{
    u16 year;
    u8  month;
    u8  day;
    u8  hour;
    u8  minute;
    u8  second;
};

typedef struct system_time type_sys_time;

void rtcSet(type_sys_time);
time_t getTimeStamp(void);

#endif /* APPLICATIONS_INC_REAL_TIME_H_ */
