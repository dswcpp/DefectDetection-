# DefectDetection 待办事项清单

> 基于项目设计文档和现有源码分析，以下为接下来需要实现的工作项。

---

## 1. 算法层 (algorithm/) - 高优先级

### 1.1 检测器实现
- [ ] **ScratchDetector** - 划痕检测
  - [ ] Canny 边缘检测
  - [ ] 形态学细化处理
  - [ ] HoughLinesP 线段检测
  - [ ] 长度/角度/对比度过滤
  - [ ] 参数配置支持 (canny_low/high, min_length, max_gap)

- [ ] **CrackDetector** - 裂纹检测
  - [ ] Top-hat 暗纹理增强
  - [ ] 自适应阈值二值化
  - [ ] Zhang-Suen 细化算法
  - [ ] 连通域分析 + 骨架分支计数
  - [ ] 参数配置支持 (min_area, branch_threshold)

- [ ] **ForeignDetector** - 异物检测
  - [ ] 背景建模/光照校正
  - [ ] 高通滤波
  - [ ] 连通域筛选 (面积/圆度/偏心率)
  - [ ] 参数配置支持 (min_area, min_circularity)

- [ ] **DimensionDetector** - 尺寸测量
  - [ ] Sobel/Scharr 边缘检测
  - [ ] 轮廓提取
  - [ ] 最小外接矩形/圆拟合
  - [ ] 像素-物理单位换算
  - [ ] 公差判定

### 1.2 深度学习推理
- [ ] **DnnDetector** - OpenCV DNN 基类
  - [ ] 模型加载 (ONNX)
  - [ ] 前向推理封装
  - [ ] 后处理 (NMS, 坐标缩放)

- [ ] **YoloDetector** - YOLO 系列封装
  - [ ] YOLOv5/v8 输出解析
  - [ ] 多类别支持
  - [ ] GPU 加速支持 (CUDA)

- [ ] **ModelManager** - 模型管理
  - [ ] 模型热更新
  - [ ] 版本管理
  - [ ] 模型验证

### 1.3 预处理与后处理
- [ ] **ImagePreprocessor** - 图像预处理
  - [ ] 灰度化
  - [ ] 高斯/双边滤波
  - [ ] CLAHE 直方图均衡
  - [ ] ROI 裁剪
  - [ ] 尺寸标准化

- [ ] **DefectScorer** - 严重度评分
  - [ ] 特征量化 (长度/面积/对比度/分支数)
  - [ ] 加权融合计算
  - [ ] 分级判定 (Minor/Moderate/Severe)
  - [ ] 可配置权重

- [ ] **NMSFilter** - 非极大值抑制
- [ ] **DefectMerger** - 缺陷区域合并

### 1.4 接口完善
- [ ] **IDefectDetector** - 检测器接口定义
  - [ ] 虚析构函数
  - [ ] detect() 纯虚函数
  - [ ] setParams() 参数设置
  - [ ] name() 检测器名称

- [ ] **DetectorFactory** - 检测器工厂
  - [ ] 注册机制
  - [ ] 按名称创建检测器

---

## 2. 硬件抽象层 (hal/) - 高优先级

### 2.1 相机驱动
- [ ] **HikCamera** - 海康相机 SDK
  - [ ] MVS SDK 集成
  - [ ] 设备枚举
  - [ ] 参数设置 (曝光/增益/触发)
  - [ ] 图像采集
  - [ ] 错误处理与重连

- [ ] **DahengCamera** - 大恒相机 SDK
  - [ ] Galaxy SDK 集成
  - [ ] 设备枚举
  - [ ] 参数设置
  - [ ] 图像采集

- [ ] **GigECamera** - GigE Vision 通用相机
  - [ ] GenICam 标准支持
  - [ ] 设备发现
  - [ ] 流控制

- [ ] **USBCamera** - USB 相机
  - [ ] OpenCV VideoCapture 封装
  - [ ] 分辨率设置
  - [ ] 帧率控制

### 2.2 PLC 通信
- [ ] **ModbusTCPClient** - Modbus TCP
  - [ ] libmodbus 集成
  - [ ] 连接管理 (超时/重连)
  - [ ] 寄存器读写
  - [ ] 线圈操作
  - [ ] 心跳保活

- [ ] **ModbusRTUClient** - Modbus RTU
  - [ ] 串口通信
  - [ ] 帧格式处理

- [ ] **SiemensS7Client** - 西门子 S7 协议
  - [ ] snap7 库集成
  - [ ] DB 块读写

- [ ] **MitsubishiMCClient** - 三菱 MC 协议
  - [ ] TCP/UDP 支持
  - [ ] 软元件读写

### 2.3 光源控制
- [ ] **SerialLightController** - 串口光源控制
  - [ ] 通道亮度设置
  - [ ] 频闪控制
  - [ ] 协议封装

- [ ] **ModbusLightController** - Modbus 光源控制

### 2.4 IO 控制
- [ ] **SerialIO** - 串口 IO
  - [ ] 输入/输出信号读写
  - [ ] 触发信号处理

- [ ] **GPIOController** - Linux GPIO (可选)

---

## 3. 数据层 (data/) - 中优先级

### 3.1 数据库管理
- [ ] 数据库自动初始化
  - [ ] 首次启动执行 schema.sql
  - [ ] 版本迁移支持

- [ ] 事务封装完善

### 3.2 Repository 完善
- [ ] **InspectionRepository** - 检测记录
  - [ ] 插入检测结果
  - [ ] 按时间/结果查询
  - [ ] 统计查询 (今日/本周/本月)
  - [ ] 分页查询

- [ ] **DefectRepository** - 缺陷记录
  - [ ] 插入缺陷详情
  - [ ] 按类型/严重度查询
  - [ ] 缺陷分布统计

- [ ] **ConfigRepository** - 配置存储
  - [ ] 配置版本管理
  - [ ] 配置历史记录

### 3.3 图像存储
- [ ] **ImageStorage** - 图像存储管理
  - [ ] 按日期目录组织
  - [ ] 原图/标注图/缩略图
  - [ ] 压缩存储
  - [ ] NAS 支持

- [ ] **BackupManager** - 备份管理
  - [ ] 定时备份
  - [ ] 自动清理过期数据

### 3.4 数据导出
- [ ] **CSVExporter** - CSV 导出
- [ ] **ExcelExporter** - Excel 导出
- [ ] **ReportGenerator** - PDF 报表生成

---

## 4. 网络层 (network/) - 中优先级

### 4.1 HTTP 服务
- [ ] **HttpServer** - REST API 服务
  - [ ] Qt HTTP Server 集成
  - [ ] 路由注册
  - [ ] JSON 响应
  - [ ] 认证中间件

- [ ] **ApiRoutes** - API 路由
  - [ ] GET /api/v1/inspection/{id}
  - [ ] GET /api/v1/inspections (列表)
  - [ ] GET /api/v1/statistics/overview
  - [ ] POST /api/v1/system/control

### 4.2 WebSocket 推送
- [ ] **WSServer** - WebSocket 服务
  - [ ] 检测结果实时推送
  - [ ] 系统告警推送
  - [ ] 心跳机制

### 4.3 MES 对接
- [ ] **MESClient** - MES 客户端
  - [ ] 数据上报
  - [ ] 指令接收
  - [ ] 协议适配

---

## 5. 用户界面 (ui/) - 中优先级

### 5.1 视图完善
- [ ] **DetectView** - 检测视图
  - [ ] 接入真实检测流水线
  - [ ] 多工位切换

- [ ] **StatisticsView** - 统计视图
  - [ ] 良率趋势图
  - [ ] 缺陷分布饼图
  - [ ] 节拍统计

- [ ] **HistoryView** - 历史记录
  - [ ] 分页表格
  - [ ] 筛选条件 (时间/结果/类型)
  - [ ] 图像预览
  - [ ] 数据导出

- [ ] **SPCView** - SPC 控制图
  - [ ] X-bar R 图
  - [ ] P 图
  - [ ] CP/CPK 计算
  - [ ] 控制限配置

### 5.2 对话框完善
- [ ] **AlarmDialog** - 报警对话框
  - [ ] 报警列表显示
  - [ ] 报警确认/处理
  - [ ] 历史报警查询

- [ ] **CalibrationDialog** - 标定向导
  - [ ] 相机标定
  - [ ] 像素-物理比例标定

- [ ] **LoginDialog** - 登录对话框
  - [ ] 用户认证
  - [ ] 权限验证
  - [ ] 登录超时

### 5.3 控件完善
- [ ] **SeverityBar** - 严重度进度条
- [ ] **ROIEditor** - ROI 可视化编辑
  - [ ] 多ROI支持
  - [ ] 拖拽调整

### 5.4 用户体验
- [ ] 快捷键完善
- [ ] 全屏模式
- [ ] 多语言支持 (en_US)
- [ ] 浅色主题

---

## 6. 应用层 (app/) - 高优先级

### 6.1 流程控制
- [ ] **FlowController** - 状态机
  - [ ] 状态管理 (Idle/Initializing/Ready/Detecting/Error)
  - [ ] 状态转换逻辑
  - [ ] 错误恢复

- [ ] **DetectPipeline** - 检测流水线
  - [ ] 接入真实检测算法
  - [ ] 多检测器并行执行
  - [ ] 结果聚合

### 6.2 系统监控
- [ ] **SystemWatchdog** - 看门狗
  - [ ] 模块心跳监控
  - [ ] 超时告警
  - [ ] 自动恢复

- [ ] **ResultAggregator** - 多工位结果聚合
  - [ ] 等待所有工位结果
  - [ ] 综合判定

### 6.3 配置管理
- [ ] **ConfigValidator** - 配置校验
  - [ ] 参数范围校验
  - [ ] 依赖关系校验
  - [ ] 错误提示

- [ ] 配置热加载
  - [ ] 文件监控
  - [ ] 动态更新
  - [ ] 失败回滚

---

## 7. 测试 (tests/) - 中优先级

### 7.1 单元测试
- [ ] 算法测试
  - [ ] 各检测器测试用例
  - [ ] 评分算法测试

- [ ] 数据层测试
  - [ ] Repository CRUD 测试
  - [ ] 事务测试

- [ ] 配置测试
  - [ ] JSON 序列化/反序列化
  - [ ] 校验逻辑

### 7.2 集成测试
- [ ] 流水线端到端测试
- [ ] 相机采集测试
- [ ] PLC 通信测试

### 7.3 性能测试
- [ ] 节拍测试 (目标 ≤100ms)
- [ ] 内存泄漏检测
- [ ] 72小时稳定性测试

---

## 8. 工程化 - 低优先级

### 8.1 构建系统
- [ ] CMakeLists.txt 支持
- [ ] vcpkg/Conan 包管理
- [ ] CI/CD 配置 (GitHub Actions)

### 8.2 打包部署
- [ ] Windows 安装包 (NSIS/Inno Setup)
- [ ] Linux 打包 (AppImage/deb)
- [ ] windeployqt 自动化脚本

### 8.3 文档
- [ ] API 文档生成 (Doxygen)
- [ ] 用户手册
- [ ] 运维手册

---

## 优先级说明

| 优先级 | 说明 | 建议时间 |
|--------|------|----------|
| 高 | 核心功能，系统可运行的基础 | 第1-2周 |
| 中 | 重要功能，完整产品所需 | 第3-4周 |
| 低 | 增强功能，可后续迭代 | 第5周+ |

---

## 建议实施顺序

1. **第一阶段 - 核心检测**
   - 实现 ScratchDetector 和 CrackDetector
   - 接入 DetectPipeline
   - 完善 DefectScorer

2. **第二阶段 - 硬件集成**
   - HikCamera SDK 集成
   - ModbusTCPClient 实现
   - 触发信号处理

3. **第三阶段 - 数据持久化**
   - 数据库初始化
   - InspectionRepository 完善
   - 图像存储

4. **第四阶段 - 功能完善**
   - 统计视图
   - 历史查询
   - 数据导出

5. **第五阶段 - 产线部署**
   - 系统稳定性优化
   - 性能调优
   - 打包部署
