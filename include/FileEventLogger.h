#pragma once

#include <fstream>
#include <string>

#include "EventLogger.h"

class FileEventLogger final : public EventLogger
{
  public:
    FileEventLogger(const std::string &path);
    void log(std::string_view message) override;

  private:
    std::ofstream mFile;
};