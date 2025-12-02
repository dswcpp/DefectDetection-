# OpenCV 深度技术解析

> 本文档详细介绍项目中 OpenCV 的应用，展示对计算机视觉算法的深入理解。

## 1. 图像预处理管线

### 1.1 预处理流程设计

```
原始图像
    │
    ▼
┌─────────────────┐
│  颜色空间转换   │  BGR → Gray / Lab / HSV
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│   去噪处理      │  高斯/中值/双边滤波
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│   对比度增强    │  CLAHE / 直方图均衡化
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│   二值化        │  Otsu / 自适应阈值
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│   形态学操作    │  开/闭/顶帽/底帽
└─────────────────┘
```

### 1.2 去噪算法选择策略

```cpp
cv::Mat ImagePreprocessor::denoise(const cv::Mat& input) {
  cv::Mat result;
  
  if (m_denoiseStrength <= 30) {
    // 轻度噪声：高斯模糊
    // 原理：卷积核加权平均，sigma 控制模糊程度
    // 时间复杂度：O(n) 使用可分离滤波器
    cv::GaussianBlur(input, result, cv::Size(3, 3), 0);
  } 
  else if (m_denoiseStrength <= 60) {
    // 中度噪声：双边滤波
    // 原理：同时考虑空间距离和像素值差异
    // 优势：保边去噪，不会模糊边缘
    // 参数：d=5(邻域直径), sigmaColor=50, sigmaSpace=50
    cv::bilateralFilter(input, result, 5, 50, 50);
  } 
  else {
    // 强噪声：非局部均值去噪 (NLM)
    // 原理：利用图像自相似性，搜索相似块进行加权平均
    // 优势：保持纹理细节，去噪效果最好
    // 缺点：计算量大 O(n²)
    cv::fastNlMeansDenoising(input, result, 10);
  }
  return result;
}
```

**面试要点**：
- 高斯模糊是**线性滤波**，可分离为两个1D卷积，复杂度从 O(k²) 降到 O(2k)
- 双边滤波是**非线性滤波**，权重 = 空间高斯权重 × 值域高斯权重
- NLM 利用**非局部自相似性**，在整张图搜索相似块

### 1.3 CLAHE 对比度增强

```cpp
// CrackDetector 中的应用
cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(8, 8));
clahe->apply(gray, gray);
```

**CLAHE 原理**（Contrast Limited Adaptive Histogram Equalization）：

```
1. 将图像分成 8×8 的小块（tiles）
2. 对每个小块计算直方图
3. 裁剪直方图（clipLimit=2.0），超过的像素重新分配
4. 对每个小块进行直方图均衡化
5. 使用双线性插值消除块边界
```

**为什么用 CLAHE 而不是普通直方图均衡化？**
- 普通 HE 是全局操作，可能过度增强噪声
- CLAHE 是局部自适应的，clipLimit 限制对比度增强幅度
- 裂纹通常是低对比度的细线，CLAHE 能有效增强

## 2. 边缘检测深度解析

### 2.1 Canny 边缘检测

```cpp
// ScratchDetector 中的应用
int lowThreshold = std::max(10, 100 - m_sensitivity);
int highThreshold = lowThreshold * 3;  // 推荐比例 1:3
cv::Canny(preprocessed, edges, lowThreshold, highThreshold);
```

**Canny 算法五步骤**：

```
Step 1: 高斯滤波去噪
        G(x,y) = (1/2πσ²) × exp(-(x²+y²)/2σ²)

Step 2: 计算梯度幅值和方向
        Gx = Sobel_x, Gy = Sobel_y
        G = √(Gx² + Gy²)
        θ = atan2(Gy, Gx)

Step 3: 非极大值抑制 (NMS)
        沿梯度方向，只保留局部最大值
        将边缘细化为单像素宽度

Step 4: 双阈值检测
        strong edge: G > highThreshold
        weak edge:   lowThreshold < G < highThreshold
        non-edge:    G < lowThreshold

Step 5: 边缘连接（滞后阈值）
        weak edge 连接到 strong edge 时保留
        否则抑制
```

**参数选择策略**：
```cpp
// 灵敏度越高，阈值越低，检测到的边缘越多
// 但误检也会增加，需要后续形态学处理
int lowThreshold = 100 - sensitivity;  // sensitivity: 0-100
int highThreshold = lowThreshold * 3;   // 经典比例
```

### 2.2 Hough 变换检测直线

```cpp
// ScratchDetector::detectLinesHough
std::vector<cv::Vec4i> lines;
cv::HoughLinesP(edges, lines, 
    1,              // rho: 距离分辨率 (像素)
    CV_PI / 180,    // theta: 角度分辨率 (弧度)
    50,             // threshold: 累加器阈值
    m_minLength,    // minLineLength: 最小线段长度
    10              // maxLineGap: 最大间隙
);
```

**Hough 变换原理**：

```
图像空间 (x, y) → 参数空间 (ρ, θ)

直线方程：ρ = x·cos(θ) + y·sin(θ)

1. 图像空间中的一个点 → 参数空间中的一条正弦曲线
2. 共线的点 → 参数空间中曲线交于一点
3. 累加器统计交点，超过阈值即检测到直线
```

**为什么用 HoughLinesP 而不是 HoughLines？**
- `HoughLines`: 返回 (ρ, θ)，是无限长的直线
- `HoughLinesP`: 概率霍夫变换，返回线段端点 (x1,y1,x2,y2)
- 划痕检测需要线段的具体位置和长度

## 3. 形态学操作深度应用

### 3.1 结构元素选择

```cpp
// 水平划痕检测：使用水平核
cv::Mat kernel = cv::getStructuringElement(
    cv::MORPH_RECT,     // 矩形核
    cv::Size(3, 1)      // 3×1 水平核
);

// 圆形缺陷检测：使用椭圆核
cv::Mat kernel = cv::getStructuringElement(
    cv::MORPH_ELLIPSE,  // 椭圆核
    cv::Size(5, 5)
);
```

**结构元素选择原则**：
- **MORPH_RECT**: 保持水平/垂直边缘，用于划痕
- **MORPH_ELLIPSE**: 各向同性，用于斑点/异物
- **MORPH_CROSS**: 保持细线结构

### 3.2 顶帽/底帽变换（异物检测核心）

```cpp
// ForeignDetector 中的应用
cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(15, 15));

// 顶帽：原图 - 开运算 = 提取比背景亮的小区域
cv::morphologyEx(preprocessed, tophat, cv::MORPH_TOPHAT, kernel);

// 底帽：闭运算 - 原图 = 提取比背景暗的小区域
cv::morphologyEx(preprocessed, blackhat, cv::MORPH_BLACKHAT, kernel);

// 合并：同时检测亮暗异物
cv::add(tophat, blackhat, combined);
```

**数学原理**：
```
开运算 = 腐蚀 + 膨胀 = 去除小的亮区域，保留背景
闭运算 = 膨胀 + 腐蚀 = 填充小的暗区域

顶帽 = I - open(I) = 小于结构元素的亮目标
底帽 = close(I) - I = 小于结构元素的暗目标
```

**核大小选择**：
- 核尺寸应略大于待检测异物的最大尺寸
- 15×15 对应约 15 像素的异物检测上限
- 太大会包含正常特征，太小会漏检

### 3.3 骨架化与细化

```cpp
// 裂纹中心线提取（概念代码）
cv::Mat skeleton;
cv::ximgproc::thinning(binary, skeleton, cv::ximgproc::THINNING_ZHANGSUEN);
```

**Zhang-Suen 细化算法**：
- 迭代删除边界像素，保留骨架
- 保持拓扑连通性
- 用于计算裂纹长度和分支

## 4. 轮廓分析与特征提取

### 4.1 轮廓查找与层级

```cpp
std::vector<std::vector<cv::Point>> contours;
std::vector<cv::Vec4i> hierarchy;
cv::findContours(binary, contours, hierarchy, 
    cv::RETR_EXTERNAL,       // 只检测最外层轮廓
    cv::CHAIN_APPROX_SIMPLE  // 压缩水平/垂直线段
);
```

**轮廓检索模式**：
| 模式 | 说明 | 使用场景 |
|------|------|----------|
| RETR_EXTERNAL | 只检测最外层 | 检测独立缺陷 |
| RETR_LIST | 所有轮廓，无层级 | 简单场景 |
| RETR_TREE | 完整层级树 | 嵌套结构分析 |

### 4.2 形状特征计算

```cpp
// CrackDetector 中的形状分析
double area = cv::contourArea(contour);
double perimeter = cv::arcLength(contour, true);

// 复杂度（圆形度的倒数）
// 圆形 complexity ≈ 1，裂纹 complexity >> 1
double complexity = (perimeter * perimeter) / (4 * CV_PI * area);

// 最小外接矩形
cv::RotatedRect rotRect = cv::minAreaRect(contour);
double length = std::max(rotRect.size.width, rotRect.size.height);
double width = std::min(rotRect.size.width, rotRect.size.height);
double aspectRatio = length / width;
```

**特征判别标准**：
```
划痕特征：
  - aspectRatio > 3 (细长)
  - length > minLength
  - width < maxWidth

裂纹特征：
  - complexity > 2 (非圆形)
  - 有分支结构
  - 边缘不规则

异物特征：
  - 与背景对比度高
  - 形状紧凑 (complexity 较低)
```

### 4.3 Hu 矩（形状不变特征）

```cpp
cv::Moments moments = cv::moments(contour);
double hu[7];
cv::HuMoments(moments, hu);

// hu[0]~hu[6] 对平移、缩放、旋转不变
// 可用于缺陷分类
```

**Hu 矩的性质**：
- 基于中心矩和归一化矩计算
- 7 个不变矩，对 RST 变换不变
- hu[0] 表示形状的整体大小
- hu[1] 表示形状的伸展程度

## 5. 颜色空间分析

### 5.1 Lab 颜色空间异物检测

```cpp
// ForeignDetector::detectColorAnomalies
cv::Mat lab;
cv::cvtColor(image, lab, cv::COLOR_BGR2Lab);

std::vector<cv::Mat> channels;
cv::split(lab, channels);
// channels[0] = L (亮度)
// channels[1] = a (绿-红)
// channels[2] = b (蓝-黄)

// 计算颜色统计
cv::Scalar meanA, stdA, meanB, stdB;
cv::meanStdDev(channels[1], meanA, stdA);
cv::meanStdDev(channels[2], meanB, stdB);

// 颜色异常：偏离均值超过 2.5σ
double colorThresh = std::max(stdA[0], stdB[0]) * 2.5;
```

**为什么用 Lab 而不是 RGB/HSV？**

| 颜色空间 | 优势 | 劣势 |
|----------|------|------|
| RGB | 简单直接 | 颜色信息与亮度耦合 |
| HSV | H 分量对光照不敏感 | H 在低饱和度时不稳定 |
| **Lab** | **感知均匀，欧氏距离有意义** | 计算略复杂 |

Lab 空间中，相同的欧氏距离对应相近的人眼感知差异，更适合颜色异常检测。

### 5.2 颜色直方图分析

```cpp
// 计算 H-S 二维直方图（用于颜色聚类）
cv::Mat hsv;
cv::cvtColor(image, hsv, cv::COLOR_BGR2HSV);

int hBins = 30, sBins = 32;
int histSize[] = {hBins, sBins};
float hRanges[] = {0, 180};
float sRanges[] = {0, 256};
const float* ranges[] = {hRanges, sRanges};
int channels[] = {0, 1};

cv::Mat hist;
cv::calcHist(&hsv, 1, channels, cv::Mat(), hist, 2, histSize, ranges);
```

## 6. 多尺度检测策略

### 6.1 图像金字塔

```cpp
// ScratchDetector 多尺度检测
std::vector<double> scales = {1.0, 0.5, 0.25};

for (double scale : scales) {
    cv::Mat scaled;
    if (scale < 1.0) {
        // INTER_AREA: 下采样最佳插值方法，抗锯齿
        cv::resize(preprocessed, scaled, cv::Size(), scale, scale, cv::INTER_AREA);
    }
    
    // 在当前尺度检测
    auto defects = detectAtScale(scaled);
    
    // 坐标映射回原始尺度
    for (auto& d : defects) {
        d.bbox.x /= scale;
        d.bbox.y /= scale;
        d.bbox.width /= scale;
        d.bbox.height /= scale;
    }
}
```

**多尺度检测优势**：
- 大尺度：检测宏观缺陷，速度快
- 小尺度：检测微小缺陷，细节保留
- NMS 合并各尺度结果

### 6.2 插值方法选择

| 方法 | 场景 | 说明 |
|------|------|------|
| INTER_NEAREST | 掩码图 | 最近邻，保持离散值 |
| INTER_LINEAR | 通用上采样 | 双线性，速度快 |
| INTER_AREA | **下采样** | 区域均值，抗锯齿 |
| INTER_CUBIC | 高质量上采样 | 双三次，平滑 |

## 7. 非极大值抑制 (NMS)

```cpp
std::vector<DefectInfo> nmsDefects(const std::vector<DefectInfo>& defects, double iouThreshold) {
    // 1. 按置信度降序排序
    std::vector<DefectInfo> sorted = defects;
    std::sort(sorted.begin(), sorted.end(), 
        [](const DefectInfo& a, const DefectInfo& b) {
            return a.confidence > b.confidence;
        });
    
    std::vector<bool> suppressed(sorted.size(), false);
    std::vector<DefectInfo> result;
    
    for (size_t i = 0; i < sorted.size(); ++i) {
        if (suppressed[i]) continue;
        result.push_back(sorted[i]);
        
        // 2. 抑制与当前框 IoU > threshold 的其他框
        for (size_t j = i + 1; j < sorted.size(); ++j) {
            if (suppressed[j]) continue;
            
            cv::Rect intersection = sorted[i].bbox & sorted[j].bbox;
            double iou = (double)intersection.area() / 
                (sorted[i].bbox.area() + sorted[j].bbox.area() - intersection.area());
            
            if (iou > iouThreshold) {
                suppressed[j] = true;
            }
        }
    }
    return result;
}
```

**IoU 计算**：
```
IoU = Area(A ∩ B) / Area(A ∪ B)
    = Area(A ∩ B) / (Area(A) + Area(B) - Area(A ∩ B))
```

## 8. 性能优化技巧

### 8.1 ROI 处理

```cpp
// 只处理感兴趣区域，减少计算量
cv::Rect roi(100, 100, 500, 500);
cv::Mat roiImage = image(roi);  // 浅拷贝，共享数据
processDetection(roiImage);
```

### 8.2 图像类型优化

```cpp
// 使用连续内存布局
if (!image.isContinuous()) {
    image = image.clone();
}

// 避免不必要的类型转换
cv::Mat gray;
if (image.type() == CV_8UC1) {
    gray = image;  // 已是灰度图，直接使用
} else {
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
}
```

### 8.3 并行处理

```cpp
// OpenCV 内置并行支持
cv::setNumThreads(4);  // 设置线程数

// 或使用 parallel_for_
cv::parallel_for_(cv::Range(0, images.size()), [&](const cv::Range& range) {
    for (int i = range.start; i < range.end; ++i) {
        processImage(images[i]);
    }
});
```

### 8.4 内存预分配

```cpp
// 避免循环内重复分配
cv::Mat result(image.size(), image.type());

for (int i = 0; i < iterations; ++i) {
    cv::GaussianBlur(image, result, cv::Size(5, 5), 0);
    // 交换而不是拷贝
    cv::swap(image, result);
}
```

## 9. 面试常见问题

### Q1: 为什么 Canny 边缘检测用双阈值？

**答**：双阈值实现滞后阈值(Hysteresis)，解决单阈值的困境：
- 单阈值高：边缘断裂
- 单阈值低：噪声过多

双阈值策略：
- 高于 highThreshold：确定边缘
- 低于 lowThreshold：确定非边缘
- 介于两者之间：只有连接到确定边缘时才保留

### Q2: 形态学开闭运算的区别和应用？

**答**：
- **开运算**（先腐蚀后膨胀）：去除小的亮区域，平滑边界，分离粘连
- **闭运算**（先膨胀后腐蚀）：填充小孔洞，连接断裂，平滑边界

应用：
```
二值图去噪：开运算去除白色噪点，闭运算填充黑色空洞
边缘处理：开运算使边缘变圆滑（外凸），闭运算使边缘变圆滑（内凹）
```

### Q3: 如何选择合适的图像滤波方法？

**答**：根据噪声类型选择：
- **高斯噪声**：高斯滤波、双边滤波
- **椒盐噪声**：中值滤波（非线性，保持边缘）
- **复杂纹理噪声**：NLM 非局部均值
- **需要保边**：双边滤波、导向滤波

### Q4: Lab 和 HSV 颜色空间的区别？

**答**：
- **HSV**：H(色相) S(饱和度) V(明度)，更符合人类直觉描述颜色
  - 问题：H 在 S 很低时不稳定（白色/灰色的 H 值无意义）

- **Lab**：L(亮度) a(绿-红) b(蓝-黄)，感知均匀空间
  - 相同欧氏距离 = 相同感知差异
  - 适合颜色差异计算和聚类

### Q5: 项目中如何处理大图像的性能问题？

**答**：多策略结合：
1. **图像金字塔**：先在低分辨率快速定位，再在高分辨率精确分析
2. **ROI 处理**：只处理感兴趣区域
3. **异步处理**：检测在后台线程执行，不阻塞 UI
4. **预处理缓存**：相同输入复用预处理结果
5. **并行检测**：多个检测器并行执行

## 10. 项目技术亮点总结

1. **多尺度检测**：金字塔结构，兼顾速度和精度
2. **多方法融合**：Canny + Hough + 形态学，提高召回率
3. **自适应阈值**：CLAHE + 自适应二值化，适应不同光照
4. **颜色空间分析**：Lab 空间颜色异常检测
5. **高效 NMS**：多尺度/多方法结果去重
6. **并行优化**：QtConcurrent 多检测器并行
7. **内存优化**：预处理缓存 + ROI 处理
