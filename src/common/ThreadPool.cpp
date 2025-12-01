#include "ThreadPool.h"
#include "Logger.h"
#include <algorithm>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

namespace {

void setThreadName([[maybe_unused]] const std::string& name) {
#if defined(__linux__)
    pthread_setname_np(pthread_self(), name.substr(0, 15).c_str());
#elif defined(__APPLE__)
    pthread_setname_np(name.c_str());
#endif
    // Windows: SetThreadDescription 在 MinGW 中不可用，跳过
}

} // namespace

// ============================================================================
// ThreadPool
// ============================================================================

ThreadPool::ThreadPool(size_t threadCount, const std::string& name)
    : m_name(name)
    , m_desiredThreadCount(threadCount) {
    if (m_desiredThreadCount == 0) {
        m_desiredThreadCount = std::max<size_t>(1, std::thread::hardware_concurrency());
    }
}

ThreadPool::~ThreadPool() {
    stop();
}

void ThreadPool::start() {
    if (m_running.exchange(true)) {
        return;  // 已经在运行
    }

    LOG_INFO("{}: starting with {} threads", m_name, m_desiredThreadCount);

    m_workers.reserve(m_desiredThreadCount);
    for (size_t i = 0; i < m_desiredThreadCount; ++i) {
        m_workers.emplace_back(&ThreadPool::workerLoop, this, i);
    }
}

void ThreadPool::stop() {
    if (!m_running.exchange(false)) {
        return;  // 已经停止
    }

    LOG_INFO("{}: stopping...", m_name);

    m_cv.notify_all();

    for (auto& t : m_workers) {
        if (t.joinable()) {
            t.join();
        }
    }
    m_workers.clear();

    // 清空剩余任务
    size_t dropped = 0;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        dropped = m_tasks.size();
        while (!m_tasks.empty()) {
            m_tasks.pop();
        }
    }

    if (dropped > 0) {
        LOG_WARN("{}: dropped {} pending tasks", m_name, dropped);
    }

    LOG_INFO("{}: stopped. completed={}, failed={}",
             m_name, m_completedTasks.load(), m_failedTasks.load());
}

void ThreadPool::waitAll() {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_cvComplete.wait(lock, [this] {
        return m_tasks.empty() && m_activeCount.load() == 0;
    });
}

void ThreadPool::enqueue(std::function<void()> task) {
    enqueue(Priority::Normal, std::move(task));
}

void ThreadPool::enqueue(Priority priority, std::function<void()> task) {
    if (!m_running.load()) {
        LOG_WARN("{}: cannot enqueue, pool not running", m_name);
        return;
    }

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_tasks.push(Task{std::move(task), priority});
        ++m_totalTasks;
    }

    m_cv.notify_one();
}

size_t ThreadPool::pendingTasks() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_tasks.size();
}

ThreadPool::Stats ThreadPool::stats() const {
    Stats s;
    s.totalTasks = m_totalTasks.load();
    s.completedTasks = m_completedTasks.load();
    s.failedTasks = m_failedTasks.load();
    s.activeThreads = m_activeCount.load();
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        s.pendingTasks = m_tasks.size();
    }
    return s;
}

void ThreadPool::setExceptionHandler(ExceptionHandler handler) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_exceptionHandler = std::move(handler);
}

void ThreadPool::workerLoop(size_t workerId) {
    std::ostringstream oss;
    oss << m_name << "-" << workerId;
    std::string threadName = oss.str();
    setThreadName(threadName);

    LOG_DEBUG("{}: worker started", threadName);

    while (m_running.load()) {
        Task task;

        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cv.wait(lock, [this] {
                return !m_running.load() || !m_tasks.empty();
            });

            if (!m_running.load() && m_tasks.empty()) {
                break;
            }

            if (m_tasks.empty()) {
                continue;
            }

            task = std::move(const_cast<Task&>(m_tasks.top()));
            m_tasks.pop();
        }

        if (task.func) {
            ++m_activeCount;

            try {
                task.func();
                ++m_completedTasks;
            } catch (const std::exception& e) {
                ++m_failedTasks;
                LOG_ERROR("{}: task exception: {}", threadName, e.what());

                if (m_exceptionHandler) {
                    try {
                        m_exceptionHandler(e, threadName);
                    } catch (...) {
                        // 忽略异常处理器的异常
                    }
                }
            } catch (...) {
                ++m_failedTasks;
                LOG_ERROR("{}: task threw unknown exception", threadName);
            }

            --m_activeCount;

            // 通知等待者
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                if (m_tasks.empty() && m_activeCount.load() == 0) {
                    m_cvComplete.notify_all();
                }
            }
        }
    }

    LOG_DEBUG("{}: worker exiting", threadName);
}

// ============================================================================
// 全局线程池单例
// ============================================================================

ThreadPool& globalThreadPool() {
    static ThreadPool pool(0, "GlobalPool");
    static std::once_flag initFlag;
    std::call_once(initFlag, [] {
        pool.start();
    });
    return pool;
}
