/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * algorithm_global.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：算法模块全局宏定义
 * 描述：定义ALGORITHM_LIBRARY导出宏，用于Windows DLL导出符号控制
 *
 * 当前版本：1.0
 */

#ifndef ALGORITHM_GLOBAL_H
#define ALGORITHM_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(ALGORITHM_LIBRARY_BUILD)
#  define ALGORITHM_EXPORT Q_DECL_EXPORT
#else
#  define ALGORITHM_EXPORT Q_DECL_IMPORT
#endif

// 兼容别名
#define ALGORITHM_LIBRARY ALGORITHM_EXPORT

#endif // ALGORITHM_GLOBAL_H
