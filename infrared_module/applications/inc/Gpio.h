/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-01-12     Qiuyijie     the first version
 */
#ifndef APPLICATIONS_GPIO_H_

#define APPLICATIONS_GPIO_H_


#include <board.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <sys/types.h>
//#include <dfs_posix.h>

#include <unistd.h>
//#include <netdb.h>
//#include "netdev.h"

//#include <sys/socket.h> /* 使用BSD socket，需要包含socket.h头文件 */
//#include <sys/select.h> /* 使用 dfs select 功能  */
#include <sys/time.h>
#include <unistd.h>
//#include <netdb.h>
//#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <rtdbg.h>
#include "typedef.h"

//配置GPIO
#define     BUTTON_DOWN         GET_PIN(B, 3)
#define     BUTTON_UP           GET_PIN(B, 2)
#define     BUTTON_ENTER        GET_PIN(B, 1)

#define     LED_STATE           GET_PIN(B, 0)

#define     UART1_EN            GET_PIN(A, 11)

//优先级
#define     UART2_PRIORITY                  18
#define     OLED_PRIORITY                   20
#define     BUTTON_PRIORITY                 21

//线程名
#define     UART2_THREAD_NAME               "sensor task"
#define     OLED_THREAD_NAME                "oled task"
#define     BUTTON_THREAD_NAME              "button task"

//线程周期
#define     UART_PERIOD                     50
#define     BUTTON_TASK_PERIOD              20

typedef     struct buttonInfo               type_button_t;
typedef     struct codeDataRecv             type_codedata_recv;
typedef     struct codeDataRecvMatch        type_recv_match;

void GpioInit(void);
u16 TimerTask(u16 *, u16, u8 *);
u16 usModbusRTU_CRC(const u8*, u32 );
void ReadUniqueId(u32 *id);
void LED_TEST(void);
void LED_TEST_LONG(void);
void debugPrint(int, void *, u8);
time_t getTimeStamp(void);
u8 checkResult(u8 *, u8);
u32 crc32_cal(u8 *, u32);
void LED_ON(void);
#endif /* APPLICATIONS_GPIO_H_ */
