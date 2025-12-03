/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * SPSCQueue.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：单生产者单消费者无锁队列
 * 描述：高性能无锁队列模板类，适用于单线程生产、单线程消费场景，
 *       用于线程间高效数据传递
 *
 * 当前版本：1.0
 */

#ifndef SPSCQUEUE_H
#define SPSCQUEUE_H

#include "common_global.h"
#include <atomic>
#include <cstddef>
#include <memory>
#include <new>
#include <type_traits>
#include <optional>
#include <stdexcept>

// ============================================================================
// 无锁单生产者单消费者队列 (Lock-Free SPSC Queue)
// 基于环形缓冲区实现，适用于一个线程写入、另一个线程读取的场景
// ============================================================================

template <typename T>
class SPSCQueue {
public:
    explicit SPSCQueue(size_t capacity);
    ~SPSCQueue();

    // 禁止拷贝和移动
    SPSCQueue(const SPSCQueue&) = delete;
    SPSCQueue& operator=(const SPSCQueue&) = delete;
    SPSCQueue(SPSCQueue&&) = delete;
    SPSCQueue& operator=(SPSCQueue&&) = delete;

    // 生产者接口 (只能从一个线程调用)
    bool push(const T& value);
    bool push(T&& value);

    template <typename... Args>
    bool emplace(Args&&... args);

    // 消费者接口 (只能从另一个线程调用)
    bool pop(T& value);
    std::optional<T> pop();
    bool tryPop(T& value);

    // 查看队首元素但不移除
    const T* front() const;

    // 状态查询 (可从任意线程调用)
    bool empty() const;
    bool full() const;
    size_t size() const;
    size_t capacity() const { return m_capacity; }

    // 清空队列 (只能在没有并发访问时调用)
    void clear();

private:
    static constexpr size_t CacheLineSize = 64;

    // 对齐到缓存行，避免伪共享
    alignas(CacheLineSize) std::atomic<size_t> m_writeIndex{0};
    alignas(CacheLineSize) std::atomic<size_t> m_readIndex{0};

    T* m_buffer;
    size_t m_capacity;

    size_t nextIndex(size_t index) const {
        return (index + 1) % (m_capacity + 1);
    }
};

// ============================================================================
// 实现
// ============================================================================

template <typename T>
SPSCQueue<T>::SPSCQueue(size_t capacity)
    : m_capacity(capacity) {
    // 分配 capacity + 1 个槽位，用于区分满和空
    m_buffer = static_cast<T*>(::operator new((m_capacity + 1) * sizeof(T)));
}

template <typename T>
SPSCQueue<T>::~SPSCQueue() {
    // 销毁所有剩余元素
    clear();
    ::operator delete(m_buffer);
}

template <typename T>
bool SPSCQueue<T>::push(const T& value) {
    return emplace(value);
}

template <typename T>
bool SPSCQueue<T>::push(T&& value) {
    return emplace(std::move(value));
}

template <typename T>
template <typename... Args>
bool SPSCQueue<T>::emplace(Args&&... args) {
    const size_t writeIdx = m_writeIndex.load(std::memory_order_relaxed);
    const size_t nextWriteIdx = nextIndex(writeIdx);

    // 队列满
    if (nextWriteIdx == m_readIndex.load(std::memory_order_acquire)) {
        return false;
    }

    // 原地构造
    new (&m_buffer[writeIdx]) T(std::forward<Args>(args)...);

    // 发布写入
    m_writeIndex.store(nextWriteIdx, std::memory_order_release);
    return true;
}

template <typename T>
bool SPSCQueue<T>::pop(T& value) {
    const size_t readIdx = m_readIndex.load(std::memory_order_relaxed);

    // 队列空
    if (readIdx == m_writeIndex.load(std::memory_order_acquire)) {
        return false;
    }

    value = std::move(m_buffer[readIdx]);
    m_buffer[readIdx].~T();

    m_readIndex.store(nextIndex(readIdx), std::memory_order_release);
    return true;
}

template <typename T>
std::optional<T> SPSCQueue<T>::pop() {
    T value;
    if (pop(value)) {
        return std::move(value);
    }
    return std::nullopt;
}

template <typename T>
bool SPSCQueue<T>::tryPop(T& value) {
    return pop(value);
}

template <typename T>
const T* SPSCQueue<T>::front() const {
    const size_t readIdx = m_readIndex.load(std::memory_order_relaxed);
    if (readIdx == m_writeIndex.load(std::memory_order_acquire)) {
        return nullptr;
    }
    return &m_buffer[readIdx];
}

template <typename T>
bool SPSCQueue<T>::empty() const {
    return m_readIndex.load(std::memory_order_acquire) ==
           m_writeIndex.load(std::memory_order_acquire);
}

template <typename T>
bool SPSCQueue<T>::full() const {
    return nextIndex(m_writeIndex.load(std::memory_order_acquire)) ==
           m_readIndex.load(std::memory_order_acquire);
}

template <typename T>
size_t SPSCQueue<T>::size() const {
    const size_t writeIdx = m_writeIndex.load(std::memory_order_acquire);
    const size_t readIdx = m_readIndex.load(std::memory_order_acquire);

    if (writeIdx >= readIdx) {
        return writeIdx - readIdx;
    }
    return (m_capacity + 1) - readIdx + writeIdx;
}

template <typename T>
void SPSCQueue<T>::clear() {
    T value;
    while (pop(value)) {
        // 持续弹出直到队列为空
    }
}

// ============================================================================
// 阻塞式 SPSC 队列 (带条件变量)
// ============================================================================

#include <condition_variable>
#include <mutex>

template <typename T>
class BlockingSPSCQueue {
public:
    explicit BlockingSPSCQueue(size_t capacity);

    // 生产者接口
    void push(const T& value);
    void push(T&& value);
    bool tryPush(const T& value);

    // 消费者接口
    T pop();
    bool tryPop(T& value);
    bool tryPopFor(T& value, std::chrono::milliseconds timeout);

    // 停止队列 (解除所有阻塞)
    void stop();
    bool isStopped() const { return m_stopped.load(); }

    size_t size() const { return m_queue.size(); }
    bool empty() const { return m_queue.empty(); }
    size_t capacity() const { return m_queue.capacity(); }

private:
    SPSCQueue<T> m_queue;
    mutable std::mutex m_mutex;
    std::condition_variable m_cvNotEmpty;
    std::condition_variable m_cvNotFull;
    std::atomic<bool> m_stopped{false};
};

template <typename T>
BlockingSPSCQueue<T>::BlockingSPSCQueue(size_t capacity)
    : m_queue(capacity) {}

template <typename T>
void BlockingSPSCQueue<T>::push(const T& value) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_cvNotFull.wait(lock, [this] {
        return m_stopped.load() || !m_queue.full();
    });

    if (m_stopped.load()) return;

    m_queue.push(value);
    m_cvNotEmpty.notify_one();
}

template <typename T>
void BlockingSPSCQueue<T>::push(T&& value) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_cvNotFull.wait(lock, [this] {
        return m_stopped.load() || !m_queue.full();
    });

    if (m_stopped.load()) return;

    m_queue.push(std::move(value));
    m_cvNotEmpty.notify_one();
}

template <typename T>
bool BlockingSPSCQueue<T>::tryPush(const T& value) {
    if (m_queue.push(value)) {
        m_cvNotEmpty.notify_one();
        return true;
    }
    return false;
}

template <typename T>
T BlockingSPSCQueue<T>::pop() {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_cvNotEmpty.wait(lock, [this] {
        return m_stopped.load() || !m_queue.empty();
    });

    if (m_stopped.load() && m_queue.empty()) {
        throw std::runtime_error("Queue stopped");
    }

    auto result = m_queue.pop();
    m_cvNotFull.notify_one();
    return result.value();
}

template <typename T>
bool BlockingSPSCQueue<T>::tryPop(T& value) {
    if (m_queue.pop(value)) {
        m_cvNotFull.notify_one();
        return true;
    }
    return false;
}

template <typename T>
bool BlockingSPSCQueue<T>::tryPopFor(T& value, std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> lock(m_mutex);
    if (!m_cvNotEmpty.wait_for(lock, timeout, [this] {
            return m_stopped.load() || !m_queue.empty();
        })) {
        return false;
    }

    if (m_stopped.load() && m_queue.empty()) {
        return false;
    }

    if (m_queue.pop(value)) {
        m_cvNotFull.notify_one();
        return true;
    }
    return false;
}

template <typename T>
void BlockingSPSCQueue<T>::stop() {
    m_stopped.store(true);
    m_cvNotEmpty.notify_all();
    m_cvNotFull.notify_all();
}

#endif // SPSCQUEUE_H
