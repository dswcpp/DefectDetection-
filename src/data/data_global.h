/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * data_global.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：数据模块全局宏定义
 * 描述：定义DATA_LIBRARY导出宏，用于Windows DLL导出符号控制
 *
 * 当前版本：1.0
 */

#ifndef DATA_GLOBAL_H
#define DATA_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(DATA_LIBRARY_BUILD)
#  define DATA_EXPORT Q_DECL_EXPORT
#else
#  define DATA_EXPORT Q_DECL_IMPORT
#endif

#endif // DATA_GLOBAL_H
