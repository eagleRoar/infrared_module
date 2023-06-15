/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-09-25     12154       the first version
 *
 * 本文件主要是针对红外模块的通讯
 */
#include "infrared.h"
#include "Oled1309.h"
#include "uart.h"

static  infrared_event_t    infrared_event;           //与空调的事件

infrared_event_t getInfraredEvent(void)
{
    return infrared_event;
}

void setinfraredEvent(infrared_event_t envent)
{
    infrared_event = envent;
}

void sendMatchDataToCodeData(rt_device_t device)
{
    u8      data[10]        = {0};

    data[0] = 0xAA;
    data[1] = 0x55;
    data[2] = 0x07;
    data[3] = 0x82;
    data[4] = 0x00;
    data[5] = 0x00;
    data[6] = 0x00;
    data[7] = 0x00;
    data[8] = 0x01;
    data[9] = 0x7B;
    rt_device_write(device, 0, data, 10);
}

void sendSaveCommandToCodeData(rt_device_t device)
{
    u8      data[12]        = {0};

    data[0] = 0xAA;
    data[1] = 0x55;
    data[2] = 0x09;
    data[3] = 0x91;
    data[4] = getModuleInfo()->brand_code >> 8;
    data[5] = getModuleInfo()->brand_code;
    data[6] = 0x00;
    data[7] = 0x82;
    data[8] = 0x21;
    data[9] = 0x99;
    data[10] = 0x07;
    data[11] = checkResult(data, 11);
    rt_device_write(device, 0, data, 12);
}

void sendCtrlCommandToCodeData(rt_device_t device)
{
    u8      data[12]        = {0};

    if(SELECT_MATCH == getModuleInfo()->select)
    {
        data[0] = 0xAA;
        data[1] = 0x55;
        data[2] = 0x09;
        data[3] = 0x00;
        data[4] = 0x00;
        data[5] = getModuleInfo()->brand_code >> 8;//     //代号码
        data[6] = getModuleInfo()->brand_code;//
        data[7] = getModuleInfo()->ctrl >> 8;
        data[8] = getModuleInfo()->ctrl;
        data[9] = 0x00;
        data[10] = 0x00;
        data[11] = checkResult(data, 11);
        rt_device_write(device, 0, data, 12);

        if((getModuleInfo()->ctrl & 0x8000) > 0)
        {
            setSwitchState(YES);
        }
        else
        {
            setSwitchState(NO);
        }

        setNowTemp(getTempByCtrl(getModuleInfo()->ctrl));
    }
    else if(SELECT_LEARN == getModuleInfo()->select)
    {
        //bit 7 为开关位
        if((getModuleInfo()->ctrl & 0x8000) > 0)
        {
            sendlearnOn(device);
        }
        else
        {
            sendlearnOff(device);
        }
    }
}

void sendMatchOnTest(rt_device_t device)
{
    u8      data[12]        = {0};

    //AA 55 09 00 00 00 28 E0 00 00 00 3E

    data[0] = 0xAA;
    data[1] = 0x55;
    data[2] = 0x09;
    data[3] = 0x00;
    data[4] = 0x00;
    data[5] = getModuleInfo()->brand_code >> 8;
    data[6] = getModuleInfo()->brand_code;
    data[7] = 0xE0;
    data[8] = 0x00;
    data[9] = 0x00;
    data[10] = 0x00;
    data[11] = checkResult(data, 11);
    rt_device_write(device, 0, data, 12);
    setSwitchState(YES);
    u16 ctr = 0xE000;
    getModuleInfo()->ctrl = ctr;
    setNowTemp(getTempByCtrl(ctr));
}


void sendMatchOffTest(rt_device_t device)
{
    u8      data[12]        = {0};

    data[0] = 0xAA;
    data[1] = 0x55;
    data[2] = 0x09;
    data[3] = 0x00;
    data[4] = 0x00;
    data[5] = getModuleInfo()->brand_code >> 8;     //代号码
    data[6] = getModuleInfo()->brand_code;
    data[7] = 0x60;
    data[8] = 0x00;
    data[9] = 0x00;
    data[10] = 0x00;
    data[11] = checkResult(data, 11);
    rt_device_write(device, 0, data, 12);
    setSwitchState(NO);
    u16 ctr = 0x6000;
    getModuleInfo()->ctrl = ctr;
    setNowTemp(getTempByCtrl(ctr));
}

void sendlearnOn(rt_device_t device)
{
    u8      data[10]        = {0};

    //AA 55 07 84 00 00 00 00 01 7D
    data[0] = 0xAA;
    data[1] = 0x55;
    data[2] = 0x07;
    data[3] = 0x84;
    data[4] = 0x00;
    data[5] = 0x00;     //代号码
    data[6] = 0x00;
    data[7] = 0x00;
    data[8] = 0x01;
    data[9] = checkResult(data, 9);
    rt_device_write(device, 0, data, 10);
    setSwitchState(YES);
    u16 ctr = 0xE000;
    getModuleInfo()->ctrl = ctr;
    setNowTemp(getTempByCtrl(ctr));
}

void sendlearnOff(rt_device_t device)
{
    u8      data[10]        = {0};

    //AA 55 07 84 00 00 00 00 01 7D
    data[0] = 0xAA;
    data[1] = 0x55;
    data[2] = 0x07;
    data[3] = 0x84;
    data[4] = 0x00;
    data[5] = 0x00;     //代号码
    data[6] = 0x00;
    data[7] = 0x01;
    data[8] = 0x01;
    data[9] = checkResult(data, 9);
    rt_device_write(device, 0, data, 10);
    setSwitchState(NO);
    u16 ctr = 0x6000;
    getModuleInfo()->ctrl = ctr;
    setNowTemp(getTempByCtrl(ctr));
}

void sendInLearnOnMode(rt_device_t device)
{
    u8      data[10]        = {0};
    //AA 55 07 80 00 00 00 00 01 79
    data[0] = 0xAA;
    data[1] = 0x55;
    data[2] = 0x07;
    data[3] = 0x80;
    data[4] = 0x00;
    data[5] = 0x00;     //代号码
    data[6] = 0x00;
    data[7] = 0x00;
    data[8] = 0x01;
    data[9] = checkResult(data, 9);
    rt_device_write(device, 0, data, 10);
}

void sendInLearnOffMode(rt_device_t device)
{
    u8      data[10]        = {0};
    //AA 55 07 80 00 00 00 00 01 79
    data[0] = 0xAA;
    data[1] = 0x55;
    data[2] = 0x07;
    data[3] = 0x80;
    data[4] = 0x00;
    data[5] = 0x00;     //代号码
    data[6] = 0x00;
    data[7] = 0x01;
    data[8] = 0x01;
    data[9] = checkResult(data, 9);
    rt_device_write(device, 0, data, 10);
}

void sendMatchOver(rt_device_t device)
{
    u8      data[10]        = {0};

    data[0] = 0xAA;
    data[1] = 0x55;
    data[2] = 0x07;
    data[3] = 0x81;
    data[4] = 0x00;
    data[5] = 0x00;
    data[6] = 0x00;
    data[7] = 0x00;
    data[8] = 0x01;
    data[9] = checkResult(data, 9);
    rt_device_write(device, 0, data, 10);
}

void sendLearnOnOver(rt_device_t device)
{
    u8      data[10]        = {0};
    //AA 55 07 80 00 00 00 00 01 79
    data[0] = 0xAA;
    data[1] = 0x55;
    data[2] = 0x07;
    data[3] = 0x81;
    data[4] = 0x00;
    data[5] = 0x00;     //代号码
    data[6] = 0x00;
    data[7] = 0x00;
    data[8] = 0x01;
    data[9] = checkResult(data, 9);
    rt_device_write(device, 0, data, 10);
}

void sendLearnOffOver(rt_device_t device)
{
    u8      data[10]        = {0};
    //AA 55 07 80 00 00 00 00 01 79
    data[0] = 0xAA;
    data[1] = 0x55;
    data[2] = 0x07;
    data[3] = 0x81;
    data[4] = 0x00;
    data[5] = 0x00;     //代号码
    data[6] = 0x00;
    data[7] = 0x00;
    data[8] = 0x01;
    data[9] = checkResult(data, 9);
    rt_device_write(device, 0, data, 10);
}

//码库返回是否成功结果
u8 codeDataRecvResult(u8 *recv)
{
    u8                  res         = 0;
    type_codedata_recv  code_recv;

    rt_memcpy((u8 *)&code_recv, (u8 *)&recv, sizeof(type_codedata_recv));
    //2.校验前导码是否正确
    if(START_CODE == code_recv.start_code)
    {
        //3.查看是40命令
        if(0x40 == code_recv.command)
        {
            switch (code_recv.state_code) {
                case CODE_RECV_FAIL:
                    //进入学习命令接收失败
                    res = CODE_RECV_FAIL;
                    break;
                case CODE_RECV_SUCCESS:
                    //进入学习命令接收成功
                    res = CODE_RECV_SUCCESS;
                    break;
                case CODE_RECV_TIMEOUT:
                    //学习超时
                    res = CODE_RECV_TIMEOUT;
                    break;
                default:
                    break;
            }
        }
    }

    return res;
}

