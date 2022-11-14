/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-09-19     Administrator       the first version
 *
 * 本文件为发送数据和主机交互
 */
#include "module.h"
#include "drv_flash.h"

static      u8                          mater_envent;
//static  type_recv_match             match_result;     //接受到一键匹配结果
//static  u8                          register_flg     = NO;

void setMasterEvent(u8 event)
{
    mater_envent = event;
}

u8 getMasterEvent(void)
{
    return mater_envent;
}

void sendRegisterToMaster(rt_device_t device, u8 *data)
{
    u8      type    = INFRARED_TYPE;
    u32     uuid    = 0;

    ReadUniqueId(&uuid);
    data[0] = REGISTER_CODE;
    data[1] = 0x00;
    data[2] = 0x00;
    data[3] = 0x00;
    data[4] = 0x00;
    data[5] = 0x00;
    data[6] = 0x06;
    data[7] = getModuleInfo()->addr;
    data[8] = type;
    data[9] = uuid >> 24;
    data[10] = uuid >> 16;
    data[11] = uuid >> 8;
    data[12] = uuid;
    data[13] = usModbusRTU_CRC(data, 13);
    data[14] = usModbusRTU_CRC(data, 13) >> 8;
    sendMessageToMaster(data, 15);
}


//解析数据
rt_err_t analyzeMasterData(u8 *data, rt_size_t size)
{
    rt_err_t    ret             = RT_EOK;
    u8          num             = 0;
    u8          index           = 0;
    u8          temp            = 0;
    u16         *replyData      = RT_NULL;
    u16         reg             = 0;

    //1.确定是读还是写
    switch (data[1])
    {
        //1.读取寄存器
        case READ_MUTI:
            //读取需要获取的寄存器数量
            num = data[5];
            replyData = rt_malloc(num * 2);
            if(RT_NULL != replyData)
            {
                temp = (data[2] << 8)|data[3];
                for(index = 0; index < num; index++)
                {
                    replyData[index] = getRegister(temp + index);
                }

                replyDataToMaster(READ_MUTI, (u8 *)replyData, 0, num * 2);
                rt_free(replyData);
                replyData = RT_NULL;
            }
            break;
        //2.写入单个寄存器
        case WRITE_SINGLE:
            //2.1 如果长度不对，返回错误
            if(8 == size)
            {
                reg = (data[2] << 8) | data[3];
                setRegister(reg, (data[4] << 8) | data[5]);
                replyData = rt_malloc(2);
                if(RT_NULL != replyData)
                {
                    replyData[0] = getRegister(reg);
                    replyDataToMaster(WRITE_SINGLE, (u8 *)replyData, reg, 2);

                    rt_free(replyData);
                    replyData = RT_NULL;
                }
            }
            else
            {
                ret = RT_ERROR;
            }

            break;
        //3.写入多个寄存器
        case WRITE_MUTI:

            break;
        default:
            break;
    }

    return ret;
}

//获取寄存器
u16 getRegister(u16 addr)
{
    u16     value   = 0;
    u32     id      = 0;

    switch (addr) {
        case REGISTER_LOCATION:
            getModuleInfo()->find_location = YES;
        break;
        //1.版本号
        case REGISTER_VERSION:
            value = getModuleInfo()->version;
        break;
        //2.uuid
        case REGISTER_UUID1:
            ReadUniqueId(&id);
            value = id >> 16;
        break;
        case REGISTER_UUID2:
            ReadUniqueId(&id);
            value = id;
        break;
        //3.地址
        case REGISTER_ADDR:
            value = getModuleInfo()->addr;
        break;
        //4.版本号
        case REGISTER_TYPE:
            value = getModuleInfo()->type;
        break;
        case REGISTER_CTRL:
            value = (getModuleInfo()->ctrl >> 8) | (getModuleInfo()->ctrl << 8);
            break;
        default:
            break;
    }

    return value;
}

void setRegister(u16 addr, u16 data)
{
    switch (addr) {
        //1.版本号
        case REGISTER_VERSION:
        break;
        //2.uuid
        case REGISTER_UUID1:
        break;
        case REGISTER_UUID2:
        break;
        //3.地址
        case REGISTER_ADDR:
            getModuleInfo()->addr = data;
        break;
        //4.类型
        case REGISTER_TYPE:
        break;
        //5.发送给空调的控制命令
        case REGISTER_CTRL:
            getModuleInfo()->ctrl = data;
            break;
        default:
            break;
    }
}

void replyDataToMaster(u8 rw, u8 *data, u16 reg, u8 len)
{
    u8      *buf        = RT_NULL;

    buf = rt_malloc(len + 3 + 2);
    if(RT_NULL != buf)
    {
        if(READ_MUTI == rw)
        {
            buf[0] = getModuleInfo()->addr;
            buf[1] = rw;
            buf[2] = 0x00;
            buf[3] = len;
            rt_memcpy(&buf[4], data, len);
            buf[len + 4] = usModbusRTU_CRC(buf, len + 4);
            buf[len + 5] = usModbusRTU_CRC(buf, len + 4) >> 8;

            sendMessageToMaster(buf, len + 4+ 2);
        }
        else if(WRITE_SINGLE == rw)
        {
            buf[0] = getModuleInfo()->addr;
            buf[1] = rw;
            buf[2] = reg >> 8;
            buf[3] = reg;
            rt_memcpy(&buf[4], data, len);
            buf[len + 4] = usModbusRTU_CRC(buf, len + 4);
            buf[len + 5] = usModbusRTU_CRC(buf, len + 4) >> 8;

            sendMessageToMaster(buf, len + 4+ 2);
        }
        rt_free(buf);
        buf = RT_NULL;
    }
}

void setDataToFlah(u32 addr, u8 *data, size_t size)
{
        stm32_flash_erase(addr, size);
        stm32_flash_write(addr, data, size);
}

void getDataFromFlash(u32 addr, u8 *data, size_t size)
{
    stm32_flash_read(addr, data, size);
}
