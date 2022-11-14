/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-01-21     Administrator       the first version
 */
#ifndef APPLICATIONS_OLED1309_H_

#define APPLICATIONS_OLED1309_H_

#include "Gpio.h"

//设计只有三级界面
struct pageSelect
{
    u8  cusor_home;
    u8  cusor_max;
    u8  cusor;
    u8  select;
};

#define     LINE_HIGHT              8
#define     COLUMN_HIGHT            13          //实际是13为了留间距

//一级界面
#define     HOME_PAGE               0x00        //主页面

//
#define     SETTING_PAGE            0x01        //主页面
#define     ONEKEY_MATCH_PAGE       0x02        //一键匹配             页面
#define     SELECT_MODE_PAGE        0x03        //选择                    页面
#define     LEARN_MODE_PAGE         0x04        //学习页面
#define     LEARN_ON_PAGE           0x05        //learn on      页面
#define     LEARN_OFF_PAGE          0x06        //learn off     页面
#define     TEST_PAGE               0x07        //发送测试


/****************界面显示内容****************************************/
#define     ONEKEY_MATCH_NAME       "One Key Match"
#define     SELECT_MODE_NAME        "Select Mode"
#define     LEARN_MODE_NAME         "Learn Mode"
#define     LEARN_ON_NAME           "Learn On"
#define     LEARN_OFF_NAME          "Learn Off"
#define     TEST_NAME               "Test Command"

/******************************************************************/

enum{
    RES_NULL = 0x00,
    RES_SUCCESS = 0x01,
    RES_FAIL
};

typedef     struct pageSelect               type_page_t;

void oledInit(void);
void OledTaskEntry(void*);
void OledTaskInit(void);
void MenuBtnCallBack(u8);
void EnterBtnCallBack(u8);
void UpBtnCallBack(u8);
void DowmBtnCallBack(u8);
void clear_screen(void);
#endif /* APPLICATIONS_OLED1309_H_ */
