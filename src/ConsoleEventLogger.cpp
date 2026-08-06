#include "ConsoleEventLogger.h"

#include <iostream>

void ConsoleEventLogger::log(std::string_view message) { std::cout << "[EVENT] " << message; }