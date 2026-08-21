#pragma once
#include <string_view>

#include "EventSubscriber.h"

class EventLogger : public EventSubscriber
{
  public:
    virtual ~EventLogger() = default;
    void onEvent(EventType event) override { log(toString(event)); }
    virtual void log(std::string_view message) = 0;
};
