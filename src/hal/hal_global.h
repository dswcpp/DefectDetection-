/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * hal_global.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：硬件抽象层全局宏定义
 * 描述：定义HAL_LIBRARY导出宏，用于Windows DLL导出符号控制
 *
 * 当前版本：1.0
 */

#ifndef HAL_GLOBAL_H
#define HAL_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(HAL_LIBRARY_BUILD)
#  define HAL_EXPORT Q_DECL_EXPORT
#else
#  define HAL_EXPORT Q_DECL_IMPORT
#endif

#endif // HAL_GLOBAL_H
