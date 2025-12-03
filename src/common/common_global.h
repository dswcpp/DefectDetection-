/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * common_global.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：通用模块全局宏定义
 * 描述：定义COMMON_LIBRARY导出宏，用于Windows DLL导出符号控制
 *
 * 当前版本：1.0
 */

#ifndef COMMON_GLOBAL_H
#define COMMON_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(COMMON_LIBRARY_BUILD)
#  define COMMON_LIBRARY Q_DECL_EXPORT
#else
#  define COMMON_LIBRARY Q_DECL_IMPORT
#endif

#endif // COMMON_GLOBAL_H
