#pragma once

#include "EventLogger.h"

class ConsoleEventLogger final : public EventLogger
{
  public:
    void log(std::string_view message) override;
};