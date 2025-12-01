#ifndef SINGLETON_H
#define SINGLETON_H

#include <mutex>
#include <memory>
#include <utility>

// ============================================================================
// Meyers' Singleton - 线程安全的延迟初始化单例
// ============================================================================

template <typename T>
class Singleton {
public:
    // 禁止拷贝和移动
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    Singleton(Singleton&&) = delete;
    Singleton& operator=(Singleton&&) = delete;

    // 获取单例实例 (线程安全)
    static T& instance() {
        static T inst;
        return inst;
    }

protected:
    Singleton() = default;
    ~Singleton() = default;
};

// ============================================================================
// 带参数构造的单例
// 第一次调用 instance(args...) 时构造，后续调用忽略参数
// ============================================================================

template <typename T>
class SingletonWithArgs {
public:
    SingletonWithArgs(const SingletonWithArgs&) = delete;
    SingletonWithArgs& operator=(const SingletonWithArgs&) = delete;

    // 初始化单例 (仅第一次调用有效)
    template <typename... Args>
    static T& init(Args&&... args) {
        std::call_once(s_initFlag, [&]() {
            s_instance.reset(new T(std::forward<Args>(args)...));
        });
        return *s_instance;
    }

    // 获取实例 (必须先调用 init)
    static T& instance() {
        if (!s_instance) {
            throw std::runtime_error("Singleton not initialized. Call init() first.");
        }
        return *s_instance;
    }

    // 检查是否已初始化
    static bool isInitialized() {
        return s_instance != nullptr;
    }

    // 销毁单例 (慎用，通常不需要)
    static void destroy() {
        s_instance.reset();
    }

protected:
    SingletonWithArgs() = default;
    ~SingletonWithArgs() = default;

private:
    static std::unique_ptr<T> s_instance;
    static std::once_flag s_initFlag;
};

template <typename T>
std::unique_ptr<T> SingletonWithArgs<T>::s_instance = nullptr;

template <typename T>
std::once_flag SingletonWithArgs<T>::s_initFlag;

// ============================================================================
// CRTP 单例基类 - 用于继承方式实现单例
// ============================================================================

template <typename Derived>
class SingletonCRTP {
public:
    SingletonCRTP(const SingletonCRTP&) = delete;
    SingletonCRTP& operator=(const SingletonCRTP&) = delete;

    static Derived& instance() {
        static Derived inst;
        return inst;
    }

protected:
    SingletonCRTP() = default;
    ~SingletonCRTP() = default;
};

// ============================================================================
// 宏定义 - 快速声明单例类
// ============================================================================

// 在类内使用，将构造函数设为私有并声明友元
#define DECLARE_SINGLETON(ClassName)            \
    friend class Singleton<ClassName>;          \
private:                                        \
    ClassName();                                \
    ~ClassName();                               \
    ClassName(const ClassName&) = delete;       \
    ClassName& operator=(const ClassName&) = delete;

// 定义单例访问函数
#define DEFINE_SINGLETON_INSTANCE(ClassName)    \
    ClassName& ClassName::instance() {          \
        static ClassName inst;                  \
        return inst;                            \
    }

#endif // SINGLETON_H
