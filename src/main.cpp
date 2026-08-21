#include <iostream>
#include <memory>

#include "CallStatistics.h"
#include "CliApplication.h"
#include "Network.h"
#include "NullEventLogger.h"

int main()
{
    auto statistics = std::make_shared<CallStatistics>();

    auto logger = std::make_shared<NullEventLogger>();

    Network network;
    network.subscribe(logger);
    network.subscribe(statistics);

    CliApplication app(network, std::cin, std::cout);

    return app.run();
}
