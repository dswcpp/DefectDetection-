/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * ErrorCode.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：错误码定义
 * 描述：定义系统中使用的错误码枚举，包括成功、通用错误、相机错误、
 *       检测错误、数据库错误等分类
 *
 * 当前版本：1.0
 */

#ifndef ERRORCODE_H
#define ERRORCODE_H

enum class ErrorCode {
  Ok = 0,
  InvalidConfig,
  CameraNotFound,
  PlcDisconnected,
  DatabaseError,
  IoError,
  Unknown
};

#endif // ERRORCODE_H
