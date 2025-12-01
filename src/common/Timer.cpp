#include "Timer.h"
#include "Logger.h"
#include <sstream>
#include <iomanip>
#include <limits>

// ============================================================================
// Timer
// ============================================================================

Timer::Timer() {
    reset();
}

void Timer::start() {
    m_start = Clock::now();
    m_running = true;
}

void Timer::stop() {
    m_end = Clock::now();
    m_running = false;
}

void Timer::reset() {
    m_start = Clock::now();
    m_end = m_start;
    m_running = false;
}

void Timer::restart() {
    reset();
    start();
}

Timer::Duration Timer::elapsed() const {
    const auto endPoint = m_running ? Clock::now() : m_end;
    return endPoint - m_start;
}

double Timer::elapsedNs() const {
    return std::chrono::duration<double, std::nano>(elapsed()).count();
}

double Timer::elapsedUs() const {
    return std::chrono::duration<double, std::micro>(elapsed()).count();
}

double Timer::elapsedMs() const {
    return std::chrono::duration<double, std::milli>(elapsed()).count();
}

double Timer::elapsedSec() const {
    return std::chrono::duration<double>(elapsed()).count();
}

// ============================================================================
// ScopedTimer
// ============================================================================

ScopedTimer::ScopedTimer(Callback callback)
    : m_callback(std::move(callback))
    , m_logOnDestroy(false) {
    m_timer.start();
}

ScopedTimer::ScopedTimer(const std::string& name, bool logOnDestroy)
    : m_name(name)
    , m_logOnDestroy(logOnDestroy) {
    m_timer.start();
}

ScopedTimer::~ScopedTimer() {
    m_timer.stop();
    double ms = m_timer.elapsedMs();

    if (m_callback) {
        m_callback(ms);
    }

    if (m_logOnDestroy && !m_name.empty()) {
        LOG_DEBUG("[{}] elapsed: {:.3f} ms", m_name, ms);
    }
}

double ScopedTimer::elapsedMs() const {
    return m_timer.elapsedMs();
}

// ============================================================================
// PerfStats
// ============================================================================

PerfStats::PerfStats(const std::string& name)
    : m_name(name) {
    reset();
}

void PerfStats::record(double ms) {
    if (m_count == 0) {
        m_min = ms;
        m_max = ms;
    } else {
        if (ms < m_min) m_min = ms;
        if (ms > m_max) m_max = ms;
    }
    m_total += ms;
    m_last = ms;
    ++m_count;
}

void PerfStats::reset() {
    m_count = 0;
    m_total = 0.0;
    m_min = std::numeric_limits<double>::max();
    m_max = 0.0;
    m_last = 0.0;
}

std::string PerfStats::summary() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(3);
    if (!m_name.empty()) {
        oss << "[" << m_name << "] ";
    }
    oss << "count=" << m_count
        << ", avg=" << avg() << "ms"
        << ", min=" << (m_count > 0 ? m_min : 0.0) << "ms"
        << ", max=" << m_max << "ms"
        << ", total=" << m_total << "ms";
    return oss.str();
}

// ============================================================================
// FPSCounter
// ============================================================================

FPSCounter::FPSCounter(double smoothingFactor)
    : m_smoothingFactor(smoothingFactor) {
    m_timer.start();
}

void FPSCounter::tick() {
    ++m_frameCount;

    if (m_firstFrame) {
        m_timer.restart();
        m_firstFrame = false;
        return;
    }

    double elapsed = m_timer.elapsedMs();
    if (elapsed > 0) {
        double instantFps = 1000.0 / elapsed;
        // 指数移动平均
        m_fps = m_smoothingFactor * m_fps + (1.0 - m_smoothingFactor) * instantFps;
    }

    m_timer.restart();
}

void FPSCounter::reset() {
    m_fps = 0.0;
    m_frameCount = 0;
    m_firstFrame = true;
    m_timer.restart();
}

// ============================================================================
// Throttle
// ============================================================================

Throttle::Throttle(double intervalMs)
    : m_intervalMs(intervalMs) {
    m_timer.start();
}

bool Throttle::check() {
    if (m_first) {
        m_first = false;
        m_timer.restart();
        return true;
    }

    if (m_timer.elapsedMs() >= m_intervalMs) {
        m_timer.restart();
        return true;
    }

    return false;
}

void Throttle::reset() {
    m_first = true;
    m_timer.restart();
}
