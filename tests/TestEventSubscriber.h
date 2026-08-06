#pragma once

#include <string>
#include <vector>

#include "EventDispatcher.h"
#include "EventSubscriber.h"

class RecordingEventSubscriber final : public EventSubscriber
{
  public:
    void onEvent(std::string_view message) override { mMessages.emplace_back(message); }
    const std::vector<std::string> getMessages() { return mMessages; }

  private:
    std::vector<std::string> mMessages;
};