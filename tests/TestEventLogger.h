#pragma once

#include "EventLogger.h"
#include "Network.h"

class SilentEventLogger final : public EventLogger
{
  public:
    void log(std::string_view) override {}
};

inline Network makeNetwork() { return Network(std::make_unique<SilentEventLogger>()); }

class RecordingEventLogger final : public EventLogger
{
  public:
    void log(std::string_view message) override { mMessages.emplace_back(message); }
    const std::vector<std::string> &getMessages() const { return mMessages; }

  private:
    std::vector<std::string> mMessages;
};