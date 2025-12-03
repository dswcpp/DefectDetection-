#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""批量为头文件添加版权声明（含详细描述）"""

import os
import re
from pathlib import Path

# 版权声明模板
COPYRIGHT_TEMPLATE = """/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * {filename}
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：{summary}
 * 描述：{description}
 *
 * 当前版本：1.0
 */

"""

# 文件详细描述映射
FILE_INFO = {
    # ==================== common模块 ====================
    "common_global.h": {
        "summary": "通用模块全局宏定义",
        "description": "定义COMMON_LIBRARY导出宏，用于Windows DLL导出符号控制"
    },
    "Logger.h": {
        "summary": "日志记录模块接口定义",
        "description": "基于spdlog封装的日志系统，支持多级别日志(DEBUG/INFO/WARN/ERROR)，\n *       支持控制台和文件输出，提供LOG_DEBUG/LOG_INFO等便捷宏"
    },
    "Utils.h": {
        "summary": "通用工具函数接口定义",
        "description": "提供常用工具函数：字符串处理、文件操作、时间格式化、UUID生成等"
    },
    "Types.h": {
        "summary": "通用类型定义",
        "description": "定义项目中使用的通用数据结构：DetectResult检测结果、DefectRegion缺陷区域、\n *       SeverityLevel严重等级、PerfStats性能统计等"
    },
    "Timer.h": {
        "summary": "计时器模块接口定义",
        "description": "高精度计时器类，用于性能测量，支持启动/停止/重置，返回毫秒级耗时"
    },
    "ThreadPool.h": {
        "summary": "线程池模块接口定义",
        "description": "基于std::thread的线程池实现，支持任务队列、动态线程数量、\n *       异步任务提交和future结果获取"
    },
    "SPSCQueue.h": {
        "summary": "单生产者单消费者无锁队列",
        "description": "高性能无锁队列模板类，适用于单线程生产、单线程消费场景，\n *       用于线程间高效数据传递"
    },
    "Singleton.h": {
        "summary": "单例模式模板类定义",
        "description": "CRTP单例模式实现，派生类继承此模板即可获得单例特性，\n *       线程安全，支持延迟初始化"
    },
    "ErrorCode.h": {
        "summary": "错误码定义",
        "description": "定义系统中使用的错误码枚举，包括成功、通用错误、相机错误、\n *       检测错误、数据库错误等分类"
    },
    "Constants.h": {
        "summary": "全局常量定义",
        "description": "定义系统级常量：默认超时时间、缓冲区大小、文件路径、版本号等"
    },
    "CircularBuffer.h": {
        "summary": "环形缓冲区模板类定义",
        "description": "固定大小的环形缓冲区，支持FIFO操作，用于数据流缓存、\n *       历史记录存储等场景"
    },
    "ConfigManager.h": {
        "summary": "配置管理模块接口定义",
        "description": "单例配置管理器，负责加载/保存JSON配置文件，提供类型安全的\n *       配置读写接口，支持配置变更通知"
    },
    "AppConfig.h": {
        "summary": "应用程序配置类定义",
        "description": "定义应用配置数据结构：相机配置CameraConfig、检测器配置DetectorsConfig、\n *       存储配置StorageConfig、用户配置UserConfig等"
    },
    
    # ==================== data模块 ====================
    "data_global.h": {
        "summary": "数据模块全局宏定义",
        "description": "定义DATA_LIBRARY导出宏，用于Windows DLL导出符号控制"
    },
    "DatabaseManager.h": {
        "summary": "数据库管理模块接口定义",
        "description": "SQLite数据库管理器，负责数据库连接、表创建、事务管理，\n *       提供统一的数据库访问接口"
    },
    "CSVExporter.h": {
        "summary": "CSV数据导出模块接口定义",
        "description": "将检测结果、缺陷记录导出为CSV格式文件，支持自定义列、\n *       编码格式、分隔符等"
    },
    "ExcelExporter.h": {
        "summary": "Excel数据导出模块接口定义",
        "description": "将检测数据导出为Excel格式(.xlsx)，支持多Sheet、样式设置、\n *       图表生成等功能"
    },
    "ReportGenerator.h": {
        "summary": "报表生成模块接口定义",
        "description": "生成检测报告文档，支持HTML/PDF格式，包含统计图表、\n *       缺陷详情、趋势分析等内容"
    },
    "ImageStorage.h": {
        "summary": "图像存储模块接口定义",
        "description": "管理检测图像的存储，支持原图/标注图保存、按日期分目录、\n *       自动清理过期文件等功能"
    },
    "BackupManager.h": {
        "summary": "数据备份管理模块接口定义",
        "description": "数据库和配置文件备份管理，支持定时备份、增量备份、\n *       备份恢复等功能"
    },
    "IRepository.h": {
        "summary": "数据仓储接口基类定义",
        "description": "Repository模式基类，定义CRUD操作接口，所有具体Repository继承此类"
    },
    "UserRepository.h": {
        "summary": "用户数据仓储接口定义",
        "description": "用户数据访问层，提供用户增删改查、密码验证、权限管理等功能"
    },
    "InspectionRepository.h": {
        "summary": "检测记录仓储接口定义",
        "description": "检测记录数据访问层，存储每次检测的结果、时间、操作员等信息"
    },
    "ImageRepository.h": {
        "summary": "图像数据仓储接口定义",
        "description": "图像元数据访问层，存储图像路径、关联的检测记录、缩略图等"
    },
    "DefectRepository.h": {
        "summary": "缺陷数据仓储接口定义",
        "description": "缺陷记录数据访问层，存储缺陷位置、类型、严重度、置信度等"
    },
    "ConfigRepository.h": {
        "summary": "配置数据仓储接口定义",
        "description": "配置数据访问层，支持配置版本管理、配置导入导出"
    },
    "AnnotationRepository.h": {
        "summary": "标注数据仓储接口定义",
        "description": "人工标注数据访问层，存储手动标注的缺陷信息，用于模型训练"
    },
    
    # ==================== hal模块 ====================
    "hal_global.h": {
        "summary": "硬件抽象层全局宏定义",
        "description": "定义HAL_LIBRARY导出宏，用于Windows DLL导出符号控制"
    },
    "ICamera.h": {
        "summary": "相机接口基类定义",
        "description": "相机抽象接口，定义open/close/grab/setExposure等通用方法，\n *       所有相机实现类继承此接口"
    },
    "CameraFactory.h": {
        "summary": "相机工厂类定义",
        "description": "相机对象工厂，根据类型字符串(hik/daheng/gige/usb/file)创建对应相机实例"
    },
    "FileCamera.h": {
        "summary": "文件相机模拟器接口定义",
        "description": "从本地图片文件夹读取图像的模拟相机，用于离线测试和调试"
    },
    "HikCamera.h": {
        "summary": "海康相机接口定义",
        "description": "海康威视工业相机SDK封装，支持参数设置、触发模式、图像采集"
    },
    "DahengCamera.h": {
        "summary": "大恒相机接口定义",
        "description": "大恒图像工业相机SDK封装，支持GigE和USB3接口相机"
    },
    "GigECamera.h": {
        "summary": "GigE相机接口定义",
        "description": "通用GigE Vision协议相机封装，基于GenICam标准"
    },
    "USBCamera.h": {
        "summary": "USB相机接口定义",
        "description": "基于OpenCV VideoCapture的USB相机封装，支持普通USB摄像头"
    },
    "ILightController.h": {
        "summary": "光源控制器接口基类定义",
        "description": "光源控制器抽象接口，定义开关、亮度调节、频闪控制等方法"
    },
    "SerialLightController.h": {
        "summary": "串口光源控制器接口定义",
        "description": "通过RS232/RS485串口控制的光源，支持自定义协议"
    },
    "ModbusLightController.h": {
        "summary": "Modbus光源控制器接口定义",
        "description": "通过Modbus协议控制的光源，支持TCP和RTU模式"
    },
    "IIOController.h": {
        "summary": "IO控制器接口基类定义",
        "description": "数字IO控制器抽象接口，定义输入读取、输出控制方法"
    },
    "GPIOController.h": {
        "summary": "GPIO控制器接口定义",
        "description": "通用GPIO控制器实现，支持输入输出方向设置、电平读写"
    },
    "SerialIO.h": {
        "summary": "串口IO控制器接口定义",
        "description": "通过串口扩展的IO控制器，如Arduino等开发板"
    },
    "IPLCClient.h": {
        "summary": "PLC客户端接口基类定义",
        "description": "PLC通信抽象接口，定义连接、读写寄存器、位操作等方法"
    },
    "ModbusTCPClient.h": {
        "summary": "Modbus TCP客户端接口定义",
        "description": "Modbus TCP协议实现，支持线圈、保持寄存器读写"
    },
    "ModbusRTUClient.h": {
        "summary": "Modbus RTU客户端接口定义",
        "description": "Modbus RTU协议实现，通过串口与PLC通信"
    },
    "SiemensS7Client.h": {
        "summary": "西门子S7 PLC客户端接口定义",
        "description": "西门子S7协议实现，支持S7-200/300/400/1200/1500系列PLC"
    },
    "MitsubishiMCClient.h": {
        "summary": "三菱MC协议客户端接口定义",
        "description": "三菱MC协议实现，支持Q/L/FX系列PLC的数据读写"
    },
    
    # ==================== algorithm模块 ====================
    "algorithm_global.h": {
        "summary": "算法模块全局宏定义",
        "description": "定义ALGORITHM_LIBRARY导出宏，用于Windows DLL导出符号控制"
    },
    "IDefectDetector.h": {
        "summary": "缺陷检测器接口基类定义",
        "description": "检测器抽象接口，定义initialize/release/detect等核心方法，\n *       以及参数管理、结果过滤等通用功能"
    },
    "BaseDetector.h": {
        "summary": "检测器基类实现定义",
        "description": "检测器公共实现基类，提供参数管理、置信度过滤、结果构造等\n *       通用功能，具体检测器继承此类"
    },
    "DetectorFactory.h": {
        "summary": "检测器工厂类定义",
        "description": "检测器对象工厂，根据类型字符串创建对应检测器实例，\n *       支持注册自定义检测器类型"
    },
    "DetectorManager.h": {
        "summary": "检测器管理模块接口定义",
        "description": "管理多个检测器的生命周期和执行，支持串行/并行检测、\n *       结果合并、配置同步等功能"
    },
    "ScratchDetector.h": {
        "summary": "划痕检测器接口定义",
        "description": "基于边缘检测和Hough变换的划痕检测算法，支持多尺度检测、\n *       参数可配置（灵敏度、最小长度、最大宽度等）"
    },
    "CrackDetector.h": {
        "summary": "裂纹检测器接口定义",
        "description": "基于形态学和轮廓分析的裂纹检测算法，支持分支裂纹检测、\n *       裂纹长度和宽度测量"
    },
    "ForeignDetector.h": {
        "summary": "异物检测器接口定义",
        "description": "基于颜色和纹理分析的异物检测算法，支持背景建模、\n *       多尺度斑点检测"
    },
    "DimensionDetector.h": {
        "summary": "尺寸检测器接口定义",
        "description": "尺寸测量检测器，基于边缘检测和亚像素拟合，\n *       支持长度、宽度、角度测量和公差判断"
    },
    "DnnDetector.h": {
        "summary": "深度学习检测器基类定义",
        "description": "DNN检测器基类，封装OpenCV DNN模块，提供模型加载、\n *       推理、后处理等通用功能"
    },
    "YoloDetector.h": {
        "summary": "YOLO检测器接口定义",
        "description": "基于YOLO的深度学习检测器，支持YOLOv5/v8 ONNX模型，\n *       提供letterbox预处理、NMS后处理、CUDA加速"
    },
    "ModelManager.h": {
        "summary": "模型管理模块接口定义",
        "description": "DNN模型管理器，负责模型文件的加载、缓存、版本管理、\n *       热更新等功能"
    },
    "ModelValidator.h": {
        "summary": "模型验证模块接口定义",
        "description": "模型质量验证工具，检查模型格式、输入输出维度、\n *       推理速度等指标"
    },
    "ModelValidationReport.h": {
        "summary": "模型验证报告类定义",
        "description": "存储模型验证结果的数据结构，包含验证状态、错误信息、\n *       性能指标等"
    },
    "ImagePreprocessor.h": {
        "summary": "图像预处理模块接口定义",
        "description": "图像预处理器，提供去噪、对比度增强、亮度调整、\n *       Gamma校正、ROI裁剪等功能"
    },
    "PreprocessCache.h": {
        "summary": "预处理缓存模块接口定义",
        "description": "预处理结果缓存，避免重复计算，支持LRU淘汰策略、\n *       命中率统计"
    },
    "Calibration.h": {
        "summary": "标定模块接口定义",
        "description": "相机标定和畸变校正，支持棋盘格标定、透视变换、\n *       像素-毫米转换"
    },
    "ROIManager.h": {
        "summary": "ROI管理模块接口定义",
        "description": "感兴趣区域管理器，支持多ROI定义、ROI模板保存加载、\n *       ROI内外检测区分"
    },
    "NMSFilter.h": {
        "summary": "非极大值抑制过滤器接口定义",
        "description": "NMS后处理过滤器，去除重叠检测框，支持按类别NMS、\n *       IoU阈值和置信度阈值配置"
    },
    "DefectMerger.h": {
        "summary": "缺陷合并模块接口定义",
        "description": "合并来自多个检测器的缺陷结果，处理重叠区域、\n *       统一缺陷ID分配"
    },
    "DefectScorer.h": {
        "summary": "缺陷评分模块接口定义",
        "description": "缺陷严重度评分器，根据缺陷类型、大小、位置等因素\n *       计算综合评分，确定产品等级(OK/Minor/Major/Critical)"
    },
    "SeverityConfig.h": {
        "summary": "严重度配置类定义",
        "description": "严重度评分配置数据结构，定义各缺陷类型的权重、\n *       阈值、评分规则等"
    },
    "IDetectorPlugin.h": {
        "summary": "检测器插件接口定义",
        "description": "Qt插件接口，定义插件元数据、检测器创建方法，\n *       支持动态加载第三方检测器"
    },
    "PluginManager.h": {
        "summary": "插件管理模块接口定义",
        "description": "插件管理器，负责扫描插件目录、加载/卸载插件、\n *       查询可用检测器类型"
    },
    "ExamplePlugin.h": {
        "summary": "示例插件实现",
        "description": "检测器插件示例代码，演示如何实现IDetectorPlugin接口，\n *       可作为开发新插件的模板"
    },
    
    # ==================== network模块 ====================
    "network_global.h": {
        "summary": "网络模块全局宏定义",
        "description": "定义NETWORK_LIBRARY导出宏，用于Windows DLL导出符号控制"
    },
    "MESClient.h": {
        "summary": "MES客户端接口定义",
        "description": "制造执行系统客户端，负责与MES服务器通信，\n *       上报检测结果、获取工单信息等"
    },
    "MESProtocol.h": {
        "summary": "MES通信协议定义",
        "description": "MES通信协议数据结构定义，包含消息类型、数据格式、\n *       校验规则等"
    },
    "WSServer.h": {
        "summary": "WebSocket服务器接口定义",
        "description": "WebSocket服务器，用于实时推送检测结果到Web客户端，\n *       支持多客户端连接、消息广播"
    },
    "HttpServer.h": {
        "summary": "HTTP服务器接口定义",
        "description": "RESTful HTTP服务器，提供API接口供外部系统调用，\n *       查询检测记录、获取统计数据等"
    },
    "ApiRoutes.h": {
        "summary": "API路由定义",
        "description": "HTTP API路由配置，定义URL路径与处理函数的映射关系"
    },
    
    # ==================== ui模块 ====================
    "ui_global.h": {
        "summary": "UI模块全局宏定义",
        "description": "定义UI_LIBRARY导出宏，用于Windows DLL导出符号控制"
    },
    "mainwindow.h": {
        "summary": "主窗口接口定义",
        "description": "应用程序主窗口，包含菜单栏、工具栏、状态栏，\n *       管理各功能视图和对话框"
    },
    "DetectPipeline.h": {
        "summary": "检测流水线模块接口定义",
        "description": "检测流程控制器，协调相机采集、图像预处理、缺陷检测、\n *       结果评分等步骤，支持连续检测和单次检测模式"
    },
    "ImageView.h": {
        "summary": "图像视图控件接口定义",
        "description": "基于QGraphicsView的图像显示控件，支持缩放、平移、\n *       缺陷标注显示、ROI编辑、手动标注绘制等功能"
    },
    "ImageViewControls.h": {
        "summary": "图像视图控制控件接口定义",
        "description": "图像视图的控制面板，包含缩放按钮、适应窗口、1:1显示等快捷操作"
    },
    "AnnotationPanel.h": {
        "summary": "标注面板控件接口定义",
        "description": "缺陷标注工具面板，提供标注形状选择、缺陷类型选择、\n *       严重等级选择、标注列表管理等功能"
    },
    "ParamPanel.h": {
        "summary": "参数面板控件接口定义",
        "description": "检测参数调节面板，显示和修改各检测器的参数，\n *       支持实时预览参数效果"
    },
    "ResultCard.h": {
        "summary": "结果卡片控件接口定义",
        "description": "检测结果显示卡片，展示OK/NG状态、缺陷类型、\n *       严重等级、置信度等信息"
    },
    "SeverityBar.h": {
        "summary": "严重度条控件接口定义",
        "description": "严重度可视化条形控件，用颜色和长度直观展示缺陷严重程度"
    },
    "ROIEditor.h": {
        "summary": "ROI编辑器控件接口定义",
        "description": "感兴趣区域编辑器，支持矩形/多边形ROI绘制、调整、保存"
    },
    "DetectView.h": {
        "summary": "检测视图接口定义",
        "description": "检测主界面视图，整合图像显示、结果卡片、参数面板等组件"
    },
    "HistoryView.h": {
        "summary": "历史视图接口定义",
        "description": "历史记录查看界面，显示检测历史列表、支持筛选、排序、导出"
    },
    "StatisticsView.h": {
        "summary": "统计视图接口定义",
        "description": "统计分析界面，显示良率趋势图、缺陷类型分布图、\n *       时间段统计等图表"
    },
    "SPCView.h": {
        "summary": "SPC统计过程控制视图接口定义",
        "description": "SPC控制图界面，显示X-bar图、R图、Cpk计算等质量控制工具"
    },
    "DefectTableModel.h": {
        "summary": "缺陷表格模型定义",
        "description": "Qt Model/View架构的缺陷数据模型，供QTableView显示缺陷列表"
    },
    "HistoryTableModel.h": {
        "summary": "历史表格模型定义",
        "description": "Qt Model/View架构的历史记录数据模型，供QTableView显示检测历史"
    },
    "UserManager.h": {
        "summary": "用户管理服务接口定义",
        "description": "用户会话管理服务，处理登录验证、权限检查、会话超时等"
    },
    "AboutDialog.h": {
        "summary": "关于对话框接口定义",
        "description": "显示软件版本、版权信息、开源许可等内容的对话框"
    },
    "AlarmDialog.h": {
        "summary": "报警对话框接口定义",
        "description": "报警信息显示和处理对话框，展示报警详情、确认操作等"
    },
    "CalibrationDialog.h": {
        "summary": "标定对话框接口定义",
        "description": "相机标定向导对话框，引导用户完成棋盘格拍摄、标定计算等步骤"
    },
    "HistoryDialog.h": {
        "summary": "历史对话框接口定义",
        "description": "历史记录详情对话框，显示单次检测的完整信息和图像"
    },
    "ImagePreviewDialog.h": {
        "summary": "图片预览对话框接口定义",
        "description": "增强版图片预览对话框，集成ImageView和AnnotationPanel，\n *       支持缺陷标注显示、ROI编辑、手动标注添加/编辑/删除等功能"
    },
    "LoginDialog.h": {
        "summary": "登录对话框接口定义",
        "description": "用户登录对话框，输入用户名密码进行身份验证"
    },
    "SettingsDialog.h": {
        "summary": "设置对话框接口定义",
        "description": "系统设置对话框，包含多个设置页面（相机、光源、检测、存储等）"
    },
    "StatisticsDialog.h": {
        "summary": "统计对话框接口定义",
        "description": "统计数据查看对话框，显示指定时间范围的统计信息"
    },
    "UserManagementDialog.h": {
        "summary": "用户管理对话框接口定义",
        "description": "用户账号管理对话框，支持用户增删改、权限设置、密码重置"
    },
    "CameraSettingsPage.h": {
        "summary": "相机设置页面接口定义",
        "description": "相机参数设置页面，配置相机类型、曝光、增益、触发模式等"
    },
    "LightSettingsPage.h": {
        "summary": "光源设置页面接口定义",
        "description": "光源参数设置页面，配置光源类型、亮度、频闪时间等"
    },
    "PLCSettingsPage.h": {
        "summary": "PLC设置页面接口定义",
        "description": "PLC通信设置页面，配置PLC类型、IP地址、端口、寄存器地址等"
    },
    "StorageSettingsPage.h": {
        "summary": "存储设置页面接口定义",
        "description": "数据存储设置页面，配置图像保存路径、保留天数、备份策略等"
    },
    "DetectionSettingsPage.h": {
        "summary": "检测设置页面接口定义",
        "description": "检测参数设置页面，配置各检测器的启用状态和参数"
    },
    "UserSettingsPage.h": {
        "summary": "用户设置页面接口定义",
        "description": "用户偏好设置页面，配置界面语言、主题、快捷键等"
    },
    "SettingsPageUtils.h": {
        "summary": "设置页面工具类定义",
        "description": "设置页面公共工具函数和样式定义，提供统一的UI组件创建方法"
    },
    
    # ==================== app模块 ====================
    "FlowController.h": {
        "summary": "流程控制器接口定义",
        "description": "检测流程总控制器，管理自动检测循环、与PLC交互、\n *       异常处理和恢复等"
    },
    "ConfigValidator.h": {
        "summary": "配置验证器接口定义",
        "description": "配置文件验证工具，检查配置完整性、参数合法性、\n *       硬件连接可用性等"
    },
    "SystemWatchdog.h": {
        "summary": "系统看门狗接口定义",
        "description": "系统健康监控，检测主线程阻塞、内存泄漏、磁盘空间不足等异常"
    },
    "ResultAggregator.h": {
        "summary": "结果聚合器接口定义",
        "description": "多工位/多相机检测结果聚合器，合并各路检测结果，\n *       生成综合判定"
    },
}

def get_file_info(filename):
    """获取文件信息"""
    if filename in FILE_INFO:
        return FILE_INFO[filename]
    # 默认描述
    name = filename.replace('.h', '').replace('_', ' ')
    return {
        "summary": f"{name}模块接口定义",
        "description": "（待补充详细描述）"
    }

def has_copyright(content):
    """检查是否已有版权声明"""
    return "Copyright" in content[:500] or "copyright" in content[:500]

def remove_old_copyright(content):
    """移除旧的版权声明"""
    # 匹配开头的 /* ... */ 块
    pattern = r'^/\*[\s\S]*?\*/\s*'
    return re.sub(pattern, '', content)

def process_file(filepath, force_update=True):
    """处理单个文件"""
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
    except UnicodeDecodeError:
        try:
            with open(filepath, 'r', encoding='gbk') as f:
                content = f.read()
        except:
            print(f"  跳过(编码错误): {filepath}")
            return False
    
    # 跳过moc生成的文件
    filename = os.path.basename(filepath)
    if filename == "moc_predefs.h":
        print(f"  跳过(moc文件): {filepath}")
        return False
    
    # 移除旧的版权声明
    if has_copyright(content):
        if force_update:
            content = remove_old_copyright(content)
        else:
            print(f"  跳过(已有版权): {filepath}")
            return False
    
    # 获取文件信息
    info = get_file_info(filename)
    
    # 生成版权声明
    copyright_header = COPYRIGHT_TEMPLATE.format(
        filename=filename, 
        summary=info["summary"],
        description=info["description"]
    )
    
    # 写入文件
    new_content = copyright_header + content.lstrip()
    with open(filepath, 'w', encoding='utf-8') as f:
        f.write(new_content)
    
    print(f"  已更新: {filepath}")
    return True

def main():
    src_dir = Path(__file__).parent.parent / "src"
    
    print(f"扫描目录: {src_dir}")
    print("-" * 60)
    
    count = 0
    total = 0
    
    for header_file in src_dir.rglob("*.h"):
        total += 1
        if process_file(str(header_file), force_update=True):
            count += 1
    
    print("-" * 60)
    print(f"完成! 更新 {count}/{total} 个文件")

if __name__ == "__main__":
    main()
