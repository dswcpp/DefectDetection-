# DefectDetection 项目架构文档

## 1. 项目概述

DefectDetection 是一个基于 Qt6 和 OpenCV 的工业缺陷检测系统，用于自动化产品质量检测。系统支持多种缺陷类型检测，包括划痕、裂纹、异物和尺寸偏差。

## 2. 技术选型

| 技术栈 | 版本 | 用途 |
|--------|------|------|
| Qt | 6.8.1 | GUI框架、信号槽机制、多线程 |
| OpenCV | 4.6.0 | 图像处理、计算机视觉算法 |
| C++ | C++17 | 核心开发语言 |
| MinGW | 13.2.0 | 编译工具链 |
| spdlog | - | 高性能日志库 |
| nlohmann/json | - | JSON 配置解析 |
| SQLite | - | 本地数据存储 |

## 3. 系统架构

### 3.1 分层架构

```
┌─────────────────────────────────────────────────────────┐
│                    Application Layer                     │
│                      (src/app)                          │
│         main.cpp, FlowController, SystemWatchdog        │
├─────────────────────────────────────────────────────────┤
│                      UI Layer                            │
│                      (src/ui)                           │
│    MainWindow, Dialogs, Views, Widgets, DetectPipeline  │
├─────────────────────────────────────────────────────────┤
│                   Algorithm Layer                        │
│                   (src/algorithm)                        │
│  DetectorManager, Detectors, Preprocess, Postprocess    │
├─────────────────────────────────────────────────────────┤
│                     HAL Layer                            │
│                     (src/hal)                           │
│         Camera, GPIO, PLC Communication                  │
├─────────────────────────────────────────────────────────┤
│                    Data Layer                            │
│                    (src/data)                           │
│      DatabaseManager, Repositories, Storage              │
├─────────────────────────────────────────────────────────┤
│                   Common Layer                           │
│                   (src/common)                          │
│        ConfigManager, Logger, Utils, Timer               │
├─────────────────────────────────────────────────────────┤
│                   Network Layer                          │
│                   (src/network)                         │
│          HttpServer, WSServer, MESClient                 │
└─────────────────────────────────────────────────────────┘
```

### 3.2 模块依赖关系

```
app
 ├── ui
 │    ├── algorithm
 │    │    ├── common
 │    │    ├── data
 │    │    └── hal
 │    ├── data
 │    │    └── common
 │    └── hal
 │         └── common
 ├── network
 │    └── common
 └── common
```

## 4. 核心模块说明

### 4.1 common - 公共基础层

**职责**: 提供全局配置、日志、工具类等基础服务

| 组件 | 说明 |
|------|------|
| ConfigManager | 单例模式配置管理，支持热重载 |
| Logger | 基于 spdlog 的日志系统 |
| Timer | 高精度计时器 |
| ThreadPool | 线程池实现 |

**关键类**:
```cpp
// 配置管理器 (Meyers' Singleton)
class ConfigManager : public QObject {
    static ConfigManager& instance();
    bool load(const QString& path = "");
    bool save();
    AppConfig& config();
};

// 日志宏
LOG_INFO("message: {}", value);
LOG_WARN("warning");
LOG_ERROR("error: {}", err);
```

### 4.2 algorithm - 算法引擎层

**职责**: 图像预处理、缺陷检测、评分评级

**核心架构**:
```
algorithm/
├── IDefectDetector.h      # 检测器接口
├── BaseDetector.h         # 检测器基类
├── DetectorFactory        # 检测器工厂
├── DetectorManager        # 检测器管理器（支持并行检测）
├── detectors/             # 具体检测器实现
│   ├── ScratchDetector    # 划痕检测（多尺度+Hough）
│   ├── CrackDetector      # 裂纹检测（CLAHE增强）
│   ├── ForeignDetector    # 异物检测（颜色+灰度）
│   └── DimensionDetector  # 尺寸检测
├── dnn/                   # 深度学习推理
│   └── YoloDetector       # YOLO 目标检测
├── preprocess/            # 预处理
│   ├── ImagePreprocessor  # 图像预处理
│   └── PreprocessCache    # 预处理缓存
├── postprocess/           # 后处理
│   └── NMSFilter          # 非极大值抑制
└── scoring/               # 评分
    └── DefectScorer       # 严重度评分
```

**检测器接口**:
```cpp
class IDefectDetector {
    virtual QString name() const = 0;
    virtual QString type() const = 0;
    virtual bool initialize() = 0;
    virtual void release() = 0;
    virtual DetectionResult detect(const cv::Mat& image) = 0;
    virtual void setParameters(const QVariantMap& params) = 0;
};
```

**并行检测**:
```cpp
// DetectorManager 支持并行执行所有检测器
CombinedResult detectAllParallel(const cv::Mat& image);
```

### 4.3 ui - 用户界面层

**职责**: Qt6 GUI、用户交互、检测流水线控制

**核心组件**:
| 组件 | 说明 |
|------|------|
| MainWindow | 主窗口，整合所有视图 |
| DetectPipeline | 检测流水线（异步执行） |
| DetectView | 实时检测视图 |
| HistoryView | 历史记录视图 |
| StatisticsView | 统计分析视图 |
| SettingsDialog | 设置对话框 |

**DetectPipeline 工作流程**:
```
Timer触发 → 后台线程抓取图像 → 缩放 → 并行检测 → NMS去重 → 评分 → 发射结果信号
```

### 4.4 hal - 硬件抽象层

**职责**: 相机、GPIO、PLC 通信抽象

**相机支持**:
- FileCamera: 文件夹图像源（开发测试）
- USBCamera: USB 摄像头
- GigECamera: GigE 工业相机
- HikCamera: 海康威视相机
- DahengCamera: 大恒相机

**PLC 通信**:
- ModbusTCP/RTU
- Siemens S7
- Mitsubishi MC

### 4.5 data - 数据层

**职责**: 数据持久化、数据库操作

**Repository 模式**:
```cpp
class DefectRepository {
    bool insert(const DefectRecord& record);
    DefectRecord findById(qint64 id);
    QList<DefectRecord> findByDateRange(QDate from, QDate to);
};
```

### 4.6 network - 网络层

**职责**: HTTP API、WebSocket、MES 集成

## 5. 数据流

### 5.1 检测流程

```
┌──────────┐    ┌──────────┐    ┌─────────────┐    ┌──────────┐
│  Camera  │───▶│ Pipeline │───▶│ Detectors   │───▶│  Result  │
│  (HAL)   │    │  (UI)    │    │ (Algorithm) │    │  (Data)  │
└──────────┘    └──────────┘    └─────────────┘    └──────────┘
     │               │                 │                 │
     │               ▼                 ▼                 ▼
     │         ┌──────────┐    ┌─────────────┐    ┌──────────┐
     └────────▶│ frameReady│───▶│ Preprocess  │───▶│ Database │
               │  signal   │    │ NMS, Score  │    │  Storage │
               └──────────┘    └─────────────┘    └──────────┘
```

### 5.2 配置流程

```
app.json (文件) 
    ↓
ConfigManager.load()
    ↓
AppConfig (内存)
    ↓
各模块读取配置
    ↓
ConfigManager.save()
    ↓
app.json (持久化)
```

## 6. 线程模型

| 线程 | 职责 |
|------|------|
| 主线程 | Qt 事件循环、UI 更新 |
| 检测线程池 | QtConcurrent 管理，并行执行检测器 |
| 相机线程 | 图像采集（部分相机 SDK 要求） |

**异步检测实现**:
```cpp
// DetectPipeline::onCaptureTimeout()
QFuture<DetectResult> future = QtConcurrent::run([this]() {
    // 后台线程：抓取 + 检测
    cv::Mat frame = m_camera->grab();
    return runDetection(frame);
});
m_detectWatcher.setFuture(future);

// 检测完成后在主线程发射信号
void DetectPipeline::onDetectionFinished() {
    emit frameReady(m_pendingFrame);
    emit resultReady(m_detectWatcher.result());
}
```

## 7. 构建系统

### 7.1 项目结构

```
DefectDetection/
├── DefectDetection.pro    # 主项目文件 (subdirs)
├── config/
│   ├── config.pri         # 全局配置
│   └── app.json           # 应用配置
├── src/
│   ├── common/common.pro  # 公共库
│   ├── data/data.pro      # 数据层
│   ├── hal/hal.pro        # 硬件抽象
│   ├── algorithm/algorithm.pro  # 算法
│   ├── ui/ui.pro          # 界面
│   ├── network/network.pro # 网络
│   └── app/app.pro        # 主程序
├── third_party/           # 第三方库
│   ├── opencv/
│   ├── spdlog/
│   └── json/
└── docs/                  # 文档
```

### 7.2 编译命令

```bash
# Debug 构建
cd build/Desktop_Qt_6_8_1_MinGW_64_bit-Debug
mingw32-make -j4

# Release 构建
cd build/Desktop_Qt_6_8_1_MinGW_64_bit-Release
mingw32-make -j4
```

## 8. 扩展性设计

### 8.1 检测器扩展

实现 `IDefectDetector` 接口即可添加新检测器：

```cpp
class MyDetector : public BaseDetector {
    QString name() const override { return "My Detector"; }
    QString type() const override { return "my_type"; }
    DetectionResult detect(const cv::Mat& image) override {
        // 实现检测逻辑
    }
};

// 注册到 DetectorManager
manager->addDetector("my_detector", std::make_shared<MyDetector>());
```

### 8.2 相机扩展

实现 `ICamera` 接口：

```cpp
class MyCamera : public ICamera {
    bool open(const CameraConfig& config) override;
    void close() override;
    bool grab(cv::Mat& frame) override;
};
```

## 9. 性能优化

| 优化项 | 实现方式 |
|--------|----------|
| 并行检测 | QtConcurrent 多线程执行检测器 |
| 图像缩放 | 大图自动缩小到 1920px 处理 |
| 预处理缓存 | PreprocessCache 避免重复计算 |
| 缺陷数量限制 | 每检测器最多100个，最终最多50个 |
| 异步处理 | 检测在后台线程，不阻塞 UI |

## 10. 版本信息

- 当前版本: 1.0.0
- Qt 版本: 6.8.1
- OpenCV 版本: 4.6.0
- 编译器: MinGW 13.2.0 (64-bit)
