/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-09-03     Administrator       the first version
 */
#ifndef APPLICATIONS_OLED1309_OLEDPAGE_H_
#define APPLICATIONS_OLED1309_OLEDPAGE_H_

#include "Gpio.h"

void matchCallBack(u8);
void cleanLastMatchRes(void);
void SettingPage(type_page_t);
void oneKeyMatchPage(type_page_t *);
void nullCallBack(u8);
void testPage(type_page_t *);
void sendOnTestCallBack(u8);
void pageReturn(void);
void learnPage(type_page_t);
void learnOnPage(type_page_t);
void learnOffPage(type_page_t);
void learnCallBack(u8);
void selectModePage(type_page_t *);
void HomePage(type_page_t);
#endif /* APPLICATIONS_OLED1309_OLEDPAGE_H_ */
