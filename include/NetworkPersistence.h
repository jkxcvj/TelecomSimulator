#pragma once

#include <filesystem>
#include <fstream>

#include "FileUtils.h"

class Network;

bool saveNetwork(const Network &network, const std::filesystem::path &path);

LoadNetworkResult loadNetwork(Network &network, const std::filesystem::path &path);

void handleSave(const Network &network, std::istream &input, std::ostream &output);
void handleLoad(Network &network, std::istream &input, std::ostream &output);