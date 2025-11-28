#include "Timer.h"

Timer::Timer() { reset(); }

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

double Timer::elapsedMs() const {
  const auto endPoint = m_running ? Clock::now() : m_end;
  const auto duration = std::chrono::duration<double, std::milli>(endPoint - m_start);
  return duration.count();
}
