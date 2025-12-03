/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * ui_global.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：UI模块全局宏定义
 * 描述：定义UI_LIBRARY导出宏，用于Windows DLL导出符号控制
 *
 * 当前版本：1.0
 */

#ifndef UI_GLOBAL_H
#define UI_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(UI_LIBRARY_BUILD)
#  define UI_LIBRARY Q_DECL_EXPORT
#else
#  define UI_LIBRARY Q_DECL_IMPORT
#endif

#endif // UI_GLOBAL_H
