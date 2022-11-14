/*
#include <Oled1309.h>
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-01-21     Administrator       the first version
 */
#ifdef __cplusplus
extern "C" {
#endif

#include <rtthread.h>

#include "Oled1309.h"
#include "Uart.h"
#include "ButtonTask.h"
#include "ascii_fonts.h"
#include "ST7567.h"
#include "OledPage.h"
#include "infrared.h"

#define  GO_RIGHT  1
#define  GO_LEFT   2

u8              reflash_flag        = NO;
type_page_t     pageSelect;
u32             pageInfor       = 0x00000000;   //只支持最多四级目录
time_t          backlightTime;

#define OLED_SPI_PIN_CLK                    29//PB13
#define OLED_SPI_PIN_MOSI                   31//PB15
#define OLED_SPI_PIN_RES                    22//PB6
#define OLED_SPI_PIN_DC                     23//PB7
#define OLED_SPI_PIN_CS                     21//PB5
#define OLED_BACK_LIGHT                     24//PB8

void clear_screen(void)
{
  ST7567_Fill(0);
//  ST7567_UpdateScreen();
}

void st7567Init(void)
{
    ST7567_Init();
    rt_thread_mdelay(100);
    clear_screen();
}

void LCD_Test(void)
{
    ST7567_GotoXY(5, 5);
    ST7567_Puts("ABCDEFGHIJKLMNOPQRSTUVWXYZ", &Font_5x7, 1);
    ST7567_UpdateScreen();
}

//唤醒屏幕背景光
void wakeUpOledBackLight(time_t *time)
{
    //*time = getTimeStamp();
    *time = 0;

    rt_pin_write(OLED_BACK_LIGHT, YES);
}

//监控无操作1min 后熄屏幕
void monitorBackLight(time_t *time)
{
//    if(time + 60  < getTimeStamp())     //getTimeStamp单位S
    if(*time < 60)
    {
        *time += 1;
    }
    else
    {
        rt_pin_write(OLED_BACK_LIGHT, NO);
    }
}

/**
 * ：确认键
 */
void EnterBtnCallBack(u8 type)
{
    u8 info = 0;
    infrared_event_t event;

    if(SHORT_PRESS == type)
    {
        //唤醒屏幕
        wakeUpOledBackLight(&backlightTime);
        clear_screen();
        pageSelect.select = YES;
        //提示界面刷新
        reflash_flag = YES;
    }
    else if(LONG_PRESS == type)
    {
        info = pageInfor;
        if(ONEKEY_MATCH_PAGE == info)
        {
            //如果从一键匹配页面返回 则清除上次匹配结果
            cleanLastMatchRes();
            //退出后不再一直发送匹配
            event.event = EVENT_SEND_MATCH_OVER;
            setinfraredEvent(event);
        }
        else if(LEARN_ON_PAGE == info)
        {
            //退出后不再一直发送匹配
            event.event = EVENT_SEND_LEARN_ON_OVER;
            setinfraredEvent(event);
        }
        else if(LEARN_OFF_PAGE == info)
        {
            //退出后不再一直发送匹配
            event.event = EVENT_SEND_LEARN_OFF_OVER;
            setinfraredEvent(event);
        }

        pageInfor = pageInfor >> 8;
    }
}

/**
 * ：向上键
 */
void UpBtnCallBack(u8 type)
{
    if(SHORT_PRESS == type)
    {
        //唤醒屏幕
        wakeUpOledBackLight(&backlightTime);
        clear_screen();
        if(pageSelect.cusor > pageSelect.cusor_home)
        {
            pageSelect.cusor--;
        }
        else
        {
            pageSelect.cusor = pageSelect.cusor_max;
        }
        //提示界面刷新
        reflash_flag = YES;
        LOG_D("cusor %d",pageSelect.cusor);
    }
}

/**
 * ：向下键
 */
void DowmBtnCallBack(u8 type)
{
    if(SHORT_PRESS == type)
    {
        //唤醒屏幕
        wakeUpOledBackLight(&backlightTime);
        clear_screen();
        if(pageSelect.cusor_max > 0)
        {
            if(pageSelect.cusor < pageSelect.cusor_max)
            {
                pageSelect.cusor++;
            }
            else
            {
                pageSelect.cusor = pageSelect.cusor_home;
            }
        }
        //提示界面刷新
        reflash_flag = YES;
    }
}

void pageSelectSet(u8 home, u8 max)
{
    pageSelect.cusor_home = home;
    pageSelect.cusor = pageSelect.cusor_home;
    pageSelect.cusor_max = max;
}

static void pageSetting(u8 page)
{
    switch (page)
    {
        case HOME_PAGE:
            pageSelectSet(1, 1);
            break;
        case SETTING_PAGE:
            pageSelectSet(1, 4);
            break;
        case LEARN_MODE_PAGE:
            pageSelectSet(1, 2);
            break;
        case SELECT_MODE_PAGE:
            pageSelectSet(1, 2);
            pageSelect.cusor = getModuleInfo()->select == SELECT_MATCH ? 1 : 2;
            break;
        case TEST_PAGE:
            pageSelectSet(1, 2);
            break;
        default:
            break;
    }
}

static void pageProgram(u8 page)
{

    switch (page)
    {
        case HOME_PAGE:
            HomePage(pageSelect);
            if(YES == pageSelect.select)
            {
                if(1 == pageSelect.cusor)
                {
                    pageInfor <<= 8;
                    pageInfor |= SETTING_PAGE;
                }
                pageSelect.select = NO;
            }
            break;
        case SETTING_PAGE:
            SettingPage(pageSelect);
            if(YES == pageSelect.select)
            {
                if(1 == pageSelect.cusor)
                {
                    pageInfor <<= 8;
                    pageInfor |= SELECT_MODE_PAGE;
                }
                else if(2 == pageSelect.cusor)
                {
                    pageInfor <<= 8;
                    pageInfor |= ONEKEY_MATCH_PAGE;
                }
                else if(3 == pageSelect.cusor)
                {
                    pageInfor <<= 8;
                    pageInfor |= LEARN_MODE_PAGE;
                }
                else if(4 == pageSelect.cusor)
                {
                    pageInfor <<= 8;
                    pageInfor |= TEST_PAGE;
                }

                pageSelect.select = NO;
            }
            break;
        case ONEKEY_MATCH_PAGE:
            oneKeyMatchPage(&pageSelect);
            break;
        case SELECT_MODE_PAGE:
            selectModePage(&pageSelect);
            break;
        case LEARN_MODE_PAGE:
            learnPage(pageSelect);
            if(YES == pageSelect.select)
            {
                if(1 == pageSelect.cusor)
                {
                    pageInfor <<= 8;
                    pageInfor |= LEARN_ON_PAGE;
                }
                else if(2 == pageSelect.cusor)
                {
                    pageInfor <<= 8;
                    pageInfor |= LEARN_OFF_PAGE;
                }
                pageSelect.select = NO;
            }
            break;

        case LEARN_ON_PAGE:
            learnOnPage(pageSelect);
            break;

        case LEARN_OFF_PAGE:
            learnOffPage(pageSelect);
            break;

        case TEST_PAGE:
            testPage(&pageSelect);
            break;
        default:
            break;
    }

    reflash_flag = NO;
}

void OledTaskEntry(void* parameter)
{
                u8              nowPage             = 0;
    static      u8              Timer1sTouch        = NO;
    static      u16             time1S              = 0;
    static      u8              Timer3sTouch        = NO;
    static      u16             time3S              = 0;
    static      u8              nowPagePre          = 0xFF;
    static      u16             ctrl                = 0;

    st7567Init();
    pageInfor <<= 8;
    pageInfor |= HOME_PAGE;
    wakeUpOledBackLight(&backlightTime);
    while(1)
    {
        time1S = TimerTask(&time1S, 20, &Timer1sTouch);
        time3S = TimerTask(&time3S, 60, &Timer3sTouch);

        //1.监听页面是否更新,同时响应各个页面的显示
        nowPage = pageInfor & 0x000000FF;

        if(nowPagePre != nowPage)
        {
            //1.1更新页面标签
            pageSetting(nowPage);
            //1.2页面展示
            pageProgram(nowPage);

            nowPagePre = nowPage;
        }
        else
        {
            //1.3周期更新页面事件
            if(YES == reflash_flag)
            {
                pageProgram(nowPage);
            }
        }

        if(ctrl != getModuleInfo()->ctrl)
        {
            ctrl = getModuleInfo()->ctrl;

            reflash_flag = YES;
        }

        //1s event
        if(YES == Timer1sTouch)
        {
            monitorBackLight(&backlightTime);
            if(SELECT_MODE_PAGE == nowPage)
            {
                //需要刷新
                reflash_flag = YES;
            }
        }

        rt_thread_mdelay(50);
    }
}

void OledTaskInit(void)
{
    rt_err_t threadStart = RT_NULL;
    /* 创建串口 线程 */
    rt_thread_t thread = rt_thread_create(OLED_THREAD_NAME, OledTaskEntry, RT_NULL, 1024*2, OLED_PRIORITY, 10);

    /* 如果线程创建成功则开始启动线程，否则提示线程创建失败 */
    if (RT_NULL != thread) {
        threadStart = rt_thread_startup(thread);
        if (RT_EOK != threadStart) {
            LOG_E("oled task start failed");
        }
    } else {
        LOG_E("oled task create failed");
    }
}


#ifdef __cplusplus
}
#endif

