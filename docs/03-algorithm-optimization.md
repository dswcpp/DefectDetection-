# 检测算法优化空间分析

## 1. 当前算法问题诊断

### 1.1 ScratchDetector（划痕检测）

**当前实现**：
```cpp
Canny边缘 → 形态学连接 → 轮廓分析 + Hough线检测
```

**存在问题**：
| 问题 | 影响 | 严重程度 |
|------|------|----------|
| 固定 Canny 阈值比例 (1:3) | 不同光照下效果差异大 | ⭐⭐⭐ |
| 未考虑划痕方向一致性 | 噪声边缘误检为划痕 | ⭐⭐⭐ |
| 多尺度检测冗余 | 速度慢，重复检测 | ⭐⭐ |
| 缺乏梯度方向分析 | 无法区分划痕和纹理 | ⭐⭐ |

### 1.2 CrackDetector（裂纹检测）

**当前实现**：
```cpp
CLAHE增强 → 自适应二值化 → 形态学闭运算 → 轮廓复杂度分析
```

**存在问题**：
| 问题 | 影响 | 严重程度 |
|------|------|----------|
| 未使用骨架化 | 无法准确测量裂纹长度 | ⭐⭐⭐ |
| 缺乏分支点检测 | 无法识别裂纹网络 | ⭐⭐⭐ |
| 复杂度阈值固定 | 漏检细小裂纹 | ⭐⭐ |
| 未分析裂纹走向 | 无法判断扩展趋势 | ⭐⭐ |

### 1.3 ForeignDetector（异物检测）

**当前实现**：
```cpp
顶帽/底帽变换 → 二值化 → Lab颜色异常检测
```

**存在问题**：
| 问题 | 影响 | 严重程度 |
|------|------|----------|
| 核大小固定 (15×15) | 大/小异物检测不佳 | ⭐⭐⭐ |
| 缺乏纹理分析 | 无法检测纹理异常 | ⭐⭐ |
| 颜色阈值 2.5σ 固定 | 不同产品适应性差 | ⭐⭐ |
| 未使用 Blob 检测 | 形状分析不够精细 | ⭐ |

### 1.4 DimensionDetector（尺寸检测）

**当前实现**：
```cpp
Otsu二值化 → 最大轮廓 → 最小外接矩形
```

**存在问题**：
| 问题 | 影响 | 严重程度 |
|------|------|----------|
| 仅测量外接矩形 | 无法测量任意位置 | ⭐⭐⭐ |
| 缺乏亚像素精度 | 测量精度受限于像素 | ⭐⭐⭐ |
| 不支持圆形测量 | 圆孔直径无法测量 | ⭐⭐ |
| 单一二值化方法 | 边缘模糊时失效 | ⭐⭐ |

---

## 2. 优化方案

### 2.1 ScratchDetector 优化

#### 2.1.1 自适应 Canny 阈值

```cpp
// 基于 Otsu 自动计算 Canny 阈值
cv::Mat computeAdaptiveCannyThreshold(const cv::Mat& gray, int& low, int& high) {
    // 计算 Otsu 阈值作为参考
    double otsuThresh = cv::threshold(gray, cv::Mat(), 0, 255, 
                                       cv::THRESH_BINARY | cv::THRESH_OTSU);
    
    // 基于 Otsu 阈值和图像统计信息计算 Canny 阈值
    cv::Scalar mean, stddev;
    cv::meanStdDev(gray, mean, stddev);
    
    // 自适应阈值：考虑图像对比度
    low = std::max(10, static_cast<int>(mean[0] - stddev[0]));
    high = std::min(255, static_cast<int>(mean[0] + stddev[0]));
    
    // 确保合理比例
    if (high < low * 2) high = low * 2;
    
    cv::Mat edges;
    cv::Canny(gray, edges, low, high);
    return edges;
}
```

#### 2.1.2 方向一致性过滤

```cpp
// 使用 Gabor 滤波器检测特定方向的划痕
std::vector<DefectInfo> detectDirectionalScratches(const cv::Mat& gray) {
    std::vector<DefectInfo> allDefects;
    
    // Gabor 滤波器参数
    int ksize = 31;
    double sigma = 4.0;
    double lambd = 10.0;
    double gamma = 0.5;
    double psi = 0;
    
    // 多方向检测 (0°, 45°, 90°, 135°)
    std::vector<double> thetas = {0, CV_PI/4, CV_PI/2, 3*CV_PI/4};
    
    for (double theta : thetas) {
        cv::Mat kernel = cv::getGaborKernel(
            cv::Size(ksize, ksize), sigma, theta, lambd, gamma, psi
        );
        
        cv::Mat response;
        cv::filter2D(gray, response, CV_32F, kernel);
        
        // 阈值化响应
        cv::Mat binary;
        cv::threshold(response, binary, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
        binary.convertTo(binary, CV_8U);
        
        // 查找该方向的划痕
        auto defects = findContourDefects(binary, theta);
        allDefects.insert(allDefects.end(), defects.begin(), defects.end());
    }
    
    return nmsDefects(allDefects, 0.5);
}
```

#### 2.1.3 梯度方向直方图 (HOG) 特征

```cpp
// 使用 HOG 特征区分划痕和纹理
bool isRealScratch(const cv::Mat& roi) {
    // 计算梯度
    cv::Mat gx, gy;
    cv::Sobel(roi, gx, CV_32F, 1, 0);
    cv::Sobel(roi, gy, CV_32F, 0, 1);
    
    // 梯度方向
    cv::Mat angle;
    cv::phase(gx, gy, angle, true);  // 角度制
    
    // 计算方向直方图
    int bins = 9;
    std::vector<float> hist(bins, 0);
    
    for (int y = 0; y < angle.rows; ++y) {
        for (int x = 0; x < angle.cols; ++x) {
            float a = angle.at<float>(y, x);
            int bin = static_cast<int>(a / 20) % bins;
            float mag = std::sqrt(gx.at<float>(y,x)*gx.at<float>(y,x) + 
                                  gy.at<float>(y,x)*gy.at<float>(y,x));
            hist[bin] += mag;
        }
    }
    
    // 划痕特征：方向集中度高
    float maxVal = *std::max_element(hist.begin(), hist.end());
    float sum = std::accumulate(hist.begin(), hist.end(), 0.0f);
    float dominance = maxVal / (sum + 1e-6);
    
    return dominance > 0.4;  // 主方向占比超过 40%
}
```

### 2.2 CrackDetector 优化

#### 2.2.1 骨架化与分支检测

```cpp
// Zhang-Suen 细化算法实现
cv::Mat thinning(const cv::Mat& binary) {
    cv::Mat skeleton = binary.clone();
    cv::Mat prev;
    
    do {
        prev = skeleton.clone();
        thinningIteration(skeleton, 0);  // 子迭代 1
        thinningIteration(skeleton, 1);  // 子迭代 2
    } while (cv::countNonZero(skeleton != prev) > 0);
    
    return skeleton;
}

// 检测骨架分支点（8邻域有3+个连接）
std::vector<cv::Point> findBranchPoints(const cv::Mat& skeleton) {
    std::vector<cv::Point> branchPoints;
    
    for (int y = 1; y < skeleton.rows - 1; ++y) {
        for (int x = 1; x < skeleton.cols - 1; ++x) {
            if (skeleton.at<uchar>(y, x) == 0) continue;
            
            // 统计 8 邻域连接数
            int connections = 0;
            int prev = skeleton.at<uchar>(y-1, x+1) > 0 ? 1 : 0;
            
            // 顺时针遍历 8 邻域
            int dx[] = {1, 1, 0, -1, -1, -1, 0, 1};
            int dy[] = {0, 1, 1, 1, 0, -1, -1, -1};
            
            for (int i = 0; i < 8; ++i) {
                int curr = skeleton.at<uchar>(y+dy[i], x+dx[i]) > 0 ? 1 : 0;
                if (prev == 0 && curr == 1) connections++;
                prev = curr;
            }
            
            if (connections >= 3) {
                branchPoints.push_back(cv::Point(x, y));
            }
        }
    }
    return branchPoints;
}

// 裂纹网络分析
struct CrackNetwork {
    std::vector<std::vector<cv::Point>> segments;  // 裂纹段
    std::vector<cv::Point> branchPoints;           // 分支点
    std::vector<cv::Point> endpoints;              // 端点
    double totalLength;                            // 总长度
    int branchCount;                               // 分支数
};

CrackNetwork analyzeCrackNetwork(const cv::Mat& skeleton) {
    CrackNetwork network;
    network.branchPoints = findBranchPoints(skeleton);
    network.endpoints = findEndpoints(skeleton);
    network.segments = traceSegments(skeleton, network.branchPoints);
    
    network.totalLength = 0;
    for (const auto& seg : network.segments) {
        network.totalLength += cv::arcLength(seg, false);
    }
    network.branchCount = network.branchPoints.size();
    
    return network;
}
```

#### 2.2.2 裂纹走向与扩展预测

```cpp
// 分析裂纹主方向（用于预测扩展趋势）
double analyzeCrackDirection(const std::vector<cv::Point>& crackPoints) {
    if (crackPoints.size() < 5) return 0;
    
    // PCA 分析主方向
    cv::Mat data(crackPoints.size(), 2, CV_32F);
    for (size_t i = 0; i < crackPoints.size(); ++i) {
        data.at<float>(i, 0) = crackPoints[i].x;
        data.at<float>(i, 1) = crackPoints[i].y;
    }
    
    cv::PCA pca(data, cv::Mat(), cv::PCA::DATA_AS_ROW);
    
    // 主成分方向
    cv::Point2f eigenVec(pca.eigenvectors.at<float>(0, 0),
                         pca.eigenvectors.at<float>(0, 1));
    
    double angle = std::atan2(eigenVec.y, eigenVec.x) * 180 / CV_PI;
    return angle;
}
```

### 2.3 ForeignDetector 优化

#### 2.3.1 多尺度形态学

```cpp
// 多尺度顶帽/底帽变换
cv::Mat multiScaleMorphology(const cv::Mat& gray) {
    cv::Mat combined = cv::Mat::zeros(gray.size(), CV_32F);
    
    // 多个核大小
    std::vector<int> kernelSizes = {7, 15, 25, 35};
    
    for (int ksize : kernelSizes) {
        cv::Mat kernel = cv::getStructuringElement(
            cv::MORPH_ELLIPSE, cv::Size(ksize, ksize)
        );
        
        cv::Mat tophat, blackhat;
        cv::morphologyEx(gray, tophat, cv::MORPH_TOPHAT, kernel);
        cv::morphologyEx(gray, blackhat, cv::MORPH_BLACKHAT, kernel);
        
        cv::Mat scale;
        cv::add(tophat, blackhat, scale);
        scale.convertTo(scale, CV_32F);
        
        // 加权融合（小核权重高，检测小异物）
        float weight = 1.0f / ksize;
        combined += scale * weight;
    }
    
    cv::normalize(combined, combined, 0, 255, cv::NORM_MINMAX);
    combined.convertTo(combined, CV_8U);
    return combined;
}
```

#### 2.3.2 LBP 纹理分析

```cpp
// 局部二值模式 (LBP) 纹理检测
cv::Mat computeLBP(const cv::Mat& gray) {
    cv::Mat lbp = cv::Mat::zeros(gray.size(), CV_8U);
    
    for (int y = 1; y < gray.rows - 1; ++y) {
        for (int x = 1; x < gray.cols - 1; ++x) {
            uchar center = gray.at<uchar>(y, x);
            uchar code = 0;
            
            // 8 邻域比较
            code |= (gray.at<uchar>(y-1, x-1) >= center) << 7;
            code |= (gray.at<uchar>(y-1, x  ) >= center) << 6;
            code |= (gray.at<uchar>(y-1, x+1) >= center) << 5;
            code |= (gray.at<uchar>(y  , x+1) >= center) << 4;
            code |= (gray.at<uchar>(y+1, x+1) >= center) << 3;
            code |= (gray.at<uchar>(y+1, x  ) >= center) << 2;
            code |= (gray.at<uchar>(y+1, x-1) >= center) << 1;
            code |= (gray.at<uchar>(y  , x-1) >= center) << 0;
            
            lbp.at<uchar>(y, x) = code;
        }
    }
    return lbp;
}

// 检测纹理异常区域
std::vector<cv::Rect> detectTextureAnomalies(const cv::Mat& gray, int blockSize = 32) {
    cv::Mat lbp = computeLBP(gray);
    std::vector<cv::Rect> anomalies;
    
    // 计算全局 LBP 直方图
    cv::Mat globalHist;
    int histSize = 256;
    float range[] = {0, 256};
    const float* histRange = {range};
    cv::calcHist(&lbp, 1, 0, cv::Mat(), globalHist, 1, &histSize, &histRange);
    cv::normalize(globalHist, globalHist, 1, 0, cv::NORM_L1);
    
    // 滑动窗口检测局部异常
    for (int y = 0; y < lbp.rows - blockSize; y += blockSize/2) {
        for (int x = 0; x < lbp.cols - blockSize; x += blockSize/2) {
            cv::Rect roi(x, y, blockSize, blockSize);
            cv::Mat block = lbp(roi);
            
            cv::Mat localHist;
            cv::calcHist(&block, 1, 0, cv::Mat(), localHist, 1, &histSize, &histRange);
            cv::normalize(localHist, localHist, 1, 0, cv::NORM_L1);
            
            // 卡方距离
            double dist = cv::compareHist(globalHist, localHist, cv::HISTCMP_CHISQR);
            
            if (dist > 50) {  // 阈值可调
                anomalies.push_back(roi);
            }
        }
    }
    
    return anomalies;
}
```

#### 2.3.3 SimpleBlobDetector

```cpp
// 使用 Blob 检测器检测圆形异物
std::vector<cv::KeyPoint> detectBlobs(const cv::Mat& gray) {
    cv::SimpleBlobDetector::Params params;
    
    // 阈值参数
    params.minThreshold = 10;
    params.maxThreshold = 200;
    params.thresholdStep = 10;
    
    // 面积过滤
    params.filterByArea = true;
    params.minArea = 25;
    params.maxArea = 5000;
    
    // 圆度过滤
    params.filterByCircularity = true;
    params.minCircularity = 0.5;
    
    // 凸度过滤
    params.filterByConvexity = true;
    params.minConvexity = 0.8;
    
    // 惯性比过滤
    params.filterByInertia = true;
    params.minInertiaRatio = 0.5;
    
    cv::Ptr<cv::SimpleBlobDetector> detector = 
        cv::SimpleBlobDetector::create(params);
    
    std::vector<cv::KeyPoint> keypoints;
    detector->detect(gray, keypoints);
    
    return keypoints;
}
```

### 2.4 DimensionDetector 优化

#### 2.4.1 亚像素边缘检测

```cpp
// 亚像素精度边缘定位
cv::Point2f subpixelEdge(const cv::Mat& gray, cv::Point seed, int direction) {
    // direction: 0=水平, 1=垂直
    
    // 提取边缘法线方向的像素值
    std::vector<float> profile;
    int halfWidth = 5;
    
    for (int i = -halfWidth; i <= halfWidth; ++i) {
        int x = seed.x + (direction == 0 ? i : 0);
        int y = seed.y + (direction == 1 ? i : 0);
        
        if (x >= 0 && x < gray.cols && y >= 0 && y < gray.rows) {
            profile.push_back(gray.at<uchar>(y, x));
        }
    }
    
    // 高斯拟合找边缘精确位置
    // 对梯度曲线拟合抛物线
    std::vector<float> gradient(profile.size() - 1);
    for (size_t i = 0; i < gradient.size(); ++i) {
        gradient[i] = std::abs(profile[i+1] - profile[i]);
    }
    
    // 找梯度峰值
    auto maxIt = std::max_element(gradient.begin(), gradient.end());
    int maxIdx = std::distance(gradient.begin(), maxIt);
    
    // 抛物线插值
    if (maxIdx > 0 && maxIdx < gradient.size() - 1) {
        float y0 = gradient[maxIdx - 1];
        float y1 = gradient[maxIdx];
        float y2 = gradient[maxIdx + 1];
        
        float delta = 0.5f * (y0 - y2) / (y0 - 2*y1 + y2);
        float subpixelOffset = maxIdx + delta - halfWidth;
        
        if (direction == 0) {
            return cv::Point2f(seed.x + subpixelOffset, seed.y);
        } else {
            return cv::Point2f(seed.x, seed.y + subpixelOffset);
        }
    }
    
    return cv::Point2f(seed.x, seed.y);
}
```

#### 2.4.2 圆形测量

```cpp
// 霍夫圆检测 + 椭圆拟合
struct CircleMeasurement {
    cv::Point2f center;
    float radius;
    float confidence;
};

CircleMeasurement measureCircle(const cv::Mat& gray, const cv::Rect& roi) {
    cv::Mat roiImg = gray(roi);
    
    // 边缘检测
    cv::Mat edges;
    cv::Canny(roiImg, edges, 50, 150);
    
    // 霍夫圆检测
    std::vector<cv::Vec3f> circles;
    cv::HoughCircles(roiImg, circles, cv::HOUGH_GRADIENT, 1,
                     roiImg.rows / 4,  // minDist
                     100, 30,          // param1, param2
                     10, 0);           // minRadius, maxRadius (0=无限制)
    
    if (!circles.empty()) {
        // 取最佳圆
        cv::Vec3f best = circles[0];
        
        CircleMeasurement result;
        result.center = cv::Point2f(best[0] + roi.x, best[1] + roi.y);
        result.radius = best[2];
        
        // 使用边缘点精确拟合
        std::vector<cv::Point> edgePoints;
        cv::findNonZero(edges, edgePoints);
        
        if (edgePoints.size() >= 5) {
            cv::RotatedRect ellipse = cv::fitEllipse(edgePoints);
            result.radius = (ellipse.size.width + ellipse.size.height) / 4;
            result.center = ellipse.center + cv::Point2f(roi.x, roi.y);
        }
        
        result.confidence = 0.9;
        return result;
    }
    
    return CircleMeasurement{cv::Point2f(0,0), 0, 0};
}
```

#### 2.4.3 任意两点测量

```cpp
// 卡尺工具：测量两条边之间的距离
struct CaliperResult {
    cv::Point2f edge1;      // 第一条边位置
    cv::Point2f edge2;      // 第二条边位置
    float distance;         // 距离（像素）
    float angle;            // 测量方向角度
};

CaliperResult measureWithCaliper(const cv::Mat& gray, 
                                  cv::Point2f start, 
                                  cv::Point2f end,
                                  int searchWidth = 20) {
    CaliperResult result;
    
    // 计算测量方向
    cv::Point2f direction = end - start;
    float length = cv::norm(direction);
    direction /= length;
    
    cv::Point2f normal(-direction.y, direction.x);
    
    // 沿测量线提取像素
    std::vector<float> profile;
    for (float t = 0; t < length; t += 1.0f) {
        cv::Point2f pt = start + direction * t;
        
        // 在法线方向取平均（抗噪）
        float sum = 0;
        int count = 0;
        for (int n = -searchWidth/2; n <= searchWidth/2; ++n) {
            cv::Point2f samplePt = pt + normal * n;
            int x = cvRound(samplePt.x);
            int y = cvRound(samplePt.y);
            if (x >= 0 && x < gray.cols && y >= 0 && y < gray.rows) {
                sum += gray.at<uchar>(y, x);
                count++;
            }
        }
        profile.push_back(sum / count);
    }
    
    // 计算梯度，找边缘
    std::vector<float> gradient(profile.size() - 1);
    for (size_t i = 0; i < gradient.size(); ++i) {
        gradient[i] = profile[i+1] - profile[i];
    }
    
    // 找第一个正边缘（从暗到亮）
    int edge1Idx = -1;
    for (size_t i = 0; i < gradient.size(); ++i) {
        if (gradient[i] > 20) {
            edge1Idx = i;
            break;
        }
    }
    
    // 找第一个负边缘（从亮到暗）
    int edge2Idx = -1;
    for (size_t i = edge1Idx + 10; i < gradient.size(); ++i) {
        if (gradient[i] < -20) {
            edge2Idx = i;
            break;
        }
    }
    
    if (edge1Idx >= 0 && edge2Idx >= 0) {
        result.edge1 = start + direction * edge1Idx;
        result.edge2 = start + direction * edge2Idx;
        result.distance = edge2Idx - edge1Idx;
        result.angle = std::atan2(direction.y, direction.x) * 180 / CV_PI;
    }
    
    return result;
}
```

---

## 3. 深度学习集成

### 3.1 YOLOv5/v8 目标检测

```cpp
class YoloDetector : public BaseDetector {
public:
    bool initialize() override {
        try {
            // 加载 ONNX 模型
            m_net = cv::dnn::readNetFromONNX(m_modelPath.toStdString());
            
            // 设置推理后端
            m_net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
            m_net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
            // 或使用 CUDA
            // m_net.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
            // m_net.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);
            
            m_initialized = true;
            return true;
        } catch (const cv::Exception& e) {
            LOG_ERROR("Failed to load YOLO model: {}", e.what());
            return false;
        }
    }
    
    DetectionResult detect(const cv::Mat& image) override {
        if (!m_initialized) {
            return makeErrorResult("Model not initialized");
        }
        
        QElapsedTimer timer;
        timer.start();
        
        // 预处理：letterbox resize
        cv::Mat blob;
        cv::Mat resized = letterbox(image, cv::Size(640, 640));
        cv::dnn::blobFromImage(resized, blob, 1.0/255.0, 
                               cv::Size(640, 640), 
                               cv::Scalar(), true, false);
        
        // 推理
        m_net.setInput(blob);
        std::vector<cv::Mat> outputs;
        m_net.forward(outputs, m_net.getUnconnectedOutLayersNames());
        
        // 后处理
        std::vector<DefectInfo> defects = postprocess(outputs, image.size());
        
        return makeSuccessResult(defects, timer.elapsed());
    }
    
private:
    cv::Mat letterbox(const cv::Mat& image, cv::Size targetSize) {
        float scale = std::min(
            targetSize.width / (float)image.cols,
            targetSize.height / (float)image.rows
        );
        
        int newW = int(image.cols * scale);
        int newH = int(image.rows * scale);
        
        cv::Mat resized;
        cv::resize(image, resized, cv::Size(newW, newH));
        
        cv::Mat result = cv::Mat::zeros(targetSize, image.type());
        resized.copyTo(result(cv::Rect(0, 0, newW, newH)));
        
        return result;
    }
    
    cv::dnn::Net m_net;
    QString m_modelPath;
    float m_confThreshold = 0.5;
    float m_nmsThreshold = 0.4;
};
```

### 3.2 U-Net 语义分割

```cpp
// 用于像素级缺陷分割
class UNetDetector : public BaseDetector {
public:
    DetectionResult detect(const cv::Mat& image) override {
        // 预处理
        cv::Mat input;
        cv::resize(image, input, cv::Size(512, 512));
        input.convertTo(input, CV_32F, 1.0/255.0);
        
        cv::Mat blob = cv::dnn::blobFromImage(input);
        
        // 推理
        m_net.setInput(blob);
        cv::Mat output = m_net.forward();
        
        // 输出是 [1, num_classes, H, W]
        // 转换为分割掩码
        cv::Mat mask = postprocessSegmentation(output);
        
        // 从掩码提取缺陷轮廓
        return extractDefectsFromMask(mask, image);
    }
    
private:
    cv::Mat postprocessSegmentation(const cv::Mat& output) {
        // 取 argmax 得到类别
        std::vector<cv::Mat> channels;
        cv::dnn::imagesFromBlob(output, channels);
        
        cv::Mat mask = cv::Mat::zeros(channels[0].size(), CV_8U);
        
        for (int y = 0; y < mask.rows; ++y) {
            for (int x = 0; x < mask.cols; ++x) {
                float maxVal = 0;
                int maxIdx = 0;
                for (size_t c = 0; c < channels.size(); ++c) {
                    float val = channels[c].at<float>(y, x);
                    if (val > maxVal) {
                        maxVal = val;
                        maxIdx = c;
                    }
                }
                mask.at<uchar>(y, x) = maxIdx;
            }
        }
        
        return mask;
    }
};
```

---

## 4. 自适应参数优化

### 4.1 基于图像统计的自动参数调整

```cpp
struct ImageStats {
    double mean;
    double stddev;
    double contrast;
    double brightness;
    double noiseLevel;
};

ImageStats analyzeImage(const cv::Mat& gray) {
    ImageStats stats;
    
    cv::Scalar meanVal, stddevVal;
    cv::meanStdDev(gray, meanVal, stddevVal);
    
    stats.mean = meanVal[0];
    stats.stddev = stddevVal[0];
    stats.brightness = meanVal[0] / 255.0;
    stats.contrast = stddevVal[0] / 128.0;
    
    // 估计噪声水平（使用拉普拉斯算子）
    cv::Mat laplacian;
    cv::Laplacian(gray, laplacian, CV_64F);
    cv::Scalar lapMean, lapStddev;
    cv::meanStdDev(laplacian, lapMean, lapStddev);
    stats.noiseLevel = lapStddev[0] / 100.0;
    
    return stats;
}

QVariantMap adaptParameters(const ImageStats& stats) {
    QVariantMap params;
    
    // 根据图像特性调整参数
    
    // 低对比度图像：降低边缘检测阈值
    if (stats.contrast < 0.3) {
        params["cannyLow"] = 30;
        params["cannyHigh"] = 90;
    } else {
        params["cannyLow"] = 50;
        params["cannyHigh"] = 150;
    }
    
    // 高噪声图像：增加去噪强度
    if (stats.noiseLevel > 0.5) {
        params["denoiseStrength"] = 60;
        params["morphKernelSize"] = 5;
    } else {
        params["denoiseStrength"] = 20;
        params["morphKernelSize"] = 3;
    }
    
    // 过亮/过暗图像：调整二值化方法
    if (stats.brightness < 0.2 || stats.brightness > 0.8) {
        params["binaryMethod"] = "adaptive";
    } else {
        params["binaryMethod"] = "otsu";
    }
    
    return params;
}
```

---

## 5. 优化效果预期

| 优化项 | 预期提升 | 复杂度增加 |
|--------|----------|------------|
| 自适应 Canny 阈值 | 准确率 +10% | 低 |
| Gabor 方向滤波 | 误检率 -30% | 中 |
| 骨架化分析 | 裂纹长度精度 +50% | 中 |
| 多尺度形态学 | 异物召回率 +20% | 低 |
| LBP 纹理分析 | 纹理缺陷检出 +新功能 | 中 |
| 亚像素边缘 | 尺寸精度 +5x | 中 |
| YOLO 集成 | 整体准确率 +30% | 高 |
| 自适应参数 | 鲁棒性 +40% | 低 |

---

## 6. 实施优先级

### 高优先级（立即可做）
1. 自适应 Canny 阈值
2. 自适应参数调整
3. 亚像素边缘检测

### 中优先级（下一阶段）
1. Gabor 方向滤波
2. 骨架化与分支检测
3. 多尺度形态学
4. Blob 检测器

### 低优先级（长期）
1. YOLO/U-Net 深度学习集成
2. LBP 纹理分析
3. 在线学习系统
