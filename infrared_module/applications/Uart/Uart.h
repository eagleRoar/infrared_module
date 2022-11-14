/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-01-12     Qiuyijie     the first version
 */
#ifndef APPLICATIONS_UART_H_
#define APPLICATIONS_UART_H_

#include "Gpio.h"
#include "Oled1309.h"
#include "module.h"

#define             DEVICE_UART1            "uart1"
#define             DEVICE_UART2            "uart2"
#define             DEVICE_UART3            "uart3"

#define             UART_MSG_SIZE           100//512
#define             CONNRCT_MISS_MAX        5

#define             START_CODE              0xAA55

#define             HAVE_SAVE_FLAG          0xA5A55A5A

//定义flash 数据区间的开始地址
#define             DATA_BLOCK_START        0x801FC00

/* 串口接收消息结构*/
struct rx_msg
{
    rt_device_t dev;
    rt_size_t size;
    u8 messageFlag;
    u8 data[UART_MSG_SIZE];                 //保存串口数据
};

//码库芯片返回格式
struct codeDataRecv{
    u16 start_code;         //前导码
    u8  len;                //长度
    u8  command;            //命令
    u8  static_code;        //固定码  返回当前的状态
    u8  state_code;         //状态码
    u8  check;              //异或校验码
};

enum
{
    CODE_RECV_FAIL = 0X00,
    CODE_RECV_SUCCESS,
    CODE_RECV_TIMEOUT
};

//码库返回是否匹配成功
struct codeDataRecvMatch{
    u8  start_code[2];      //前导码
    u8  len;                //长度
    u8  command;            //命令
    u8  brand_code[2];         //品牌代号码
    u8  check;              //异或校验码
};

//Modbus 命令
#define     READ_MUTI           0x03
#define     WRITE_SINGLE        0x06
#define     WRITE_MUTI          0x10

//
#define             REGISTER_CODE           0xFA
#define             INFRARED_TYPE           0xB4

//寄存器
#define             REGISTER_LOCATION       0x0000
#define             REGISTER_VERSION        0x0001      //版本号
#define             REGISTER_UUID1          0x0003      //uuid
#define             REGISTER_UUID2          0x0004      //uuid
#define             REGISTER_ADDR           0x0006      //地址
#define             REGISTER_TYPE           0x0007      //类型
#define             REGISTER_CTRL           0x0100      //控制红外的状态


//结构体存储
struct module_info
{
    u16     crc;                        //标志已经有存储了,在第一次有使用
    u8      addr;                       //modbus 通讯地址
    u8      type;                       //type
    u8      version;                    //版本号
    u8      register_state;             //注册状态
    u16     brand_code;                 //接收到的代号码
    u16     ctrl;
    u8      select;                     //选择使用match 和 learn
    u8      find_location;              //定位
};

typedef struct module_info module_info_t;

/************************************************/
enum
{
    EVENT_NULL,
    EVENT_SEND_MATCH = 0x01,
    EVENT_SEND_MATCH_OVER,
    EVENT_SEND_SAVE,             //保存代号码到码库
    EVENT_SEND_ON_TEST,
    EVENT_SEND_OFF_TEST,
    EVENT_SEND_LEARN_MODE_ON,
    EVENT_SEND_LEARN_MODE_OFF,
    EVENT_WAIT_LEARN_ON,
    EVENT_WAIT_LEARN_OFF,
    EVENT_SEND_LEARN_ON_OVER,
    EVENT_SEND_LEARN_OFF_OVER,
};

/***********************************************/
enum
{
    EVENT_CLEAN,
    EVENT_REGISTER = 0x01,
    EVENT_GET_ADDR,
    EVENT_SET_ADDR,
};

enum
{
    CB_IN_LEARN_ON = 0x01,
    CB_IN_LEARN_OFF,
    CB_LEARN_ON_OK,
    CB_LEARN_OFF_OK,
};

enum
{
    SELECT_MATCH = 0x00,
    SELECT_LEARN,
};

void initModuleInfo(void);
module_info_t *getModuleInfo(void);
void UartTaskEntry(void*);
void UartTaskInit(void);
void sendMessageToMaster(u8 *, u8);
#endif /* APPLICATIONS_UART_H_ */
