#pragma once
#include <atomic>

#include "EventSubscriber.h"

class CallStatistics : public EventSubscriber
{
  public:
    void recordCreated() { ++created; }
    void recordStarted() { ++started; }
    void recordEnded() { ++ended; }

    int createdCount() const { return created.load(); }
    int startedCount() const { return started.load(); }
    int endedCount() const { return ended.load(); }
    void onEvent(EventType event) override
    {
        switch (event)
        {
        case EventType::CallCreated:
            recordCreated();
            break;

        case EventType::CallStarted:
            recordStarted();
            break;

        case EventType::CallEnded:
            recordEnded();
            break;

        case EventType::CallRestored:
            recordCreated();
            break;

        default:
            break;
        }
    }

  private:
    std::atomic<int> created{0};
    std::atomic<int> started{0};
    std::atomic<int> ended{0};
};
