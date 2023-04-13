/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-01-12     Qiuyijie     the first version
 */

/*所有的设备类都需要在周期内发送询问状态等指令保持连接，设备的周期在10秒，从机询问周期要低于10秒，在6秒询问一次*/

#include "Gpio.h"
#include "Uart.h"
#include "Oled1309.h"
#include "OledPage.h"
#include "infrared.h"
#include "module.h"

static      module_info_t               module_info;
struct      rx_msg                      uart1_msg;              //接收串口数据以及相关消息
struct      rx_msg                      uart2_msg;              //接收串口数据以及相关消息

extern u32             pageInfor;   //只支持最多四级目录
static      rt_device_t     uart1_serial;

//获取模块相关信息
module_info_t *getModuleInfo(void)
{
    return &module_info;
}

//初始化模块相关信息
void initModuleInfo(void)
{
    u16 crc = 0;
    //如果crc校验不对 说明第一次使用flash数据
    crc = usModbusRTU_CRC((u8 *)(getModuleInfo() + 2), sizeof(module_info_t) - 2);
    if(crc != getModuleInfo()->crc)
    {
        rt_memset((u8 *)(getModuleInfo() + 2), 0, sizeof(module_info_t) - 2);
        getModuleInfo()->addr = 0x01;
    }
    getModuleInfo()->register_state = REGISTER_NULL;
    getModuleInfo()->type = INFRARED_TYPE;

    //crc校验
    getModuleInfo()->crc = usModbusRTU_CRC((u8 *)(getModuleInfo() + 2), sizeof(module_info_t) - 2);
}

/**
 * @brief  : 接收hub 控制的函数
 */
static rt_err_t Uart1_input(rt_device_t dev, rt_size_t size)
{
    u16 crc16 = 0x0000;

    uart1_msg.dev = dev;
    uart1_msg.size = size;
    rt_device_read(uart1_msg.dev, 0, uart1_msg.data, uart1_msg.size);
    if(2 > size)
    {
        return RT_ERROR;
    }

    //如果不是注册命令或者控制命令则直接过滤
    if(!((REGISTER_CODE == uart1_msg.data[0]) || (getModuleInfo()->addr == uart1_msg.data[0])))
    {
        return RT_ERROR;
    }

    crc16 |= uart1_msg.data[uart1_msg.size-1];
    crc16 <<= 8;
    crc16 |= uart1_msg.data[uart1_msg.size-2];
    if(crc16 == usModbusRTU_CRC(uart1_msg.data, uart1_msg.size - 2))
    {
        uart1_msg.messageFlag = YES;
        return RT_EOK;
    }
    else
    {
        return RT_ERROR;
    }
}

/**
 *  : 发送给红外的
 */
static rt_err_t Uart2_input(rt_device_t dev, rt_size_t size)
{
    uart2_msg.dev = dev;
    uart2_msg.size = size;
    rt_device_read(uart2_msg.dev, 0, uart2_msg.data, uart2_msg.size);

    if(size > 2)
    {
        if((0xAA == uart2_msg.data[0]) && (0x55 == uart2_msg.data[1]))
        {
            uart2_msg.messageFlag = YES;
        }
    }

    return RT_EOK;
}

/**
 *
 * @param period 时间周期
 */
void IoCheckProgram(rt_device_t uart, u16 period)
{
    static  u8      Laststate = 1;
    static  u16      continueTime = 0;
    u8 state = 0;

    state = rt_pin_read(IO_CHECK);

    if(Laststate != state)
    {
        //持续时间
        if(continueTime < period * 10)
        {
            continueTime += period;
        }
        else
        {
            Laststate = state;

            if(1 == state)
            {
                //电平有低变高，发送关闭指令
                if(SELECT_MATCH == getModuleInfo()->select)
                {
                    sendMatchOffTest(uart);
                }
                else if(SELECT_LEARN == getModuleInfo()->select)
                {
                    sendlearnOff(uart);
                }
            }
            else if(0 == state)
            {
                //电平高变低，发送开指令
                if(SELECT_MATCH == getModuleInfo()->select)
                {
                    sendMatchOnTest(uart);
                }
                else if(SELECT_LEARN == getModuleInfo()->select)
                {
                    sendlearnOn(uart);
                }
            }
        }
    }
    else
    {
        continueTime = 0;
    }
}

/**
 * @brief  : 红外
 */

void UartTaskEntry(void* parameter)
{
    u8                          data[16]            = {0};
    u32                         uuid                = 0;
    u32                         id                  = 0;
    static      u8              Timer100msTouch     = NO;
    static      u8              Timer1sTouch        = NO;
    static      u8              Timer5sTouch        = NO;
    static      u8              Timer10sTouch       = NO;
    static      u16             time100ms           = 0;
    static      u16             time1S              = 0;
    static      u16             time5S              = 0;
    static      u16             time10S             = 0;
//    static      rt_device_t     uart1_serial;
    static      rt_device_t     uart2_serial;
    type_recv_match             recv_match;
    type_codedata_recv          recvResult;
    infrared_event_t            infrared_event      = getInfraredEvent();   //与红外交互的事件
    u8                          master_event        = getMasterEvent();     //与模块交互的事件
    static      module_info_t   modulePre           = {0};
    static      u16             ctrl_pre            = 0;
    static      u16             find_location_cnt   = 0;
    static      u8              startFlag           = NO;
    static      u8              startCheckIo        = NO;

    /* 查找串口设备 */
    uart1_serial = rt_device_find(DEVICE_UART1);
    rt_device_open(uart1_serial, RT_DEVICE_FLAG_DMA_RX);
    rt_device_set_rx_indicate(uart1_serial, Uart1_input);

    uart2_serial = rt_device_find(DEVICE_UART2);
    rt_device_open(uart2_serial, RT_DEVICE_FLAG_DMA_RX);
    rt_device_set_rx_indicate(uart2_serial, Uart2_input);

    //获取module info
    getDataFromFlash(DATA_BLOCK_START, (u8 *)getModuleInfo(), sizeof(module_info_t));
    if((SELECT_MATCH != getModuleInfo()->select) && (SELECT_LEARN != getModuleInfo()->select))
    {
        getModuleInfo()->select = SELECT_MATCH;
    }
    //初始化事件
    initModuleInfo();
    //发送注册命令
    setMasterEvent(EVENT_REGISTER);

    getModuleInfo()->ctrl = 0x0000;
    ctrl_pre = getModuleInfo()->ctrl;

    while (1)
    {
        time1S = TimerTask(&time1S, 1000/UART_PERIOD, &Timer1sTouch);                       //1s定时任务
        time5S = TimerTask(&time5S, 5000/UART_PERIOD, &Timer5sTouch);                       //5s定时任务
        time10S = TimerTask(&time10S, 10000/UART_PERIOD, &Timer10sTouch);                       //5s定时任务
        time100ms = TimerTask(&time100ms, 100/UART_PERIOD, &Timer100msTouch);                       //100ms定时任务

        //50ms
        {
            //单品功能:支持检测外部电平变化
            if(YES == startCheckIo)
            {
                IoCheckProgram(uart2_serial, UART_PERIOD);
            }

            //1.串口1为和hub 通讯
            if(YES == uart1_msg.messageFlag)
            {
                if(REGISTER_CODE == uart1_msg.data[0])
                {
                    //1.如果uuid 符合则认为是回复注册命令
                    if(uart1_msg.size >= 6)
                    {
                        uuid = 0;
                        uuid |= uart1_msg.data[2] << 24;
                        uuid |= uart1_msg.data[3] << 16;
                        uuid |= uart1_msg.data[4] << 8;
                        uuid |= uart1_msg.data[5];
                        ReadUniqueId(&id);
                        if(uuid == id)
                        {
                            //2.收到注册回复消息
                            getModuleInfo()->register_state = REGISTER_SUCESS;
                            getModuleInfo()->addr = uart1_msg.data[7];
                        }
                        setMasterEvent(EVENT_CLEAN);
                    }
                }
                else if(getModuleInfo()->addr == uart1_msg.data[0])
                {
                    analyzeMasterData(uart1_msg.data, uart1_msg.size);
                    //解析数据
                    getModuleInfo()->register_state = REGISTER_SUCESS;
                    setMasterEvent(EVENT_CLEAN);
                }

                uart1_msg.messageFlag = NO;
            }

            //2.串口2为和空调通讯
            if(YES == uart2_msg.messageFlag)
            {
                switch (infrared_event.event) {
                    case EVENT_SEND_MATCH:
                        //1.如果返回的命令符合码库格式长度，则接受
                        if(uart2_msg.size == sizeof(type_codedata_recv))
                        {
                            codeDataRecvResult(uart2_msg.data);
                        }

                        if(uart2_msg.size == sizeof(type_recv_match))
                        {
                            rt_memcpy((u8 *)&recv_match, uart2_msg.data, sizeof(type_recv_match));
                            //4.接受到匹配的代号码
                            if(((recv_match.start_code[0] << 8) | recv_match.start_code[1]) &&
                               (0x41 == recv_match.command))
                            {
                                getModuleInfo()->brand_code =
                                  (recv_match.brand_code[0] << 8) | recv_match.brand_code[1];

                                //触发将代号码写入码库配置
                                infrared_event.event = EVENT_SEND_SAVE;
                                infrared_event.call_back(INFRARED_EVENT_MATCH_OK);
                                setinfraredEvent(infrared_event);

                            }
                        }
                        break;

                    case EVENT_SEND_SAVE:
                        if(uart2_msg.size == sizeof(type_codedata_recv))
                        {
                            codeDataRecvResult(uart2_msg.data);
                            //保存之后结束流程
                            infrared_event.event = EVENT_NULL;
                            infrared_event.call_back(INFRARED_EVENT_SAVE_OK);
                            setinfraredEvent(infrared_event);
                        }
                        break;
                    //接收进入学习模式的命令
                    case EVENT_SEND_LEARN_MODE_ON:
                        if(uart2_msg.size == sizeof(type_codedata_recv))
                        {
                            rt_memcpy((u8 *)&recvResult, uart2_msg.data, uart2_msg.size);
                            if(1 == recvResult.state_code)
                            {
                                //回复已经进入学习模式
                                infrared_event.call_back(CB_IN_LEARN_ON);
                                setinfraredEvent(infrared_event);
                            }
                            else if(3 == recvResult.state_code)
                            {

                                infrared_event.event = EVENT_NULL;
                                setinfraredEvent(infrared_event);
                                pageReturn();
                            }
                        }
                        break;

                    case EVENT_SEND_LEARN_MODE_OFF:
                        if(uart2_msg.size == sizeof(type_codedata_recv))
                        {
                            rt_memcpy((u8 *)&recvResult, uart2_msg.data, uart2_msg.size);
                            if(1 == recvResult.state_code)
                            {
                                //回复已经进入学习模式
                                infrared_event.call_back(CB_IN_LEARN_OFF);
                                setinfraredEvent(infrared_event);
                            }
                            else if(3 == recvResult.state_code)
                            {

                                infrared_event.event = EVENT_NULL;
                                setinfraredEvent(infrared_event);
                                pageReturn();
                            }
                        }
                        break;

                    default:
                        break;
                }
                //关闭数据接收标志
                 uart2_msg.messageFlag = NO;
            }

            //3.如果module info有变化需要存储到flash
            if((modulePre.brand_code != getModuleInfo()->brand_code) ||
               (modulePre.addr != getModuleInfo()->addr) ||
               (modulePre.select != getModuleInfo()->select))
            {
                rt_memcpy((u8 *)&modulePre, (u8 *)getModuleInfo(), sizeof(module_info_t));

                setDataToFlah(DATA_BLOCK_START, (u8 *)getModuleInfo(), sizeof(module_info_t));
            }

            //4.如果收到控制指令的话需要发送,
            if(ctrl_pre != getModuleInfo()->ctrl)
            {
                ctrl_pre = getModuleInfo()->ctrl;

                sendCtrlCommandToCodeData(uart2_serial);
            }

        }

        if(YES == Timer100msTouch)
        {
            if(YES == getModuleInfo()->find_location)
            {
                //闪烁60秒
                if(find_location_cnt < 600)
                {
                    find_location_cnt ++;
                    LED_TEST();

                }
                else
                {
                    find_location_cnt = 0;
                    getModuleInfo()->find_location = NO;
                }
            }
        }

        //1s event
        //流程 发送匹配命令 接受匹配结果
        if(YES == Timer1sTouch)
        {
            infrared_event = getInfraredEvent();
            master_event = getMasterEvent();

            //和空调通讯部分
            if(EVENT_NULL != infrared_event.event)
            {
                switch (infrared_event.event) {
                    case EVENT_SEND_MATCH:
                        //发送匹配命令
                        sendMatchDataToCodeData(uart2_serial);
                        break;

                    case EVENT_SEND_SAVE:
                        //AA 55 09 91 00 24 00 82 21 99 07 7E
                        sendSaveCommandToCodeData(uart2_serial);
                        break;

                    case EVENT_SEND_ON_TEST:
                        if(SELECT_MATCH == getModuleInfo()->select)
                        {
                            sendMatchOnTest(uart2_serial);
                        }
                        else if(SELECT_LEARN == getModuleInfo()->select)
                        {
                            sendlearnOn(uart2_serial);
                        }

                        infrared_event.event = EVENT_NULL;
                        setinfraredEvent(infrared_event);
                        break;

                    case EVENT_SEND_OFF_TEST:
                        if(SELECT_MATCH == getModuleInfo()->select)
                        {
                            sendMatchOffTest(uart2_serial);
                        }
                        else
                        {
                            sendlearnOff(uart2_serial);
                        }

                        infrared_event.event = EVENT_NULL;
                        setinfraredEvent(infrared_event);
                        break;

                    case EVENT_SEND_LEARN_MODE_ON:
                        //1.发送进入学习模式
                        sendInLearnOnMode(uart2_serial);
                        break;

                    case EVENT_SEND_LEARN_MODE_OFF:
                        //1.发送进入学习模式
                        sendInLearnOffMode(uart2_serial);
                        break;
                    case EVENT_SEND_MATCH_OVER:
                        sendMatchOver(uart2_serial);
                        infrared_event.event = EVENT_NULL;
                        setinfraredEvent(infrared_event);
                        break;
                    case EVENT_SEND_LEARN_ON_OVER:
                        sendLearnOnOver(uart2_serial);
                        infrared_event.event = EVENT_NULL;
                        setinfraredEvent(infrared_event);
                        break;
                    case EVENT_SEND_LEARN_OFF_OVER:
                        sendLearnOffOver(uart2_serial);
                        infrared_event.event = EVENT_NULL;
                        setinfraredEvent(infrared_event);
                        break;
                    default:
                        break;
                }
            }

            //灯光部分
            if(NO == getModuleInfo()->find_location)
            {
                //如果ctrl是on 的时候灯光常开
                if(getModuleInfo()->ctrl >> 15)
                {
                    LED_ON();
                }
                else
                {
                    LED_TEST();
                }
            }

            if(NO == startFlag)
            {
                sendRegisterToMaster(uart1_serial, data);
                startFlag = YES;
            }
        }

        //5s事件
        if(YES == Timer5sTouch)
        {
            //1.如果主机没有发送修改命令或者控制命令过来，则间隔5s向主机发送一次
            if(REGISTER_SUCESS != getModuleInfo()->register_state)
            {
                setMasterEvent(EVENT_REGISTER);
            }
        }

        //10s事件
        if(YES == Timer10sTouch)
        {
            //和hub通讯部分
            switch (master_event)
            {
                //1.发送注册
                case EVENT_REGISTER:
                    //1.发送注册码给主机
                    sendRegisterToMaster(uart1_serial, data);
                    break;
                default:
                    break;
            }

            startCheckIo = YES;
        }

        rt_thread_mdelay(UART_PERIOD);
    }
}

void UartTaskInit(void)
{
    rt_err_t threadStart = RT_NULL;

    /* 创建串口 线程 */
    rt_thread_t thread = rt_thread_create(UART2_THREAD_NAME, UartTaskEntry, RT_NULL, 1024*4, UART2_PRIORITY, 10);

    /* 如果线程创建成功则开始启动线程，否则提示线程创建失败 */
    if (RT_NULL != thread) {
        threadStart = rt_thread_startup(thread);
        if (RT_EOK != threadStart) {
            LOG_E("sensor task start failed");
        }
    } else {
        LOG_E("sensor task create failed");
    }
}

void sendMessageToMaster(u8 *data, u8 len)
{
    rt_pin_write(UART1_EN, YES);
    rt_device_write(rt_device_find(DEVICE_UART1), 0, data, len);
    rt_pin_write(UART1_EN, NO);
}
