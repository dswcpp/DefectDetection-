/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * algorithm_pch.h - 预编译头文件
 *
 * 说明：将频繁使用但很少修改的头文件放在这里，
 *       可以显著加速编译（约 2-5 倍）
 */

#ifndef ALGORITHM_PCH_H
#define ALGORITHM_PCH_H

// ============ C++ 标准库 ============
#include <vector>
#include <map>
#include <memory>
#include <string>
#include <algorithm>
#include <cmath>
#include <functional>

// ============ Qt 核心 ============
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVariantMap>
#include <QObject>
#include <QMutex>
#include <QElapsedTimer>

// ============ OpenCV 核心（最耗时） ============
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

// ============ 项目公共头文件 ============
#include "algorithm_global.h"

#endif // ALGORITHM_PCH_H
