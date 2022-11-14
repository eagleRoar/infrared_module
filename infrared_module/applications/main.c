/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-09-03     RT-Thread    first version
 */

#include <rtthread.h>
#include "Gpio.h"
#include "Uart.h"

#include "Oled1309.h"
#include "ButtonTask.h"
#include "real_time.h"

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

int main(void)
{
    GpioInit();
    UartTaskInit();
    ButtonTaskInit();
    OledTaskInit();

    while (1)
    {
        rt_thread_mdelay(1000);
    }

    return RT_EOK;
}

void GpioInit(void)
{

    /* oled 1309屏设置引脚*/
    rt_pin_mode(GET_PIN(B, 5), PIN_MODE_OUTPUT);
    rt_pin_mode(GET_PIN(B, 6), PIN_MODE_OUTPUT);
    rt_pin_mode(GET_PIN(B, 7), PIN_MODE_OUTPUT);
    rt_pin_mode(GET_PIN(B, 8), PIN_MODE_OUTPUT);
    rt_pin_mode(GET_PIN(B, 13), PIN_MODE_OUTPUT);
    rt_pin_mode(GET_PIN(B, 15), PIN_MODE_OUTPUT);

    //Button 设置
//    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);    //禁止JTAG保留SWD
    rt_pin_mode(BUTTON_DOWN, PIN_MODE_INPUT);
    rt_pin_mode(BUTTON_ENTER, PIN_MODE_INPUT);
    rt_pin_mode(BUTTON_UP, PIN_MODE_INPUT);

    rt_pin_mode(LED_STATE, PIN_MODE_OUTPUT);

    //串口
    rt_pin_mode(UART1_EN, PIN_MODE_OUTPUT);
    rt_pin_write(UART1_EN, NO); //使能为接受模式
}

u16 usModbusRTU_CRC(const u8* pucData, u32 ulLen)
{
    u8 ucIndex = 0U;
    u16 usCRC = 0xFFFFU;

    while (ulLen > 0U) {
        usCRC ^= *pucData++;
        while (ucIndex < 8U) {
            if (usCRC & 0x0001U) {
                usCRC >>= 1U;
                usCRC ^= 0xA001U;
            } else {
                usCRC >>= 1U;
            }
            ucIndex++;
        }
        ucIndex = 0U;
        ulLen--;
    }
    return usCRC;
}

u16 TimerTask(u16 *time, u16 touchTime, u8 *flag)
{
    u16 temp = 0;

    temp = *time;

    if(*time < touchTime)
    {
        temp++;
        *time = temp;
        *flag = NO;
    }
    else
    {
        *flag = YES;
        *time = 0;
    }

    return *time;
}

void ReadUniqueId(u32 *id)
{
    u8      data[12]    = {0};
    u32     id1         = 0;
    u32     id2         = 0;
    u32     id3         = 0;

    id1 = *(__IO u32*)(ID_ADDR1);
    id2 = *(__IO u32*)(ID_ADDR2);
    id3 = *(__IO u32*)(ID_ADDR3);

    rt_memcpy(&data[0], (u8 *)&id1, 4);
    rt_memcpy(&data[4], (u8 *)&id2, 4);
    rt_memcpy(&data[8], (u8 *)&id3, 4);

    *id = crc32_cal(data, 12);
}

void LED_ON(void)
{
    rt_pin_write(LED_STATE, NO);
}

void LED_TEST(void)
{
    static  u8      state = 0;

    if(state++ % 2)
    {
        rt_pin_write(LED_STATE, YES);
    }
    else
    {
        rt_pin_write(LED_STATE, NO);
    }
}

void LED_TEST_LONG(void)
{
    u8 state = 0;

    while(state < 40)
    {
        state++;
        LED_TEST();
        rt_thread_mdelay(50);
    }
}

void debugPrint(int i, void *data, u8 len)
{
    u8 index = 0;

    rt_pin_write(UART1_EN, YES);
    switch (i) {
        case 0:
            for(index = 0; index < len; index++)
            {
                rt_kprintf("%x ",*(u8 *)&data[index]);
            }
            rt_kprintf("\r\n");
            break;
        case 1:
            rt_kprintf("%.*s\r\n",len,(char *)data);
            break;
        default:
            break;
    }
    rt_pin_write(UART1_EN, NO);
}

u8 checkResult(u8 *data, u8 len)
{
    u8      index       = 0;
    u16     crc         = 0;

    for(index = 0; index < len; index++)
    {
        crc ^= data[index];
    }

    return crc & 0xff;
}
