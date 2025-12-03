#include "FlowController.h"
#include "DetectPipeline.h"
#include "plc/IPLCClient.h"
#include "Types.h"
#include "common/Logger.h"
#include <QDateTime>
#include <QDebug>

FlowController::FlowController(QObject* parent)
    : QObject(parent)
    , m_plcPollTimer(new QTimer(this))
    , m_heartbeatTimer(new QTimer(this))
{
    m_plcPollTimer->setInterval(50);  // 50ms PLC 轮询
    connect(m_plcPollTimer, &QTimer::timeout, this, &FlowController::onPLCPollTimeout);

    m_heartbeatTimer->setInterval(1000);  // 1s 心跳
    connect(m_heartbeatTimer, &QTimer::timeout, this, &FlowController::onHeartbeatTimeout);
}

FlowController::~FlowController()
{
    stop();
}

QString FlowController::stateString() const
{
    switch (m_state) {
    case State::Idle:        return tr("空闲");
    case State::Initializing: return tr("初始化中");
    case State::Ready:       return tr("就绪");
    case State::Running:     return tr("运行中");
    case State::Paused:      return tr("暂停");
    case State::Error:       return tr("错误");
    case State::Recovering:  return tr("恢复中");
    default:                 return tr("未知");
    }
}

void FlowController::setPipeline(DetectPipeline* pipeline)
{
    if (m_pipeline) {
        disconnectPipeline();
    }
    m_pipeline = pipeline;
    if (m_pipeline) {
        connectPipeline();
    }
}

void FlowController::setPLCClient(IPLCClient* plc)
{
    m_plc = plc;
}

void FlowController::setTriggerMode(TriggerMode mode)
{
    if (m_state == State::Running) {
        qWarning() << "Cannot change trigger mode while running";
        return;
    }
    m_triggerMode = mode;
}

void FlowController::setTriggerInterval(int ms)
{
    m_triggerIntervalMs = qMax(50, ms);
    if (m_pipeline) {
        m_pipeline->setCaptureInterval(m_triggerIntervalMs);
    }
}

void FlowController::setPLCAddresses(int triggerAddr, int resultAddr, int heartbeatAddr)
{
    m_plcTriggerAddr = triggerAddr;
    m_plcResultAddr = resultAddr;
    m_plcHeartbeatAddr = heartbeatAddr;
}

void FlowController::resetStatistics()
{
    m_stats = Statistics();
    m_cycleTimeSum = 0.0;
    emit statisticsUpdated(m_stats);
}

void FlowController::initialize()
{
    if (m_state != State::Idle && m_state != State::Error) {
        LOG_WARN("FlowController: Cannot initialize from state: {}", stateString().toStdString());
        return;
    }

    LOG_INFO("FlowController: Initializing...");
    setState(State::Initializing);

    bool success = true;
    QString errorMsg;

    // 检查 Pipeline
    if (!m_pipeline) {
        success = false;
        errorMsg = tr("检测管道未设置");
    }

    // 如果是外部触发模式，检查 PLC
    if (success && m_triggerMode == TriggerMode::External && !m_plc) {
        success = false;
        errorMsg = tr("PLC 未连接，无法使用外部触发模式");
    }

    if (success) {
        LOG_INFO("FlowController: Initialized successfully");
        setState(State::Ready);
        emit initialized();
    } else {
        LOG_ERROR("FlowController: Initialize failed - {}", errorMsg.toStdString());
        m_lastErrorModule = "FlowController";
        m_lastErrorMessage = errorMsg;
        setState(State::Error);
        emit error("FlowController", errorMsg);
    }
}

void FlowController::start()
{
    if (!checkTransition(m_state, State::Running)) {
        LOG_WARN("FlowController: Cannot start from state: {}", stateString().toStdString());
        return;
    }
    
    LOG_INFO("FlowController: Starting (triggerMode={})", static_cast<int>(m_triggerMode));

    if (!m_pipeline) {
        emit error("FlowController", tr("检测管道未设置"));
        setState(State::Error);
        return;
    }

    // 重置统计
    if (m_state == State::Ready) {
        resetStatistics();
        m_stats.startTime = QDateTime::currentMSecsSinceEpoch();
    }

    setState(State::Running);

    // 根据触发模式启动
    switch (m_triggerMode) {
    case TriggerMode::Timer:
        m_pipeline->setCaptureInterval(m_triggerIntervalMs);
        m_pipeline->start();
        break;

    case TriggerMode::Software:
        // 软件触发模式，等待 triggerOnce() 调用
        break;

    case TriggerMode::External:
        // 外部触发模式，启动 PLC 轮询
        startPLCPolling();
        break;
    }

    // 启动心跳
    if (m_plc) {
        m_heartbeatTimer->start();
    }

    emit started();
}

void FlowController::stop()
{
    if (m_state != State::Running && m_state != State::Paused) {
        return;
    }

    LOG_INFO("FlowController: Stopping...");
    
    if (m_pipeline && m_pipeline->isRunning()) {
        m_pipeline->stop();
    }

    stopPLCPolling();
    m_heartbeatTimer->stop();

    // 更新运行时间
    if (m_stats.startTime > 0) {
        m_stats.runningTime = QDateTime::currentMSecsSinceEpoch() - m_stats.startTime;
    }

    setState(State::Ready);
    emit stopped();
}

void FlowController::pause()
{
    if (m_state != State::Running) {
        return;
    }

    if (m_pipeline && m_pipeline->isRunning()) {
        m_pipeline->stop();
    }

    stopPLCPolling();

    setState(State::Paused);
    emit paused();
}

void FlowController::resume()
{
    if (m_state != State::Paused) {
        return;
    }

    // 恢复运行
    start();
    emit resumed();
}

void FlowController::reset()
{
    stop();
    m_lastErrorModule.clear();
    m_lastErrorMessage.clear();
    setState(State::Idle);
}

void FlowController::triggerOnce()
{
    if (m_state == State::Ready) {
        // 从就绪状态单次触发
        if (m_pipeline) {
            m_pipeline->singleShot();
        }
    } else if (m_state == State::Running && m_triggerMode == TriggerMode::Software) {
        // 软件触发模式下的单次触发
        if (m_pipeline) {
            m_pipeline->singleShot();
        }
    } else {
        qWarning() << "Cannot trigger from state:" << stateString();
    }
}

void FlowController::setState(State newState)
{
    if (m_state == newState) {
        return;
    }
    State oldState = m_state;
    m_state = newState;
    emit stateChanged(newState, oldState);
}

bool FlowController::checkTransition(State from, State to) const
{
    // 状态转换规则
    switch (from) {
    case State::Idle:
        return to == State::Initializing;
    case State::Initializing:
        return to == State::Ready || to == State::Error;
    case State::Ready:
        return to == State::Running || to == State::Idle;
    case State::Running:
        return to == State::Paused || to == State::Ready || to == State::Error;
    case State::Paused:
        return to == State::Running || to == State::Ready;
    case State::Error:
        return to == State::Idle || to == State::Recovering;
    case State::Recovering:
        return to == State::Ready || to == State::Error;
    default:
        return false;
    }
}

void FlowController::connectPipeline()
{
    if (!m_pipeline) return;

    connect(m_pipeline, &DetectPipeline::resultReady,
            this, &FlowController::onPipelineResult);
    connect(m_pipeline, &DetectPipeline::error,
            this, &FlowController::onPipelineError);
    connect(m_pipeline, &DetectPipeline::started,
            this, &FlowController::onPipelineStarted);
    connect(m_pipeline, &DetectPipeline::stopped,
            this, &FlowController::onPipelineStopped);
}

void FlowController::disconnectPipeline()
{
    if (!m_pipeline) return;

    disconnect(m_pipeline, &DetectPipeline::resultReady,
               this, &FlowController::onPipelineResult);
    disconnect(m_pipeline, &DetectPipeline::error,
               this, &FlowController::onPipelineError);
    disconnect(m_pipeline, &DetectPipeline::started,
               this, &FlowController::onPipelineStarted);
    disconnect(m_pipeline, &DetectPipeline::stopped,
               this, &FlowController::onPipelineStopped);
}

void FlowController::startPLCPolling()
{
    if (m_plc && m_triggerMode == TriggerMode::External) {
        m_lastTriggerState = false;
        m_plcPollTimer->start();
    }
}

void FlowController::stopPLCPolling()
{
    m_plcPollTimer->stop();
}

void FlowController::writePLCResult(const DetectResult& result)
{
    if (!m_plc) return;

    // 写入结果寄存器: 1 = OK, 2 = NG
    uint16_t resultValue = result.isOK ? 1 : 2;
    m_plc->writeHoldingRegister(m_plcResultAddr, resultValue);

    // 可扩展: 写入缺陷类型、严重度等
}

void FlowController::updateStatistics(const DetectResult& result)
{
    m_stats.totalCount++;

    if (result.isOK) {
        m_stats.okCount++;
    } else {
        m_stats.ngCount++;
    }

    // 计算良率
    if (m_stats.totalCount > 0) {
        m_stats.yield = 100.0 * m_stats.okCount / m_stats.totalCount;
    }

    // 计算平均节拍
    m_cycleTimeSum += result.cycleTimeMs;
    m_stats.avgCycleTime = m_cycleTimeSum / m_stats.totalCount;

    // 更新运行时间
    if (m_stats.startTime > 0) {
        m_stats.runningTime = QDateTime::currentMSecsSinceEpoch() - m_stats.startTime;
    }

    emit statisticsUpdated(m_stats);
}

void FlowController::onPipelineResult(const DetectResult& result)
{
    // 更新统计
    updateStatistics(result);

    // 写入 PLC
    if (m_plc) {
        writePLCResult(result);
    }

    // 转发结果
    emit resultReady(result);
}

void FlowController::onPipelineError(const QString& module, const QString& message)
{
    LOG_ERROR("FlowController: Pipeline error - module={}, message={}", 
              module.toStdString(), message.toStdString());
    
    m_lastErrorModule = module;
    m_lastErrorMessage = message;

    // 停止运行
    if (m_pipeline && m_pipeline->isRunning()) {
        m_pipeline->stop();
    }
    stopPLCPolling();
    m_heartbeatTimer->stop();

    setState(State::Error);
    emit error(module, message);
}

void FlowController::onPipelineStarted()
{
    LOG_DEBUG("FlowController: Pipeline started");
}

void FlowController::onPipelineStopped()
{
    LOG_DEBUG("FlowController: Pipeline stopped, stats: total={}, ok={}, ng={}, yield={:.1f}%",
              m_stats.totalCount, m_stats.okCount, m_stats.ngCount, m_stats.yield);
}

void FlowController::onPLCPollTimeout()
{
    if (!m_plc || m_state != State::Running) {
        return;
    }

    // 读取触发信号
    std::vector<uint16_t> data;
    if (!m_plc->readHoldingRegisters(m_plcTriggerAddr, 1, data)) {
        qWarning() << "Failed to read PLC trigger";
        return;
    }

    bool currentTrigger = (data[0] != 0);

    // 上升沿检测
    if (currentTrigger && !m_lastTriggerState) {
        if (m_pipeline) {
            m_pipeline->singleShot();
        }
    }

    m_lastTriggerState = currentTrigger;
}

void FlowController::onHeartbeatTimeout()
{
    if (!m_plc) {
        return;
    }

    // 发送心跳
    if (!m_plc->heartbeat()) {
        qWarning() << "PLC heartbeat failed";
        // 可以在这里触发重连逻辑
    }
}
