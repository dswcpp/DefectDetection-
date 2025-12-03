/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * Timer.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：计时器模块接口定义
 * 描述：高精度计时器类，用于性能测量，支持启动/停止/重置，返回毫秒级耗时
 *
 * 当前版本：1.0
 */

#ifndef TIMER_H
#define TIMER_H

#include "common_global.h"
#include <chrono>
#include <string>
#include <functional>
#include <atomic>

// ============================================================================
// 高精度计时器
// ============================================================================

class COMMON_LIBRARY Timer {
public:
    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;
    using Duration = Clock::duration;

    Timer();

    void start();
    void stop();
    void reset();
    void restart();

    bool isRunning() const { return m_running; }

    // 获取耗时
    double elapsedNs() const;   // 纳秒
    double elapsedUs() const;   // 微秒
    double elapsedMs() const;   // 毫秒
    double elapsedSec() const;  // 秒

    // 获取原始 duration
    Duration elapsed() const;

private:
    TimePoint m_start;
    TimePoint m_end;
    bool m_running = false;
};

// ============================================================================
// 作用域计时器 - 自动记录代码块执行时间
// ============================================================================

class COMMON_LIBRARY ScopedTimer {
public:
    using Callback = std::function<void(double ms)>;

    // 构造时开始计时，析构时调用回调
    explicit ScopedTimer(Callback callback);
    explicit ScopedTimer(const std::string& name, bool logOnDestroy = true);
    ~ScopedTimer();

    // 禁止拷贝
    ScopedTimer(const ScopedTimer&) = delete;
    ScopedTimer& operator=(const ScopedTimer&) = delete;

    double elapsedMs() const;

private:
    Timer m_timer;
    Callback m_callback;
    std::string m_name;
    bool m_logOnDestroy;
};

// ============================================================================
// 性能统计器 - 统计多次调用的耗时
// ============================================================================

class COMMON_LIBRARY PerfStats {
public:
    explicit PerfStats(const std::string& name = "");

    void record(double ms);
    void reset();

    std::string name() const { return m_name; }
    size_t count() const { return m_count; }
    double total() const { return m_total; }
    double min() const { return m_min; }
    double max() const { return m_max; }
    double avg() const { return m_count > 0 ? m_total / m_count : 0.0; }
    double last() const { return m_last; }

    std::string summary() const;

private:
    std::string m_name;
    size_t m_count = 0;
    double m_total = 0.0;
    double m_min = 0.0;
    double m_max = 0.0;
    double m_last = 0.0;
};

// ============================================================================
// 帧率计算器
// ============================================================================

class COMMON_LIBRARY FPSCounter {
public:
    explicit FPSCounter(double smoothingFactor = 0.9);

    void tick();
    void reset();

    double fps() const { return m_fps; }
    double avgFrameTimeMs() const { return m_fps > 0 ? 1000.0 / m_fps : 0.0; }
    size_t frameCount() const { return m_frameCount; }

private:
    Timer m_timer;
    double m_fps = 0.0;
    double m_smoothingFactor;
    size_t m_frameCount = 0;
    bool m_firstFrame = true;
};

// ============================================================================
// 节流器 - 限制操作频率
// ============================================================================

class COMMON_LIBRARY Throttle {
public:
    explicit Throttle(double intervalMs);

    // 检查是否可以执行，如果可以则更新时间戳
    bool check();

    // 重置
    void reset();

    void setInterval(double intervalMs) { m_intervalMs = intervalMs; }
    double interval() const { return m_intervalMs; }

private:
    Timer m_timer;
    double m_intervalMs;
    bool m_first = true;
};

// ============================================================================
// 便捷宏
// ============================================================================

#define SCOPED_TIMER(name) ScopedTimer _scopedTimer##__LINE__(name)
#define SCOPED_TIMER_CB(callback) ScopedTimer _scopedTimer##__LINE__(callback)

#endif // TIMER_H
