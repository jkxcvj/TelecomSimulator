#pragma once

#include "EventLogger.h"

class NullEventLogger final : public EventLogger
{
  public:
    void log(std::string_view) override {}
};