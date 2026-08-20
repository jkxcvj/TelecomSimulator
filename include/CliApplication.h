#pragma once

#include <iosfwd>

#include "CliUtils.h"

class Network;

class CliApplication
{
  public:
    CliApplication(Network &network, std::istream &input, std::ostream &output);

    int run();

  private:
    void handleOption(MenuOption option);

    Network &mNetwork;
    std::istream &mInput;
    std::ostream &mOutput;
};