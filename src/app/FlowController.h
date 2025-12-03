/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * FlowController.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：流程控制器接口定义
 * 描述：检测流程总控制器，管理自动检测循环、与PLC交互、
 *       异常处理和恢复等
 *
 * 当前版本：1.0
 */

#ifndef FLOWCONTROLLER_H
#define FLOWCONTROLLER_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <memory>

class DetectPipeline;
class IPLCClient;
struct DetectResult;

class FlowController : public QObject {
    Q_OBJECT
public:
    enum class State {
        Idle,           // 空闲 - 未初始化
        Initializing,   // 初始化中
        Ready,          // 就绪 - 可以开始
        Running,        // 运行中
        Paused,         // 暂停
        Error,          // 错误
        Recovering      // 恢复中
    };
    Q_ENUM(State)

    enum class TriggerMode {
        Timer,          // 定时触发
        Software,       // 软件触发
        External        // 外部 PLC 触发
    };
    Q_ENUM(TriggerMode)

    explicit FlowController(QObject* parent = nullptr);
    ~FlowController();

    // 状态查询
    State state() const { return m_state; }
    QString stateString() const;
    bool isRunning() const { return m_state == State::Running; }
    bool isReady() const { return m_state == State::Ready; }
    bool isError() const { return m_state == State::Error; }

    // 配置
    void setPipeline(DetectPipeline* pipeline);
    void setPLCClient(IPLCClient* plc);
    void setTriggerMode(TriggerMode mode);
    void setTriggerInterval(int ms);
    void setPLCAddresses(int triggerAddr, int resultAddr, int heartbeatAddr);

    TriggerMode triggerMode() const { return m_triggerMode; }
    int triggerInterval() const { return m_triggerIntervalMs; }

    // 统计信息
    struct Statistics {
        int totalCount = 0;
        int okCount = 0;
        int ngCount = 0;
        double yield = 100.0;       // 良率 %
        double avgCycleTime = 0.0;  // 平均节拍 ms
        qint64 startTime = 0;
        qint64 runningTime = 0;     // 运行时间 ms
    };
    const Statistics& statistics() const { return m_stats; }
    void resetStatistics();

public slots:
    void initialize();
    void start();
    void stop();
    void pause();
    void resume();
    void reset();
    void triggerOnce();

signals:
    void stateChanged(FlowController::State newState, FlowController::State oldState);
    void initialized();
    void started();
    void stopped();
    void paused();
    void resumed();
    void error(const QString& module, const QString& message);
    void resultReady(const DetectResult& result);
    void statisticsUpdated(const Statistics& stats);

private slots:
    void onPipelineResult(const DetectResult& result);
    void onPipelineError(const QString& module, const QString& message);
    void onPipelineStarted();
    void onPipelineStopped();
    void onPLCPollTimeout();
    void onHeartbeatTimeout();

private:
    void setState(State newState);
    bool checkTransition(State from, State to) const;
    void connectPipeline();
    void disconnectPipeline();
    void startPLCPolling();
    void stopPLCPolling();
    void writePLCResult(const DetectResult& result);
    void updateStatistics(const DetectResult& result);

    State m_state = State::Idle;
    DetectPipeline* m_pipeline = nullptr;
    IPLCClient* m_plc = nullptr;

    TriggerMode m_triggerMode = TriggerMode::Timer;
    int m_triggerIntervalMs = 500;

    // PLC 寄存器地址
    int m_plcTriggerAddr = 40001;
    int m_plcResultAddr = 40002;
    int m_plcHeartbeatAddr = 40005;
    bool m_lastTriggerState = false;

    // 定时器
    QTimer* m_plcPollTimer = nullptr;
    QTimer* m_heartbeatTimer = nullptr;

    // 统计
    Statistics m_stats;
    double m_cycleTimeSum = 0.0;

    QString m_lastErrorModule;
    QString m_lastErrorMessage;
};

#endif // FLOWCONTROLLER_H
