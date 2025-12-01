# DefectDetection 开发任务清单

> 版本: 1.0  
> 更新日期: 2024-12  
> 基于项目设计文档和现有源码分析

---

## 目录

1. [项目现状分析](#项目现状分析)
2. [里程碑规划](#里程碑规划)
3. [详细任务清单](#详细任务清单)
4. [技术债务](#技术债务)
5. [风险与注意事项](#风险与注意事项)
6. [验收标准](#验收标准)

---

## 项目现状分析

### 已完成模块

| 模块 | 完成度 | 说明 |
|------|--------|------|
| 项目框架 | 100% | qmake 多模块构建，依赖关系正确 |
| 配置系统 | 90% | ConfigManager + JSON 序列化，缺少热加载 |
| 日志系统 | 95% | spdlog 封装，支持 Qt 类型格式化 |
| 主窗口 UI | 85% | 布局完成，信号槽连接，缺少部分功能 |
| ImageView | 90% | 缩放/标注/ROI编辑/导出，功能较完整 |
| SettingsDialog | 80% | 6页设置界面，需接入真实硬件 |
| DetectPipeline | 60% | 框架完成，检测算法为模拟数据 |
| FileCamera | 100% | 从目录读取图片，离线调试可用 |
| CameraFactory | 80% | 工厂模式，各相机类为空壳 |
| DatabaseManager | 70% | 基础 CRUD，缺少初始化和 Repository |
| Types/Constants | 90% | 数据类型定义基本完整 |

### 待实现模块

| 模块 | 完成度 | 阻塞项 |
|------|--------|--------|
| 检测算法 | 5% | IDefectDetector 接口为空 |
| DNN 推理 | 10% | YoloDetector 框架存在，无实现 |
| 真实相机驱动 | 0% | 需 SDK 集成 |
| PLC 通信 | 5% | 接口定义存在，无实现 |
| 光源/IO 控制 | 0% | 接口定义存在 |
| REST API | 0% | HttpServer 为空壳 |
| WebSocket | 10% | WSServer 框架存在 |
| 统计/历史视图 | 20% | 界面框架存在，无数据 |
| SPC 视图 | 10% | 基础框架 |
| 单元测试 | 0% | tests 目录为空 |

---

## 里程碑规划

### M1: 核心检测功能 (第1-2周)

**目标**: 实现完整的单类型缺陷检测流程

```
Week 1:
├── Day 1-2: IDefectDetector 接口 + ScratchDetector 实现
├── Day 3-4: ImagePreprocessor + DefectScorer
└── Day 5: DetectPipeline 集成测试

Week 2:
├── Day 1-2: CrackDetector + ForeignDetector
├── Day 3-4: DimensionDetector + 参数配置
└── Day 5: 算法参数调优 + 性能测试
```

**交付物**:
- 4个检测器可独立运行
- 检测节拍 < 100ms
- 参数可通过 SettingsDialog 配置

---

### M2: 硬件集成 (第3-4周)

**目标**: 接入真实工业相机和 PLC

```
Week 3:
├── Day 1-2: HikCamera SDK 集成
├── Day 3: DahengCamera SDK 集成
├── Day 4: 相机参数设置 (曝光/增益/触发)
└── Day 5: 相机稳定性测试

Week 4:
├── Day 1-2: ModbusTCPClient 实现
├── Day 3: 触发信号 + OK/NG 输出
├── Day 4: 光源控制器实现
└── Day 5: 硬件联调测试
```

**交付物**:
- 支持海康/大恒相机
- Modbus TCP 通信正常
- 触发-采集-检测-输出闭环

---

### M3: 数据持久化 (第5周)

**目标**: 完整的数据存储和查询

```
Week 5:
├── Day 1: 数据库自动初始化 + schema 执行
├── Day 2: InspectionRepository + DefectRepository
├── Day 3: ImageStorage 图像存储
├── Day 4: HistoryView 历史查询界面
└── Day 5: 数据导出 (CSV/Excel)
```

**交付物**:
- 检测结果自动入库
- 历史记录查询和导出
- 图像按日期目录存储

---

### M4: 功能完善 (第6-7周)

**目标**: 完整的产品功能

```
Week 6:
├── Day 1-2: StatisticsView 统计图表
├── Day 3-4: SPCView 控制图
└── Day 5: AlarmDialog 报警管理

Week 7:
├── Day 1-2: REST API + WebSocket
├── Day 3: 用户认证 + 权限控制
├── Day 4: SystemWatchdog 看门狗
└── Day 5: 配置热加载
```

**交付物**:
- 良率统计和趋势图
- SPC 控制图 (X-bar R)
- 远程监控 API

---

### M5: 测试与部署 (第8周)

**目标**: 产线级稳定性

```
Week 8:
├── Day 1-2: 单元测试编写
├── Day 3: 集成测试
├── Day 4: 72小时稳定性测试
└── Day 5: 打包部署脚本
```

**交付物**:
- 测试覆盖率 > 70%
- 72小时无崩溃
- Windows 安装包

---

## 详细任务清单

### 1. 算法层 (algorithm/)

#### 1.1 检测器接口 [P0] [2h]

```cpp
// src/algorithm/IDefectDetector.h
class IDefectDetector {
public:
    virtual ~IDefectDetector() = default;
    
    // 核心接口
    virtual DetectResult detect(const cv::Mat& image, const cv::Rect& roi) = 0;
    virtual void setParams(const QVariantMap& params) = 0;
    virtual QVariantMap getParams() const = 0;
    
    // 元信息
    virtual QString name() const = 0;
    virtual QString version() const = 0;
    virtual QStringList supportedDefectTypes() const = 0;
};
```

**验收标准**:
- [ ] 接口定义完整
- [ ] 虚析构函数
- [ ] 所有检测器继承此接口

---

#### 1.2 ScratchDetector 划痕检测 [P0] [8h]

**算法流程**:
```
输入图像 → 灰度化 → 高斯滤波 → Canny边缘 → 形态学闭运算 
→ HoughLinesP → 线段过滤 → 评分 → 输出结果
```

**关键参数**:
| 参数 | 类型 | 默认值 | 范围 | 说明 |
|------|------|--------|------|------|
| canny_low | int | 50 | 10-100 | Canny 低阈值 |
| canny_high | int | 150 | 50-255 | Canny 高阈值 |
| min_length | int | 20 | 5-200 | 最小线段长度(像素) |
| max_gap | int | 10 | 1-50 | 线段最大间隙 |
| min_contrast | int | 30 | 10-100 | 最小对比度 |

**实现要点**:
- [ ] 输入校验 (空图像/ROI越界)
- [ ] ROI 坐标转换回原图
- [ ] 线段角度过滤 (避免误检边缘)
- [ ] 对比度计算 (线段两侧灰度差)
- [ ] 异常捕获 (cv::Exception)

**测试用例**:
- [ ] 正常划痕图像检出
- [ ] 无划痕图像不误检
- [ ] 边界条件 (ROI 在边缘)
- [ ] 参数极端值测试

---

#### 1.3 CrackDetector 裂纹检测 [P0] [8h]

**算法流程**:
```
输入图像 → 灰度化 → Top-hat增强 → 自适应阈值 → 细化(Zhang-Suen)
→ 连通域分析 → 骨架分支计数 → 评分 → 输出结果
```

**关键参数**:
| 参数 | 类型 | 默认值 | 范围 | 说明 |
|------|------|--------|------|------|
| tophat_kernel | int | 15 | 5-31 | Top-hat 核大小 |
| adaptive_block | int | 11 | 3-51 | 自适应阈值块大小 |
| adaptive_c | int | 2 | -10-10 | 自适应阈值常数 |
| min_area | int | 100 | 10-1000 | 最小面积(像素²) |
| min_branch | int | 2 | 1-10 | 最小分支数 |

**实现要点**:
- [ ] cv::ximgproc::thinning 细化
- [ ] 骨架分支点检测 (3x3邻域)
- [ ] 连通域长度计算
- [ ] 裂纹深度估算 (灰度梯度)

---

#### 1.4 ForeignDetector 异物检测 [P0] [6h]

**算法流程**:
```
输入图像 → 灰度化 → 背景建模 → 差分 → 阈值分割
→ 形态学开运算 → 连通域分析 → 形状过滤 → 输出结果
```

**关键参数**:
| 参数 | 类型 | 默认值 | 范围 | 说明 |
|------|------|--------|------|------|
| bg_subtract_method | string | "static" | static/mog2 | 背景建模方法 |
| threshold | int | 30 | 10-100 | 差分阈值 |
| min_area | int | 50 | 10-500 | 最小面积 |
| max_area | int | 5000 | 100-50000 | 最大面积 |
| min_circularity | double | 0.3 | 0.1-1.0 | 最小圆度 |

**实现要点**:
- [ ] 静态背景学习
- [ ] 光照变化补偿
- [ ] 圆度/偏心率计算
- [ ] 面积过滤

---

#### 1.5 DimensionDetector 尺寸测量 [P0] [8h]

**算法流程**:
```
输入图像 → 灰度化 → 边缘检测 → 轮廓提取 → 最小外接矩形
→ 像素-物理换算 → 公差判定 → 输出结果
```

**关键参数**:
| 参数 | 类型 | 默认值 | 范围 | 说明 |
|------|------|--------|------|------|
| pixel_per_mm | double | 10.0 | 1-100 | 像素/毫米 |
| edge_method | string | "canny" | canny/sobel | 边缘方法 |
| target_width | double | 50.0 | 1-1000 | 目标宽度(mm) |
| target_height | double | 30.0 | 1-1000 | 目标高度(mm) |
| tolerance | double | 0.5 | 0.01-10 | 公差(mm) |

**实现要点**:
- [ ] 亚像素边缘检测
- [ ] 角度测量
- [ ] 圆孔直径测量
- [ ] 标定系数管理

---

#### 1.6 DefectScorer 严重度评分 [P0] [4h]

**评分公式**:
```
severity_score = w1 * norm_length + w2 * norm_area + w3 * norm_contrast + w4 * norm_branch

其中:
- norm_xxx = (value - min) / (max - min)  归一化到 [0, 1]
- w1 + w2 + w3 + w4 = 1.0

分级:
- 0-20: OK (无缺陷或可忽略)
- 20-50: Minor (轻微)
- 50-80: Moderate (中等)  
- 80-100: Severe (严重)
```

**配置结构**:
```json
{
  "severity": {
    "scratch": {
      "weights": {"length": 0.4, "width": 0.3, "contrast": 0.3},
      "thresholds": {"minor": 20, "moderate": 50, "severe": 80}
    },
    "crack": {
      "weights": {"length": 0.3, "branch": 0.4, "depth": 0.3},
      "thresholds": {"minor": 15, "moderate": 40, "severe": 70}
    }
  }
}
```

---

#### 1.7 ImagePreprocessor 预处理 [P0] [4h]

**处理步骤**:
```cpp
class ImagePreprocessor {
public:
    struct Config {
        bool convertGray = true;
        int blurType = 0;        // 0:none, 1:gaussian, 2:bilateral
        int blurKernelSize = 5;
        bool equalize = false;   // CLAHE
        double claheClipLimit = 2.0;
        cv::Size claheGridSize = {8, 8};
    };
    
    cv::Mat process(const cv::Mat& input, const Config& cfg);
    cv::Mat cropROI(const cv::Mat& input, const cv::Rect& roi);
};
```

---

#### 1.8 DNN 推理模块 [P1] [12h]

**DnnDetector 基类**:
```cpp
class DnnDetector : public IDefectDetector {
protected:
    cv::dnn::Net m_net;
    cv::Size m_inputSize = {640, 640};
    float m_confThreshold = 0.5f;
    float m_nmsThreshold = 0.4f;
    
    virtual std::vector<DetectResult> postProcess(
        const std::vector<cv::Mat>& outputs,
        const cv::Size& imgSize) = 0;
};
```

**YoloDetector 实现**:
- [ ] ONNX 模型加载
- [ ] blobFromImage 预处理
- [ ] YOLO 输出解析 (cx, cy, w, h, conf, classes)
- [ ] NMS 后处理
- [ ] 坐标缩放到原图
- [ ] GPU 加速 (setPreferableBackend)

**ModelManager 模型管理**:
- [ ] 模型版本管理
- [ ] 模型热更新
- [ ] 模型验证 (ModelValidator)
- [ ] 推理时间统计

---

### 2. 硬件抽象层 (hal/)

#### 2.1 HikCamera 海康相机 [P0] [12h]

**SDK 集成**:
```cpp
// 需要链接: MvCameraControl.lib
#include "MvCameraControl.h"

class HikCamera : public ICamera {
private:
    void* m_handle = nullptr;
    MV_CC_DEVICE_INFO_LIST m_deviceList;
    
public:
    bool open(const CameraConfig& cfg) override;
    bool grab(cv::Mat& frame) override;
    void close() override;
    
    // 扩展接口
    bool setExposure(double us);
    bool setGain(double db);
    bool setTriggerMode(int mode);  // 0:off, 1:on
    bool softTrigger();
    std::vector<std::string> enumDevices();
};
```

**实现要点**:
- [ ] MV_CC_EnumDevices 设备枚举
- [ ] MV_CC_CreateHandle 创建句柄
- [ ] MV_CC_OpenDevice 打开设备
- [ ] MV_CC_SetFloatValue 设置曝光/增益
- [ ] MV_CC_StartGrabbing 开始采集
- [ ] MV_CC_GetOneFrameTimeout 获取图像
- [ ] Mono8/BayerRG8 → BGR 转换
- [ ] 错误码处理
- [ ] 断线重连机制

**测试用例**:
- [ ] 设备枚举
- [ ] 打开/关闭
- [ ] 连续采集 1000 帧
- [ ] 参数热修改
- [ ] 拔线重连

---

#### 2.2 DahengCamera 大恒相机 [P1] [8h]

**SDK 集成**:
```cpp
// 需要链接: GxIAPI.lib
#include "GxIAPI.h"

class DahengCamera : public ICamera {
private:
    GX_DEV_HANDLE m_handle = nullptr;
    // ...
};
```

---

#### 2.3 ModbusTCPClient [P0] [8h]

**实现**:
```cpp
class ModbusTCPClient : public QObject {
    Q_OBJECT
public:
    bool connect(const QString& ip, int port = 502);
    void disconnect();
    bool isConnected() const;
    
    // 读写接口
    bool readCoils(int addr, int count, QBitArray& result);
    bool readRegisters(int addr, int count, QVector<quint16>& result);
    bool writeCoil(int addr, bool value);
    bool writeRegister(int addr, quint16 value);
    bool writeRegisters(int addr, const QVector<quint16>& values);
    
signals:
    void connected();
    void disconnected();
    void error(const QString& msg);
    
private:
    modbus_t* m_ctx = nullptr;
    QTimer* m_heartbeatTimer;
    
    void startHeartbeat();
    void onHeartbeat();
};
```

**实现要点**:
- [ ] libmodbus 集成 (第三方库)
- [ ] 连接超时设置 (1秒)
- [ ] 响应超时设置 (500ms)
- [ ] 心跳保活 (1秒/次)
- [ ] 自动重连 (3次)
- [ ] 线程安全 (mutex)
- [ ] RAII 封装

**寄存器映射配置**:
```json
{
  "plc": {
    "trigger_in": 40001,
    "result_out": 40002,
    "defect_type": 40003,
    "severity": 40004,
    "heartbeat": 40005
  }
}
```

---

#### 2.4 SerialLightController [P1] [4h]

```cpp
class SerialLightController : public ILightController {
public:
    bool open(const QString& port, int baudrate = 9600);
    void close();
    
    bool setChannelBrightness(int channel, int brightness);  // 0-255
    bool setAllBrightness(int brightness);
    bool enableStrobe(int durationUs);
    bool disableStrobe();
    
private:
    QSerialPort* m_serial;
    
    bool sendCommand(const QByteArray& cmd);
    QByteArray buildSetBrightnessCmd(int ch, int val);
};
```

---

### 3. 数据层 (data/)

#### 3.1 数据库初始化 [P0] [2h]

```cpp
// DatabaseManager 扩展
bool DatabaseManager::initDatabase() {
    if (!open(m_configRepo->databasePath())) {
        return false;
    }
    
    // 检查是否需要初始化
    QSqlQuery query(m_db);
    query.exec("SELECT name FROM sqlite_master WHERE type='table' AND name='inspections'");
    if (!query.next()) {
        // 执行 schema.sql
        QString schemaPath = QCoreApplication::applicationDirPath() + "/config/schema.sql";
        return executeSchema(schemaPath);
    }
    return true;
}
```

---

#### 3.2 InspectionRepository [P0] [6h]

```cpp
class InspectionRepository : public QObject {
    Q_OBJECT
public:
    // CRUD
    qint64 insert(const InspectionRecord& record);
    bool update(const InspectionRecord& record);
    bool remove(qint64 id);
    std::optional<InspectionRecord> findById(qint64 id);
    
    // 查询
    QVector<InspectionRecord> findByTimeRange(
        const QDateTime& start, const QDateTime& end,
        const QString& result = QString(),
        int limit = 100, int offset = 0);
    
    int countByTimeRange(const QDateTime& start, const QDateTime& end,
                         const QString& result = QString());
    
    // 统计
    struct DailyStats {
        int total = 0;
        int okCount = 0;
        int ngCount = 0;
        double yieldRate = 0;
        double avgCycleTime = 0;
    };
    DailyStats getTodayStats();
    DailyStats getStats(const QDate& date);
    QVector<DailyStats> getStatsByRange(const QDate& start, const QDate& end);
    
    // 缺陷分布
    QMap<QString, int> getDefectDistribution(const QDate& date);
    
signals:
    void recordInserted(qint64 id);
    void recordUpdated(qint64 id);
};
```

---

#### 3.3 ImageStorage [P0] [4h]

```cpp
class ImageStorage {
public:
    struct Config {
        QString basePath = "./data/images";
        bool saveOriginal = true;
        bool saveAnnotated = true;
        bool saveThumbnail = true;
        int thumbnailSize = 200;
        int jpegQuality = 90;
    };
    
    // 保存图像，返回相对路径
    QString saveImage(const cv::Mat& image, 
                      const QString& prefix,
                      ImageType type = ImageType::Original);
    
    // 按日期组织: basePath/2024/01/15/prefix_001234.jpg
    QString generatePath(const QString& prefix, ImageType type);
    
    // 清理过期图像
    void cleanup(int retainDays = 30);
    
private:
    Config m_config;
    std::atomic<int> m_dailyCounter{0};
    QDate m_currentDate;
};
```

---

#### 3.4 数据导出 [P1] [4h]

```cpp
class CSVExporter {
public:
    bool exportInspections(const QVector<InspectionRecord>& records,
                           const QString& filePath);
    bool exportDefects(const QVector<DefectRecord>& records,
                       const QString& filePath);
};

class ExcelExporter {
public:
    // 使用 QXlsx 库
    bool exportReport(const QString& filePath,
                      const QDate& startDate,
                      const QDate& endDate);
};
```

---

### 4. 网络层 (network/)

#### 4.1 HttpServer REST API [P1] [8h]

```cpp
class HttpServer : public QObject {
    Q_OBJECT
public:
    bool start(int port = 8080);
    void stop();
    
private:
    QHttpServer* m_server;
    
    void setupRoutes();
    
    // API 处理函数
    QHttpServerResponse handleGetInspection(qint64 id);
    QHttpServerResponse handleListInspections(const QHttpServerRequest& req);
    QHttpServerResponse handleGetStatistics(const QHttpServerRequest& req);
    QHttpServerResponse handleSystemControl(const QJsonObject& body);
};
```

**API 端点**:
| 方法 | 路径 | 说明 |
|------|------|------|
| GET | /api/v1/inspection/{id} | 获取单条检测记录 |
| GET | /api/v1/inspections | 列表查询 (分页) |
| GET | /api/v1/statistics/overview | 今日统计 |
| GET | /api/v1/statistics/trend | 良率趋势 |
| POST | /api/v1/system/control | 系统控制 |
| GET | /healthz | 健康检查 |

---

#### 4.2 WebSocket 推送 [P1] [4h]

```cpp
class WSServer : public QObject {
    Q_OBJECT
public:
    bool start(int port = 8081);
    void stop();
    
    // 广播消息
    void broadcast(const QString& type, const QJsonObject& data);
    
public slots:
    void onInspectionResult(const DetectResult& result);
    void onAlarm(const AlarmInfo& alarm);
    
private:
    QWebSocketServer* m_server;
    QVector<QWebSocket*> m_clients;
};
```

**消息格式**:
```json
{
  "type": "inspection_result",
  "timestamp": 1705312335000,
  "data": {
    "id": "INS-20240115-001234",
    "result": "NG",
    "severity": 65.5,
    "defects": [...]
  }
}
```

---

### 5. 用户界面 (ui/)

#### 5.1 StatisticsView 统计视图 [P1] [8h]

**功能**:
- [ ] 今日统计卡片 (总数/OK/NG/良率)
- [ ] 良率趋势折线图 (7天/30天)
- [ ] 缺陷分布饼图
- [ ] 节拍分布直方图
- [ ] 时间范围选择器
- [ ] 数据刷新按钮
- [ ] 导出按钮

**使用 Qt Charts**:
```cpp
#include <QtCharts>

class StatisticsView : public QWidget {
    Q_OBJECT
public:
    explicit StatisticsView(QWidget* parent = nullptr);
    
public slots:
    void refresh();
    void setDateRange(const QDate& start, const QDate& end);
    
private:
    void setupUI();
    void createYieldChart();
    void createDefectPieChart();
    void createCycleTimeHistogram();
    void updateStats();
    
    QChartView* m_yieldChart;
    QChartView* m_defectChart;
    QChartView* m_cycleChart;
    
    // 统计卡片
    QLabel* m_totalLabel;
    QLabel* m_okLabel;
    QLabel* m_ngLabel;
    QLabel* m_yieldLabel;
};
```

---

#### 5.2 HistoryView 历史查询 [P1] [6h]

**功能**:
- [ ] 分页表格显示
- [ ] 时间范围筛选
- [ ] 结果筛选 (OK/NG/全部)
- [ ] 缺陷类型筛选
- [ ] 严重度筛选
- [ ] 关键词搜索
- [ ] 双击查看详情
- [ ] 批量导出
- [ ] 缩略图预览

```cpp
class HistoryView : public QWidget {
    Q_OBJECT
public:
    explicit HistoryView(QWidget* parent = nullptr);
    
public slots:
    void search();
    void exportSelected();
    void exportAll();
    
private:
    QTableView* m_tableView;
    HistoryTableModel* m_model;
    
    // 筛选条件
    QDateEdit* m_startDate;
    QDateEdit* m_endDate;
    QComboBox* m_resultFilter;
    QComboBox* m_defectTypeFilter;
    QLineEdit* m_searchEdit;
    
    // 分页
    int m_currentPage = 0;
    int m_pageSize = 50;
    int m_totalCount = 0;
};
```

---

#### 5.3 SPCView 控制图 [P2] [8h]

**功能**:
- [ ] X-bar R 控制图
- [ ] P 控制图 (不良率)
- [ ] 控制限自动计算
- [ ] 失控点标记
- [ ] 规则判定 (Western Electric Rules)
- [ ] CP/CPK 计算显示
- [ ] 控制限配置

```cpp
class SPCView : public QWidget {
    Q_OBJECT
public:
    struct ControlLimits {
        double ucl;  // 上控制限
        double cl;   // 中心线
        double lcl;  // 下控制限
        double usl;  // 上规格限
        double lsl;  // 下规格限
    };
    
    void setData(const QVector<double>& values);
    void setControlLimits(const ControlLimits& limits);
    
    double calculateCp() const;
    double calculateCpk() const;
    
private:
    QChartView* m_chartView;
    QLineSeries* m_dataSeries;
    QLineSeries* m_uclLine;
    QLineSeries* m_clLine;
    QLineSeries* m_lclLine;
    
    QVector<double> m_data;
    ControlLimits m_limits;
};
```

---

#### 5.4 AlarmDialog 报警管理 [P1] [4h]

**功能**:
- [ ] 当前报警列表
- [ ] 历史报警查询
- [ ] 报警确认/处理
- [ ] 报警统计
- [ ] 声音/弹窗提醒配置

```cpp
class AlarmDialog : public QDialog {
    Q_OBJECT
public:
    explicit AlarmDialog(QWidget* parent = nullptr);
    
    void addAlarm(const AlarmInfo& alarm);
    void acknowledgeAlarm(int id);
    void resolveAlarm(int id, const QString& resolution);
    
signals:
    void alarmAcknowledged(int id);
    void alarmResolved(int id);
    
private:
    QTableWidget* m_activeTable;
    QTableWidget* m_historyTable;
    QTabWidget* m_tabWidget;
};
```

---

### 6. 应用层 (app/)

#### 6.1 FlowController 状态机 [P0] [6h]

```cpp
class FlowController : public QObject {
    Q_OBJECT
public:
    enum class State {
        Idle,           // 空闲
        Initializing,   // 初始化中
        Ready,          // 就绪
        Running,        // 运行中
        Paused,         // 暂停
        Error,          // 错误
        Recovering      // 恢复中
    };
    Q_ENUM(State)
    
    State state() const;
    QString stateString() const;
    
public slots:
    void initialize();
    void start();
    void stop();
    void pause();
    void resume();
    void reset();
    
signals:
    void stateChanged(State newState, State oldState);
    void initialized();
    void started();
    void stopped();
    void error(const QString& module, const QString& message);
    
private:
    State m_state = State::Idle;
    
    bool canTransition(State from, State to) const;
    void setState(State state);
};
```

**状态转换规则**:
```
Idle → Initializing → Ready → Running
                   ↓         ↓      ↓
                 Error ← Paused ←──┘
                   ↓
              Recovering → Ready
```

---

#### 6.2 SystemWatchdog 看门狗 [P1] [4h]

```cpp
class SystemWatchdog : public QObject {
    Q_OBJECT
public:
    void registerModule(const QString& name, int timeoutMs = 5000);
    void unregisterModule(const QString& name);
    void feedDog(const QString& module);
    
    void start();
    void stop();
    
signals:
    void moduleTimeout(const QString& module, qint64 elapsedMs);
    void moduleRecovered(const QString& module);
    
private slots:
    void check();
    
private:
    struct ModuleInfo {
        qint64 lastHeartbeat;
        int timeoutMs;
        bool isTimeout = false;
    };
    
    QMap<QString, ModuleInfo> m_modules;
    QTimer* m_timer;
    mutable QMutex m_mutex;
};
```

---

#### 6.3 DetectPipeline 完善 [P0] [8h]

```cpp
class DetectPipeline : public QObject {
    Q_OBJECT
public:
    // 添加检测器
    void addDetector(std::shared_ptr<IDefectDetector> detector);
    void removeDetector(const QString& name);
    void clearDetectors();
    
    // 预处理配置
    void setPreprocessor(std::shared_ptr<ImagePreprocessor> preprocessor);
    
    // ROI 配置
    void setROI(const cv::Rect& roi);
    void setMultiROI(const QVector<cv::Rect>& rois);
    
    // 运行模式
    void setParallelMode(bool parallel);  // 检测器并行执行
    
private:
    DetectResult runDetection(const cv::Mat& frame) override;
    DetectResult runParallelDetection(const cv::Mat& frame);
    DetectResult runSequentialDetection(const cv::Mat& frame);
    DetectResult mergeResults(const QVector<DetectResult>& results);
    
    QVector<std::shared_ptr<IDefectDetector>> m_detectors;
    std::shared_ptr<ImagePreprocessor> m_preprocessor;
    std::shared_ptr<DefectScorer> m_scorer;
    
    cv::Rect m_roi;
    bool m_parallelMode = true;
    QThreadPool m_threadPool;
};
```

---

### 7. 测试 (tests/)

#### 7.1 单元测试框架 [P1] [4h]

```cpp
// tests/unit/test_scratch_detector.cpp
#include <gtest/gtest.h>
#include "algorithm/detectors/ScratchDetector.h"

class ScratchDetectorTest : public ::testing::Test {
protected:
    void SetUp() override {
        detector = std::make_unique<ScratchDetector>();
    }
    
    std::unique_ptr<ScratchDetector> detector;
};

TEST_F(ScratchDetectorTest, DetectsScratchInImage) {
    cv::Mat testImage = cv::imread("fixtures/scratch_01.png");
    ASSERT_FALSE(testImage.empty());
    
    auto result = detector->detect(testImage, cv::Rect(0, 0, testImage.cols, testImage.rows));
    
    EXPECT_FALSE(result.isOK);
    EXPECT_EQ(result.defectType, "Scratch");
    EXPECT_GT(result.defects.size(), 0);
}

TEST_F(ScratchDetectorTest, NoFalsePositiveOnCleanImage) {
    cv::Mat cleanImage = cv::imread("fixtures/clean_01.png");
    ASSERT_FALSE(cleanImage.empty());
    
    auto result = detector->detect(cleanImage, cv::Rect(0, 0, cleanImage.cols, cleanImage.rows));
    
    EXPECT_TRUE(result.isOK);
    EXPECT_EQ(result.defects.size(), 0);
}

TEST_F(ScratchDetectorTest, HandlesEmptyImage) {
    cv::Mat emptyImage;
    auto result = detector->detect(emptyImage, cv::Rect());
    
    EXPECT_TRUE(result.isOK);  // 空图像视为OK
    EXPECT_FALSE(result.errorMsg.isEmpty());
}
```

#### 7.2 测试用例清单

| 模块 | 测试类型 | 用例数 | 说明 |
|------|----------|--------|------|
| ScratchDetector | Unit | 10 | 正常/边界/异常 |
| CrackDetector | Unit | 10 | 正常/边界/异常 |
| ForeignDetector | Unit | 8 | 正常/边界/异常 |
| DimensionDetector | Unit | 8 | 正常/边界/异常 |
| DefectScorer | Unit | 6 | 评分/分级 |
| ConfigManager | Unit | 8 | 加载/保存/校验 |
| InspectionRepository | Integration | 10 | CRUD/查询/统计 |
| DetectPipeline | Integration | 5 | 端到端 |
| HikCamera | Integration | 5 | 采集/参数 |
| ModbusTCPClient | Integration | 5 | 读写/心跳 |

---

## 技术债务

### 高优先级
- [ ] IDefectDetector 接口为空，需要定义完整
- [ ] DetectPipeline.runDetection() 使用模拟数据
- [ ] 相机类 (HikCamera/DahengCamera/GigECamera/USBCamera) 为空壳
- [ ] PLC 客户端类为空壳
- [ ] 无单元测试

### 中优先级
- [ ] 配置热加载未实现
- [ ] 数据库无自动初始化
- [ ] 无数据迁移机制
- [ ] 日志未接入审计表
- [ ] 无用户认证

### 低优先级
- [ ] 无 CMake 构建支持
- [ ] 无 CI/CD
- [ ] 无安装包脚本
- [ ] 文档不完整
- [ ] 无多语言支持 (仅中文)

---

## 风险与注意事项

### 硬件相关
1. **相机 SDK 兼容性**: 海康/大恒 SDK 版本需与实际设备匹配
2. **相机驱动**: Windows 需安装 MVS/Galaxy 驱动
3. **网络配置**: GigE 相机需配置巨型帧和网卡中断
4. **PLC 通信**: 需确认寄存器地址映射正确

### 性能相关
1. **检测节拍**: 目标 < 100ms，需优化算法和使用多线程
2. **内存管理**: cv::Mat 深拷贝，避免跨线程共享
3. **队列容量**: 帧队列满时策略 (丢弃最旧)
4. **数据库**: 大量数据时需要索引优化

### 安全相关
1. **密码存储**: 使用 bcrypt 哈希
2. **API 认证**: JWT Token
3. **审计日志**: 关键操作记录
4. **配置备份**: 修改前自动备份

### 稳定性相关
1. **异常处理**: 所有 OpenCV 操作需 try-catch
2. **重连机制**: 相机/PLC 断线自动重连
3. **看门狗**: 模块心跳监控
4. **资源释放**: RAII 封装，析构函数清理

---

## 验收标准

### 功能验收
- [ ] 4类缺陷检测正常工作
- [ ] 检测结果自动入库
- [ ] 历史查询和导出
- [ ] 统计报表显示
- [ ] PLC 触发和输出

### 性能验收
| 指标 | 目标 | 测试方法 |
|------|------|----------|
| 检测节拍 | P95 < 100ms | 连续检测 1000 次 |
| 漏检率 | < 0.1% | Golden Sample 100片×10轮 |
| 误检率 | < 1% | OK样本 100片×10轮 |
| 内存增长 | < 10MB/h | 72小时运行 |
| 崩溃次数 | 0 | 72小时运行 |

### 兼容性验收
- [ ] Windows 10/11 正常运行
- [ ] 海康相机正常采集
- [ ] 大恒相机正常采集
- [ ] Modbus TCP 通信正常

---

## 附录

### A. 开发环境配置

```bash
# Windows 依赖
- Qt 6.5+ (MinGW 或 MSVC)
- OpenCV 4.6+
- 海康 MVS SDK
- 大恒 Galaxy SDK
- libmodbus (可选)

# 环境变量
OPENCV_DIR=C:\opencv\build
HIK_SDK_DIR=C:\Program Files\MVS
DAHENG_SDK_DIR=C:\Program Files\Daheng Imaging
```

### B. 测试图像准备

```
tests/fixtures/
├── scratch/
│   ├── scratch_01.png  # 单条划痕
│   ├── scratch_02.png  # 多条划痕
│   └── scratch_03.png  # 细微划痕
├── crack/
│   ├── crack_01.png    # 单裂纹
│   └── crack_02.png    # 分支裂纹
├── foreign/
│   ├── foreign_01.png  # 灰尘
│   └── foreign_02.png  # 油污
├── dimension/
│   └── dimension_01.png  # 标准尺寸
└── clean/
    ├── clean_01.png    # 无缺陷
    └── clean_02.png    # 无缺陷
```

### C. 参考资料

- [OpenCV 4.x 文档](https://docs.opencv.org/4.x/)
- [Qt 6 文档](https://doc.qt.io/qt-6/)
- [海康 MVS SDK 手册](https://www.hikrobotics.com/)
- [Modbus 协议规范](https://modbus.org/specs.php)

---

## 8. 产品化功能 (补充)

> 以下为产品交付必需但原文档未覆盖的功能

### 8.1 Golden Sample 验证系统 [P0] [6h]

**功能描述**: 使用标准样本验证系统检测准确性

```cpp
class GoldenSampleManager : public QObject {
    Q_OBJECT
public:
    struct GoldenSample {
        QString code;           // GS-001
        QString imagePath;      // 标准图像
        QString expectedResult; // OK/NG
        QString defectType;     // 期望缺陷类型
        QVector<cv::Rect> expectedRegions; // 期望检出区域
    };
    
    // 管理
    bool addSample(const GoldenSample& sample);
    bool removeSample(const QString& code);
    QVector<GoldenSample> getAllSamples();
    
    // 验证
    struct TestResult {
        QString sampleCode;
        bool passed;
        QString actualResult;
        double iou;  // 区域重叠度
        QString details;
    };
    TestResult runTest(const QString& code);
    QVector<TestResult> runAllTests();
    
signals:
    void testCompleted(const TestResult& result);
    void allTestsCompleted(int passed, int failed);
};
```

**界面需求**:
- [ ] 样本列表管理界面
- [ ] 添加/编辑样本对话框
- [ ] 批量测试进度显示
- [ ] 测试报告生成

---

### 8.2 相机标定向导 [P0] [8h]

**功能描述**: 引导用户完成相机参数标定

```cpp
class CalibrationWizard : public QWizard {
    Q_OBJECT
public:
    enum Page {
        PageIntro,          // 介绍
        PageCaptureGrid,    // 采集棋盘格
        PageCalculate,      // 计算标定参数
        PageVerify,         // 验证标定结果
        PagePixelRatio,     // 像素比例标定
        PageSave            // 保存结果
    };
    
    struct CalibrationResult {
        cv::Mat cameraMatrix;
        cv::Mat distCoeffs;
        double pixelPerMm;
        double reprojError;
    };
    
    CalibrationResult getResult() const;
    
private:
    // 棋盘格检测
    bool detectChessboard(const cv::Mat& image, 
                          std::vector<cv::Point2f>& corners);
    // 标定计算
    bool calibrateCamera(const std::vector<std::vector<cv::Point2f>>& imagePoints);
    // 像素比例计算
    double calculatePixelRatio(const cv::Mat& image, double realLengthMm);
};
```

**标定流程**:
1. 放置棋盘格标定板
2. 采集 10-20 张不同角度图像
3. 自动检测角点
4. 计算内参和畸变系数
5. 验证重投影误差 (< 0.5 像素)
6. 使用标尺标定像素比例
7. 保存标定结果

---

### 8.3 报警与通知系统 [P1] [4h]

```cpp
class AlarmManager : public QObject {
    Q_OBJECT
public:
    enum class NotifyMethod {
        Sound,      // 声音
        Popup,      // 弹窗
        LED,        // 三色灯
        Email,      // 邮件
        SMS         // 短信 (可选)
    };
    
    struct AlarmRule {
        QString name;
        AlarmLevel level;
        QString condition;  // 触发条件表达式
        QSet<NotifyMethod> methods;
        int cooldownSeconds;  // 冷却时间
    };
    
    void addRule(const AlarmRule& rule);
    void trigger(const QString& ruleName, const QVariantMap& context);
    void acknowledge(int alarmId, const QString& operator_);
    void resolve(int alarmId, const QString& resolution);
    
    // 声音配置
    void setSoundFile(AlarmLevel level, const QString& wavFile);
    void setVolume(int volume);  // 0-100
    void mute(bool mute);
    
private:
    QSoundEffect* m_soundEffect;
    QSystemTrayIcon* m_trayIcon;
};
```

**预置报警规则**:
| 规则名 | 级别 | 触发条件 | 通知方式 |
|--------|------|----------|----------|
| 良率过低 | Warning | yield < 95% (连续10件) | 声音+弹窗 |
| 相机断开 | Error | camera.connected == false | 声音+弹窗+LED |
| PLC断开 | Error | plc.connected == false | 声音+弹窗+LED |
| 严重缺陷 | Warning | severity >= 80 | 声音 |
| 磁盘空间不足 | Warning | disk.free < 10GB | 弹窗 |
| 检测超时 | Error | cycleTime > 200ms | 声音+弹窗 |

---

### 8.4 系统托盘与后台运行 [P1] [2h]

```cpp
class SystemTrayManager : public QObject {
    Q_OBJECT
public:
    void init(QMainWindow* mainWindow);
    void showNotification(const QString& title, const QString& message,
                          QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information);
    
private:
    QSystemTrayIcon* m_trayIcon;
    QMenu* m_trayMenu;
    
    void createTrayMenu();
    void onTrayActivated(QSystemTrayIcon::ActivationReason reason);
};

// 主窗口关闭行为
void MainWindow::closeEvent(QCloseEvent* event) {
    if (m_settings->minimizeToTray()) {
        hide();
        m_trayManager->showNotification(tr("后台运行"), 
            tr("程序已最小化到系统托盘"));
        event->ignore();
    } else {
        // 正常退出流程
        event->accept();
    }
}
```

---

### 8.5 开机自启动 [P2] [1h]

```cpp
class AutoStartManager {
public:
    static bool isEnabled();
    static bool setEnabled(bool enable);
    
private:
    // Windows: 注册表
    // HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run
    static bool setWindowsAutoStart(bool enable);
    
    // Linux: .desktop 文件
    // ~/.config/autostart/defect-detection.desktop
    static bool setLinuxAutoStart(bool enable);
};
```

---

### 8.6 软件升级机制 [P2] [6h]

```cpp
class UpdateManager : public QObject {
    Q_OBJECT
public:
    struct VersionInfo {
        QString version;
        QString releaseDate;
        QString changelog;
        QString downloadUrl;
        QString checksum;
        qint64 fileSize;
        bool mandatory;
    };
    
    // 检查更新
    void checkForUpdates();
    
    // 下载更新
    void downloadUpdate(const VersionInfo& info);
    
    // 安装更新 (需要管理员权限)
    bool installUpdate(const QString& installerPath);
    
signals:
    void updateAvailable(const VersionInfo& info);
    void downloadProgress(qint64 received, qint64 total);
    void downloadFinished(const QString& filePath);
    void updateError(const QString& error);
    
private:
    QNetworkAccessManager* m_network;
    QString m_updateServerUrl;
};
```

---

### 8.7 许可证管理 [P2] [4h]

```cpp
class LicenseManager {
public:
    enum class LicenseType {
        Trial,      // 试用版 (30天)
        Standard,   // 标准版
        Professional, // 专业版
        Enterprise  // 企业版
    };
    
    struct LicenseInfo {
        LicenseType type;
        QString customerName;
        QDate issueDate;
        QDate expiryDate;
        int maxStations;  // 最大工位数
        bool dnnEnabled;  // DNN 功能
        bool spcEnabled;  // SPC 功能
        bool apiEnabled;  // API 功能
    };
    
    bool loadLicense(const QString& licenseFile);
    bool validateLicense();
    LicenseInfo getLicenseInfo() const;
    int remainingDays() const;
    
    // 功能检查
    bool isFeatureEnabled(const QString& feature) const;
    
private:
    // RSA 签名验证
    bool verifySignature(const QByteArray& data, const QByteArray& signature);
};
```

---

## 9. 运维与诊断工具

### 9.1 日志查看器 [P1] [4h]

```cpp
class LogViewer : public QWidget {
    Q_OBJECT
public:
    explicit LogViewer(QWidget* parent = nullptr);
    
    void setLogDir(const QString& dir);
    void loadFile(const QString& filePath);
    
    // 过滤
    void setLevelFilter(QSet<QString> levels);  // info/warn/error
    void setModuleFilter(const QString& module);
    void setTimeRange(const QDateTime& start, const QDateTime& end);
    void setKeywordFilter(const QString& keyword);
    
    // 导出
    void exportFiltered(const QString& filePath);
    
private:
    QTableView* m_logTable;
    LogTableModel* m_model;
    QComboBox* m_fileSelector;
    QLineEdit* m_searchEdit;
    QCheckBox* m_autoScrollCheck;
    
    // 实时刷新
    QFileSystemWatcher* m_watcher;
};
```

---

### 9.2 系统诊断工具 [P1] [4h]

```cpp
class DiagnosticsTool : public QDialog {
    Q_OBJECT
public:
    struct DiagResult {
        QString item;
        bool passed;
        QString details;
    };
    
    void runFullDiagnostics();
    
private:
    QVector<DiagResult> m_results;
    
    // 诊断项
    DiagResult checkCamera();
    DiagResult checkPLC();
    DiagResult checkDatabase();
    DiagResult checkDiskSpace();
    DiagResult checkMemory();
    DiagResult checkCPU();
    DiagResult checkNetwork();
    DiagResult checkGoldenSamples();
    DiagResult checkModelFiles();
    DiagResult checkConfigFiles();
    
    void generateReport(const QString& filePath);
};
```

**诊断报告模板**:
```
=== 系统诊断报告 ===
生成时间: 2024-01-15 10:30:00
软件版本: 1.0.0

[硬件状态]
√ 相机连接: 正常 (海康 MV-CE060-10GC, SN: DA12345678)
√ PLC连接: 正常 (192.168.1.100:502)
× 光源控制: 异常 (COM3 无响应)

[系统资源]
√ CPU 占用: 35%
√ 内存占用: 4.2 GB / 16 GB
√ 磁盘空间: 256 GB 可用

[数据库]
√ 连接状态: 正常
√ 记录数: 125,000
√ 数据库大小: 1.2 GB

[配置检查]
√ 相机配置: 有效
√ 检测参数: 有效
√ PLC映射: 有效

[模型文件]
√ yolov5n_defect.onnx: 存在 (MD5: abc123...)
```

---

### 9.3 配置导入导出 [P1] [2h]

```cpp
class ConfigPorter {
public:
    // 导出配置包 (.zip)
    bool exportConfig(const QString& zipPath);
    
    // 导入配置包
    bool importConfig(const QString& zipPath, bool backup = true);
    
    // 包含内容
    // - config/*.json
    // - 标定文件
    // - 检测参数
    // - 用户设置
    
private:
    QStringList m_configFiles;
    
    bool createBackup();
    bool validatePackage(const QString& zipPath);
};
```

---

### 9.4 数据备份恢复 [P1] [4h]

```cpp
class BackupManager : public QObject {
    Q_OBJECT
public:
    struct BackupInfo {
        QString path;
        QDateTime timestamp;
        qint64 size;
        QString description;
    };
    
    // 手动备份
    bool createBackup(const QString& description = QString());
    
    // 自动备份 (定时)
    void setAutoBackup(bool enable, int intervalHours = 24);
    
    // 恢复
    bool restore(const QString& backupPath);
    
    // 管理
    QVector<BackupInfo> listBackups();
    bool deleteBackup(const QString& path);
    void cleanOldBackups(int keepDays = 30);
    
signals:
    void backupStarted();
    void backupProgress(int percent);
    void backupCompleted(const QString& path);
    void backupFailed(const QString& error);
    
private:
    QString m_backupDir;
    QTimer* m_autoBackupTimer;
};
```

---

## 10. 算法调优工具

### 10.1 参数调试界面 [P0] [6h]

```cpp
class AlgorithmTuner : public QWidget {
    Q_OBJECT
public:
    explicit AlgorithmTuner(QWidget* parent = nullptr);
    
    void setImage(const cv::Mat& image);
    void setDetector(std::shared_ptr<IDefectDetector> detector);
    
    // 实时预览
    void enableLivePreview(bool enable);
    
private:
    // 左侧：原图 + 标注结果
    ImageView* m_imageView;
    
    // 右侧：参数面板
    QScrollArea* m_paramScroll;
    QMap<QString, QWidget*> m_paramWidgets;
    
    // 底部：结果信息
    QLabel* m_resultLabel;
    QLabel* m_timeLabel;
    
    // 参数变化时自动重新检测
    void onParamChanged();
    void runDetection();
    
    // 生成参数控件
    QWidget* createParamWidget(const QString& name, const QVariant& value,
                               const QVariant& min, const QVariant& max);
};
```

---

### 10.2 批量测试工具 [P1] [4h]

```cpp
class BatchTester : public QDialog {
    Q_OBJECT
public:
    struct TestCase {
        QString imagePath;
        QString expectedResult;  // OK/NG
        QString expectedDefectType;
    };
    
    struct TestSummary {
        int total;
        int passed;
        int failed;
        double accuracy;
        double avgCycleTime;
        QMap<QString, int> confusionMatrix;
    };
    
    void addTestCase(const TestCase& tc);
    void loadTestCases(const QString& csvPath);
    
    void runTests();
    TestSummary getSummary() const;
    void exportResults(const QString& csvPath);
    
signals:
    void progress(int current, int total);
    void testCompleted(const TestCase& tc, bool passed, const DetectResult& result);
    void allCompleted(const TestSummary& summary);
};
```

**测试用例 CSV 格式**:
```csv
image_path,expected_result,expected_defect_type
fixtures/scratch_01.png,NG,Scratch
fixtures/crack_01.png,NG,Crack
fixtures/clean_01.png,OK,
```

---

## 11. 多工位与产线集成

### 11.1 多相机管理 [P1] [8h]

```cpp
class MultiCameraManager : public QObject {
    Q_OBJECT
public:
    struct CameraSlot {
        QString id;
        QString name;
        std::shared_ptr<ICamera> camera;
        CameraConfig config;
        bool enabled;
    };
    
    // 相机管理
    bool addCamera(const QString& id, const CameraConfig& config);
    bool removeCamera(const QString& id);
    void enableCamera(const QString& id, bool enable);
    
    // 同步采集
    void grabAll();  // 所有相机同时采集
    void grabSequential();  // 顺序采集
    
    // 配置
    void setSyncMode(bool sync);  // 是否同步触发
    
signals:
    void frameGrabbed(const QString& cameraId, const cv::Mat& frame);
    void cameraError(const QString& cameraId, const QString& error);
    
private:
    QMap<QString, CameraSlot> m_cameras;
    QThreadPool m_threadPool;
    std::atomic<bool> m_synced{false};
};
```

---

### 11.2 多工位结果聚合 [P1] [4h]

```cpp
class MultiStationAggregator : public QObject {
    Q_OBJECT
public:
    struct StationResult {
        QString stationId;
        DetectResult result;
        qint64 timestamp;
    };
    
    // 配置工位
    void addStation(const QString& id, const QString& name);
    void setAggregationRule(const QString& rule);  // ANY_NG, ALL_NG, WORST
    void setTimeout(int ms);  // 等待超时
    
    // 提交单工位结果
    void submitResult(const QString& stationId, const DetectResult& result);
    
    // 聚合结果
    DetectResult aggregate();
    
signals:
    void allStationsReady();
    void aggregationComplete(const DetectResult& finalResult);
    void stationTimeout(const QString& stationId);
    
private:
    QMap<QString, StationResult> m_results;
    QTimer* m_timeoutTimer;
    int m_expectedCount;
};
```

---

### 11.3 MES 数据上报 [P1] [6h]

```cpp
class MESReporter : public QObject {
    Q_OBJECT
public:
    struct MESConfig {
        QString serverUrl;
        QString apiKey;
        QString lineCode;
        QString stationCode;
        int retryCount = 3;
        int timeoutMs = 5000;
    };
    
    void setConfig(const MESConfig& config);
    
    // 上报检测结果
    void reportInspection(const InspectionRecord& record);
    
    // 上报产量统计
    void reportProduction(const ProductionStats& stats);
    
    // 上报报警
    void reportAlarm(const AlarmInfo& alarm);
    
    // 批量上报 (离线缓存)
    void flushPendingReports();
    
signals:
    void reportSuccess(const QString& type, const QString& id);
    void reportFailed(const QString& type, const QString& error);
    
private:
    QNetworkAccessManager* m_network;
    QQueue<QJsonObject> m_pendingReports;  // 离线缓存
    
    QJsonObject buildInspectionPayload(const InspectionRecord& record);
};
```

**MES 数据格式**:
```json
{
  "messageType": "INSPECTION_RESULT",
  "timestamp": "2024-01-15T10:30:00.123Z",
  "lineCode": "LINE-01",
  "stationCode": "ST-02",
  "data": {
    "productId": "PROD-2024-001234",
    "batchNo": "BATCH-20240115-A",
    "result": "NG",
    "defectType": "Scratch",
    "severity": 65.5,
    "cycleTimeMs": 85,
    "imageUrl": "http://192.168.1.100:8080/images/2024/01/15/001234.jpg"
  }
}
```

---

## 12. 文档要求

### 12.1 用户操作手册 [P1]

**目录结构**:
```
1. 系统概述
   1.1 功能介绍
   1.2 系统要求
   1.3 安装部署
   
2. 快速入门
   2.1 首次启动
   2.2 相机连接
   2.3 开始检测
   
3. 界面说明
   3.1 主界面
   3.2 设置界面
   3.3 统计界面
   3.4 历史查询
   
4. 操作指南
   4.1 参数配置
   4.2 ROI设置
   4.3 标定操作
   4.4 Golden Sample
   
5. 故障排查
   5.1 常见问题
   5.2 错误代码
   5.3 联系支持
```

### 12.2 运维手册 [P1]

**目录结构**:
```
1. 系统架构
2. 安装部署
   2.1 环境准备
   2.2 安装步骤
   2.3 配置说明
3. 日常维护
   3.1 日志管理
   3.2 数据备份
   3.3 磁盘清理
4. 性能监控
5. 故障处理
6. 升级流程
```

### 12.3 API 文档 [P2]

使用 OpenAPI/Swagger 规范生成。

---

## 13. 部署清单

### 13.1 安装包制作 [P1] [4h]

**Windows (Inno Setup)**:
```iss
[Setup]
AppName=DefectDetection
AppVersion=1.0.0
DefaultDirName={pf}\DefectDetection
OutputBaseFilename=DefectDetection_Setup_1.0.0

[Files]
Source: "bin\*"; DestDir: "{app}\bin"; Flags: recursesubdirs
Source: "config\*"; DestDir: "{app}\config"; Flags: recursesubdirs
Source: "models\*"; DestDir: "{app}\models"; Flags: recursesubdirs
Source: "docs\*.pdf"; DestDir: "{app}\docs"

[Icons]
Name: "{commondesktop}\DefectDetection"; Filename: "{app}\bin\DefectDetection.exe"
Name: "{group}\DefectDetection"; Filename: "{app}\bin\DefectDetection.exe"

[Run]
Filename: "{app}\bin\vcredist_x64.exe"; Parameters: "/quiet"; StatusMsg: "Installing VC++ Runtime..."
```

### 13.2 部署检查清单 [P0]

```markdown
## 部署前检查

### 硬件检查
- [ ] 工控机满足最低配置 (i5/8GB/256GB SSD)
- [ ] 相机已正确连接并安装驱动
- [ ] PLC 通信线路正常
- [ ] 光源控制器接线正确
- [ ] 网络配置正确 (静态IP)

### 软件检查
- [ ] 安装 VC++ 2019 运行库
- [ ] 安装相机 SDK (MVS/Galaxy)
- [ ] 防火墙放行端口 (8080/8081/502)
- [ ] 关闭 Windows 自动更新
- [ ] 关闭休眠/屏保

### 功能验收
- [ ] 相机采集正常
- [ ] PLC 通信正常
- [ ] 检测功能正常
- [ ] Golden Sample 测试通过
- [ ] 数据保存正常
- [ ] 72小时稳定性测试

### 文档交付
- [ ] 用户操作手册
- [ ] 运维手册
- [ ] 配置备份
- [ ] 培训记录
```

---

## 14. 修订后的里程碑规划

| 里程碑 | 周期 | 交付物 | 验收标准 |
|--------|------|--------|----------|
| **M1: 核心检测** | 1-2周 | 4个检测器 + 评分 | 节拍<100ms, Golden Sample通过 |
| **M2: 硬件集成** | 3-4周 | 相机+PLC+光源 | 硬件联调成功 |
| **M3: 数据持久化** | 5周 | 数据库+图像存储+导出 | 数据完整性验证 |
| **M4: 产品化功能** | 6-7周 | 标定+报警+诊断+备份 | 功能完整性验证 |
| **M5: 统计与报表** | 8周 | 统计图表+SPC+历史 | UI完整性验证 |
| **M6: 网络与集成** | 9周 | API+MES+多工位 | 接口测试通过 |
| **M7: 测试与优化** | 10周 | 单元测试+性能优化 | 覆盖率>70%, 稳定性72h |
| **M8: 部署交付** | 11周 | 安装包+文档+培训 | 客户验收 |

---

## 15. 工时汇总

| 模块 | P0任务 | P1任务 | P2任务 | 总计 |
|------|--------|--------|--------|------|
| 算法层 | 46h | 12h | - | 58h |
| 硬件层 | 28h | 12h | - | 40h |
| 数据层 | 12h | 8h | - | 20h |
| 网络层 | - | 12h | - | 12h |
| UI层 | 8h | 26h | 8h | 42h |
| 应用层 | 14h | 4h | - | 18h |
| 产品化 | 14h | 18h | 11h | 43h |
| 运维工具 | - | 14h | - | 14h |
| 测试 | - | 8h | - | 8h |
| 文档部署 | - | 4h | 4h | 8h |
| **总计** | **122h** | **118h** | **23h** | **263h** |

> 按每天6小时有效工作时间，约需 **44个工作日 (9周)**
