/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-09-03     Administrator       the first version
 */
#include "Gpio.h"
#include "Oled1309.h"
#include "OledPage.h"
#include "ST7567.h"
#include "ascii_fonts.h"
#include "Uart.h"
#include "infrared.h"

u8       matchRes    = RES_NULL;
extern u32             pageInfor;   //只支持最多四级目录

u8 getTempByCtrl(u16 data)
{
    u8 temp = 0;

    temp = (data >> 8) & 0x0f;
    temp += 16;

    if((temp > 30) || (temp < 16))
    {
        temp = 25;
    }

    return temp;
}

//主页内容
void HomePage(type_page_t page)
{
    char    temp[5] = " ";

    //1.清除之前屏幕内容
    clear_screen();

    //2.显示选择项 setting
    ST7567_GotoXY(0, 0);
    ST7567_Puts(" Setting             ", &Font_6x12, 0);

    //3.显示温度
    itoa(getTempByCtrl(getModuleInfo()->ctrl), temp, 10);
    ST7567_GotoXY(8, 22);
    ST7567_Puts(temp, &Font_16x32, 1);
    //显示单位
    ST7567_DrawCircle(42, 30, 1, 1);
    ST7567_GotoXY(44,30);
    ST7567_Puts("C", &Font_8x16, 1);

    //4.显示开关状态
    ST7567_GotoXY(78, 22);
    if((getModuleInfo()->ctrl & 0x8000) > 0)
    {
        ST7567_Puts("On", &Font_8x16, 1);
    }
    else
    {
        ST7567_Puts("Off", &Font_8x16, 1);
    }
    //显示分割线
    ST7567_DrawLine(8 + 3 * 16, 38, 127, 38, 1);
    //显示当前的模式
    ST7567_GotoXY(70, 39);

    if(SELECT_MATCH == getModuleInfo()->select)
    {
        ST7567_Puts("match", &Font_8x16, 1);
    }
    else if(SELECT_LEARN == getModuleInfo()->select)
    {
        ST7567_Puts("learn", &Font_8x16, 1);
    }
//    //5.显示brand code
//    itoa(getModuleInfo()->brand_code, temp, 10);
//    ST7567_GotoXY(8 , 16 * 3);
//    ST7567_Puts("key code: ", &Font_8x16, 1);
//    ST7567_GotoXY(8 + 10 * 8, 16 * 3);
//    ST7567_Puts(temp, &Font_8x16, 1);
    //6.刷新屏幕
    ST7567_UpdateScreen();
}

void SettingPage(type_page_t page)
{
    char show[4][16] = {SELECT_MODE_NAME,ONEKEY_MATCH_NAME,LEARN_MODE_NAME,TEST_NAME};
    u8 column = 0;

    //1.清除之前屏幕内容
    clear_screen();

//    //2.显示进入学习模式选项
    column = 16;

    if(page.cusor <= 3)
    {
        for(u8 index = 0; index < 3; index++)
        {
            ST7567_GotoXY(8, column + index * 16);
            //显示选中
            if(page.cusor == index + 1)
            {
                ST7567_Puts(show[index], &Font_8x16, 0);
            }
            else
            {
                ST7567_Puts(show[index], &Font_8x16, 1);
            }
        }
    }
    else
    {
        for(u8 index = 0; index < 3; index++)
        {
            ST7567_GotoXY(8, column + index * 16);
            //显示选中
            if(page.cusor == index + 1 + page.cusor - 3)
            {
                ST7567_Puts(show[index + page.cusor - 3], &Font_8x16, 0);
            }
            else
            {
                ST7567_Puts(show[index + page.cusor - 3], &Font_8x16, 1);
            }
        }
    }

    //3.刷新屏幕
    ST7567_UpdateScreen();
}

/**学习页面显示内容
 * ---------------------------------------
 * | 2022/09/06 21:15
 * | One Key Learn
 * | Learn ON
 * | Learn Off
 * ---------------------------------------
 */
void oneKeyMatchPage(type_page_t *page)
{
    infrared_event_t    event;

    //1.清除之前屏幕内容
    clear_screen();

    //2.显示正在匹配
    ST7567_GotoXY(8, 16 * 1);
    ST7567_Puts("Matching Now", &Font_8x16, 0);

    //3.刷新屏幕
    ST7567_UpdateScreen();

    //4.一键匹配功能,此时串口发送配对命令
    event.event = EVENT_SEND_MATCH;
    event.call_back = matchCallBack;
    setinfraredEvent(event);
}

void selectModePage(type_page_t *page)
{
    static      u8      cnt         = 0;
    static      char    show[6]     = " ";

    //1.清除之前屏幕内容
    clear_screen();

    //2.按键选择
    if(1 == page->cusor)
    {
        strcpy(show,"match");
    }
    else if(2 == page->cusor)
    {
        strcpy(show,"learn");
    }

    //3.选择模式
    if(YES == page->select)
    {
        if(1 == page->cusor)
        {
            getModuleInfo()->select = SELECT_MATCH;
        }
        else if(2 == page->cusor)
        {
            getModuleInfo()->select = SELECT_LEARN;
        }

        pageReturn();

        page->select = NO;
    }

    //2.显示正在匹配
    ST7567_GotoXY(8, 16 * 1);
    ST7567_Puts("select", &Font_8x16, 1);
    ST7567_GotoXY(8*8, 16 * 1);
    ST7567_Puts(show, &Font_8x16, cnt++ % 2 ? 0 : 1);

    //3.刷新屏幕
    ST7567_UpdateScreen();
}

void testPage(type_page_t *page)
{
    infrared_event_t    event;

    //1.清除之前屏幕内容
    clear_screen();

    //2.显示正在匹配
    ST7567_GotoXY(8, 16 * 1);
    ST7567_Puts("Send On Command", &Font_8x16, page->cusor == 1 ? 0 : 1);
    ST7567_GotoXY(8, 16 * 2);
    ST7567_Puts("Send Off Command", &Font_8x16, page->cusor == 2 ? 0 : 1);

    //3.刷新屏幕
    ST7567_UpdateScreen();

    //4.设置事件
    if(YES == page->select)
    {
        if(1 == page->cusor)
        {

            event.event = EVENT_SEND_ON_TEST;
            setinfraredEvent(event);
        }
        else if(2 == page->cusor)
        {

            event.event = EVENT_SEND_OFF_TEST;
            setinfraredEvent(event);
        }

        page->select = NO;
    }
}

void learnPage(type_page_t page)
{
//    infrared_event_t event;

    //1.清除之前屏幕内容
    clear_screen();

    //2.显示学习命令
    ST7567_GotoXY(8, 16 * 1);
    ST7567_Puts("learn on", &Font_8x16, (page.cusor == 1) ? 0 : 1);
    ST7567_GotoXY(8, 16 * 2);
    ST7567_Puts("learn off", &Font_8x16, (page.cusor == 2) ? 0 : 1);

    //3.刷新屏幕
    ST7567_UpdateScreen();
}

void learnOnPage(type_page_t page)
{
    infrared_event_t event;

    //1.清除之前屏幕内容
    clear_screen();

    //2.显示学习命令
    ST7567_GotoXY(8, 16 * 1);
    ST7567_Puts("start send on", &Font_8x16, 1);
    ST7567_GotoXY(8, 16 * 2);
    ST7567_Puts("command", &Font_8x16, 1);

    //3.刷新屏幕
    ST7567_UpdateScreen();

    //4.发送进去学习模块
    event.event = EVENT_SEND_LEARN_MODE_ON;
    event.call_back = learnCallBack;
    setinfraredEvent(event);
}

void learnOffPage(type_page_t page)
{
    infrared_event_t event;

    //1.清除之前屏幕内容
    clear_screen();

    //2.显示学习命令
    ST7567_GotoXY(8, 16 * 1);
    ST7567_Puts("start send off", &Font_8x16, 1);
    ST7567_GotoXY(8, 16 * 2);
    ST7567_Puts("command", &Font_8x16, 1);

    //3.刷新屏幕
    ST7567_UpdateScreen();

    //4.发送进去学习模块
    event.event = EVENT_SEND_LEARN_MODE_OFF;
    event.call_back = learnCallBack;
    setinfraredEvent(event);
}

void pageReturn(void)
{
    pageInfor >>= 8;
}

//回调函数
void matchCallBack(u8 result)
{
    char brand_code[5];
    if(result == INFRARED_EVENT_MATCH_OK)
    {
        //1.清除之前屏幕内容
        clear_screen();
        //2.显示结果
        ST7567_GotoXY(8, 16 * 1);
        ST7567_Puts("Match Sucess", &Font_8x16, 0);
        //3.刷新屏幕
        ST7567_UpdateScreen();
    }
    else if(result == INFRARED_EVENT_MATCH_FAIL)
    {
        //1.清除之前屏幕内容
        clear_screen();
        //2.显示结果
        ST7567_GotoXY(8, 16 * 1);
        ST7567_Puts("Match Fail", &Font_8x16, 0);
        //3.刷新屏幕
        ST7567_UpdateScreen();
    }
    else if(result == INFRARED_EVENT_SAVE_OK)
    {
        //1.清除之前屏幕内容
        clear_screen();
        //2.显示结果
        itoa(getModuleInfo()->brand_code,brand_code,10);
        ST7567_GotoXY(8, 16 * 1);
        ST7567_Puts("Save Sucess", &Font_8x16, 0);
        ST7567_GotoXY(8, 16 * 2);
        ST7567_Puts("key code = ", &Font_8x16, 0);
        ST7567_GotoXY(12 * 8, 16 * 2);
        ST7567_Puts(brand_code, &Font_8x16, 0);
        //3.刷新屏幕
        ST7567_UpdateScreen();

        //4.退出界面
        rt_thread_mdelay(1000);
        pageReturn();
    }
    else if(result == INFRARED_EVENT_SAVE_FAIL)
    {
        //1.清除之前屏幕内容
        clear_screen();
        //2.显示结果
        ST7567_GotoXY(8, 16 * 1);
        ST7567_Puts("Save Fail", &Font_8x16, 0);
        //3.刷新屏幕
        ST7567_UpdateScreen();
    }
}

void nullCallBack(u8 result)
{

}

void sendOnTestCallBack(u8 result)
{
    if(YES == result)
    {
        //1.清除之前屏幕内容
        clear_screen();
        //2.显示发送off测试
        ST7567_GotoXY(8, 16 * 1);
        ST7567_Puts("Send Off Command", &Font_8x16, 0);
        //3.刷新屏幕
        ST7567_UpdateScreen();
    }
}

//学习回调函数
void learnCallBack(u8 result)
{
    switch (result) {
        case CB_IN_LEARN_ON:

            //1.清除之前屏幕内容
            clear_screen();

            //2.显示学习命令
            ST7567_GotoXY(8, 16 * 1);
            ST7567_Puts("Press the on", &Font_8x16, 1);
            ST7567_GotoXY(8, 16 * 2);
            ST7567_Puts("key, please", &Font_8x16, 1);

            //3.刷新屏幕
            ST7567_UpdateScreen();
            break;
        case CB_IN_LEARN_OFF:

            //1.清除之前屏幕内容
            clear_screen();

            //2.显示学习命令
            ST7567_GotoXY(8, 16 * 1);
            ST7567_Puts("Press the off", &Font_8x16, 1);
            ST7567_GotoXY(8, 16 * 2);
            ST7567_Puts("key, please", &Font_8x16, 1);

            //3.刷新屏幕
            ST7567_UpdateScreen();
            break;
        default:
            break;
    }
}

void cleanLastMatchRes(void)
{
    matchRes = RES_NULL;
}
