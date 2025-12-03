/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * network_global.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：网络模块全局宏定义
 * 描述：定义NETWORK_LIBRARY导出宏，用于Windows DLL导出符号控制
 *
 * 当前版本：1.0
 */

#ifndef NETWORK_GLOBAL_H
#define NETWORK_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(NETWORK_LIBRARY_BUILD)
#  define NETWORK_LIBRARY Q_DECL_EXPORT
#else
#  define NETWORK_LIBRARY Q_DECL_IMPORT
#endif

#endif // NETWORK_GLOBAL_H
