#pragma once

#include <Network.h>
#include <iosfwd>
#include <optional>

enum class MenuOption
{
    Exit = 0,
    AddUser = 1,
    RemoveUser = 2,
    CreateCall = 3,
    StartCall = 4,
    EndCall = 5,
    ListUsers = 6,
    ListCalls = 7,
    Save = 8,
    Load = 9
};

void printMenu(std::ostream &output);

std::optional<MenuOption> readMenuOption(std::istream &input);

void handleAddUser(Network &network, std::istream &input, std::ostream &output);

void handleRemoveUser(Network &network, std::istream &input, std::ostream &output);

void handleCreateCall(Network &network, std::istream &input, std::ostream &output);

void handleStartCall(Network &network, std::istream &input, std::ostream &output);

void handleEndCall(Network &network, std::istream &input, std::ostream &output);

void handleListUsers(const Network &network, std::ostream &output);

void handleListCalls(const Network &network, std::ostream &output);

void handleSave(const Network &network, std::istream &input, std::ostream &output);

void handleLoad(Network &network, std::istream &input, std::ostream &output);

std::optional<int> readInt(std::istream &input);