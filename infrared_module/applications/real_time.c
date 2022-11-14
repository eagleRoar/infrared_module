/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-10-25     Administrator       the first version
 */
#include "Gpio.h"
#include "real_time.h"


//void rtcSet(type_sys_time time)
//{
//    rt_err_t ret = RT_EOK;
//
////    /* 设置日期 */
////    ret = set_date(2022, 5, 30);
////    set_time(14, 22, 0);
////    if (ret != RT_EOK)
////    {
////        LOG_D("set RTC date failed\n");
////    }
//    /* 设置日期 */
//    ret = set_date(time.year, time.month, time.day);
//    set_time(time.hour, time.minute, time.second);
//    if (ret != RT_EOK)
//    {
//        LOG_D("set RTC date failed\n");
//    }
//}
//
time_t getTimeStamp(void)
{
    //注意 经常使用该函数作为定时器 如果修改了当前时间初始设置会导致问题
    return time(RT_NULL);
}
