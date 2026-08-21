#pragma once

#include <vector>

#include "EventDispatcher.h"
#include "EventSubscriber.h"

class RecordingEventSubscriber final : public EventSubscriber
{
  public:
    void onEvent(EventType event) override { mEvents.emplace_back(event); }
    const std::vector<EventType> &getMessages() const { return mEvents; }

  private:
    std::vector<EventType> mEvents;
};
