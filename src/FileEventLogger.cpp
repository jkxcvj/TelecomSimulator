#include "FileEventLogger.h"

FileEventLogger::FileEventLogger(const std::string &path) : mFile(path)
{
    if (!mFile.is_open())
    {
        throw std::runtime_error("FILE NOT OPENED");
    }
};
void FileEventLogger::log(std::string_view message) { mFile << "[EVENT] " << message << '\n'; }