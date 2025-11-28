#ifndef TIMER_H
#define TIMER_H

#include <chrono>

class Timer {
public:
  Timer();

  void start();
  void stop();
  void reset();

  // 返回当前/上一次计时的毫秒
  double elapsedMs() const;

private:
  using Clock = std::chrono::steady_clock;
  Clock::time_point m_start;
  Clock::time_point m_end;
  bool m_running = false;
};

#endif // TIMER_H
