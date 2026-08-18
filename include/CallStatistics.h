#pragma once
#include <atomic>

class CallStatistics
{
  public:
    void recordCreated() { ++created; }
    void recordStarted() { ++started; }
    void recordEnded() { ++ended; }

    int createdCount() const { return created.load(); }
    int startedCount() const { return started.load(); }
    int endedCount() const { return ended.load(); }

  private:
    std::atomic<int> created{0};
    std::atomic<int> started{0};
    std::atomic<int> ended{0};
};