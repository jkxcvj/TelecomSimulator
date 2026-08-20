#include <iostream>
#include <memory>

#include "CliApplication.h"
#include "Network.h"
#include "NullEventLogger.h"

int main()
{
    CallStatistics statistics;

    auto logger = std::make_unique<NullEventLogger>();

    Network network(std::move(logger), statistics);

    CliApplication app(network, std::cin, std::cout);

    return app.run();
}