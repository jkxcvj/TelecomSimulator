#include "CliUtils.h"

#include <iostream>
#include <limits>
#include <variant>

#include "PersistenceUtils.h"

void printMenu(std::ostream &output)
{
    output << "================================\n"
           << "      TELECOM SIMULATOR\n"
           << "================================\n"
           << "Pick option:\n"
           << "1. AddUser\n"
           << "2. RemoveUser\n"
           << "3. CreateCall\n"
           << "4. StartCall\n"
           << "5. EndCall\n"
           << "6. ListUsers\n"
           << "7. ListCalls\n"
           << "8. Save\n"
           << "9. Load\n"
           << "0. Exit\n"
           << "> ";
}

std::optional<MenuOption> readMenuOption(std::istream &input)
{
    const auto choice = readInt(input);
    if (!choice)
    {
        return std::nullopt;
    }

    switch (*choice)
    {
    case 0:
        return MenuOption::Exit;
    case 1:
        return MenuOption::AddUser;
    case 2:
        return MenuOption::RemoveUser;
    case 3:
        return MenuOption::CreateCall;
    case 4:
        return MenuOption::StartCall;
    case 5:
        return MenuOption::EndCall;
    case 6:
        return MenuOption::ListUsers;
    case 7:
        return MenuOption::ListCalls;
    case 8:
        return MenuOption::Save;
    case 9:
        return MenuOption::Load;
    default:
        return std::nullopt;
    }
}

void handleAddUser(Network &network, std::istream &input, std::ostream &output)
{
    std::string name;
    std::string phoneNumber;
    output << "Please enter UserId\n";
    const auto id = readInt(input);
    if (!id)
    {
        output << "Invalid UserId\n";
        return;
    }
    output << "Please enter user name\n";
    std::getline(input, name);
    output << "Please enter phonenumber\n";
    std::getline(input, phoneNumber);
    User user(UserId{*id}, name, phoneNumber);
    if (network.addUser(user))
    {
        output << "User added\n";
    }
    else
    {
        output << "User already exists\n";
    }
}

void handleRemoveUser(Network &network, std::istream &input, std::ostream &output)
{
    output << "Please enter UserId to remove:\n";
    const auto id = readInt(input);
    if (!id)
    {
        output << "Invalid UserId\n";
        return;
    }
    if (network.removeUser(UserId{*id}))
    {
        output << "User removed\n";
    }
    else
    {
        output << "User not found\n";
    }
}

void handleCreateCall(Network &network, std::istream &input, std::ostream &output)
{
    output << "Please enter CallId\n";
    const auto id = readInt(input);
    if (!id)
    {
        output << "Invalid CallId\n";
        return;
    }
    output << "Please enter CallerID\n";
    const auto callerId = readInt(input);
    if (!callerId)
    {
        output << "Invalid CallerID\n";
        return;
    }
    output << "Please enter ReceiverID\n";
    const auto receiverId = readInt(input);
    if (!receiverId)
    {
        output << "Invalid ReceiverID\n";
        return;
    }
    const auto call = network.createCall(CallId{*id}, UserId{*callerId}, UserId{*receiverId});
    if (std::holds_alternative<std::monostate>(call))
    {
        output << "Call created\n";
        return;
    }
    const auto error = std::get<CreateCallError>(call);
    switch (error)
    {
    case CreateCallError::CallerNotFound:
        output << "Caller not found\n";
        break;

    case CreateCallError::ReceiverNotFound:
        output << "Receiver not found\n";
        break;

    case CreateCallError::CallerBusy:
        output << "Caller is busy\n";
        break;

    case CreateCallError::ReceiverBusy:
        output << "Receiver is busy\n";
        break;

    case CreateCallError::SameUser:
        output << "Caller and receiver cannot be the same user\n";
        break;

    case CreateCallError::CallAlreadyExists:
        output << "Call already exists\n";
        break;
    }
}

void handleStartCall(Network &network, std::istream &input, std::ostream &output)
{
    output << "Please enter CallId\n";
    const auto id = readInt(input);
    if (!id)
    {
        output << "Invalid CallId\n";
        return;
    }
    const auto call = network.startCall(CallId{*id});
    if (std::holds_alternative<std::monostate>(call))
    {
        output << "Call started\n";
        return;
    }
    const auto error = std::get<StartCallError>(call);
    switch (error)
    {
    case StartCallError::CallNotFound:
        output << "Call not found\n";
        break;

    case StartCallError::CallAlreadyStarted:
        output << "Call already started\n";
        break;

    case StartCallError::CallAlreadyEnded:
        output << "Call already ended\n";
        break;

    case StartCallError::UserBusy:
        output << "User is busy\n";
        break;
    }
}

void handleEndCall(Network &network, std::istream &input, std::ostream &output)
{
    output << "Please enter CallId\n";
    const auto id = readInt(input);
    if (!id)
    {
        output << "Invalid CallId\n";
        return;
    }
    const auto call = network.endCall(CallId{*id});
    if (call)
    {
        output << "Call ended\n";
    }
    output << "Could not end call\n";
}

void handleListUsers(const Network &network, std::ostream &output)
{
    const auto users = network.getUsers();

    if (users.empty())
    {
        output << "No users\n";
        return;
    }

    output << "Users:\n";

    for (const auto &userRef : users)
    {
        const User &user = userRef.get();

        output << user.getId().value << " | " << user.getName() << " | " << user.getPhoneNumber() << '\n';
    }
}

void handleListCalls(const Network &network, std::ostream &output)
{
    const auto calls = network.getCalls();

    if (calls.empty())
    {
        output << "No calls\n";
        return;
    }

    output << "Calls:\n";

    for (const auto &callRef : calls)
    {
        const Call &call = callRef.get();

        output << call.getId().value << " | caller=" << call.getCallerId().value << " | receiver=" << call.getReceiverId().value << " | "
               << callStatusToString(call.getStatusId()) << '\n';
    }
}

std::optional<int> readInt(std::istream &input)
{
    int value;

    if (!(input >> value))
    {
        input.clear();
        input.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        return std::nullopt;
    }

    input.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    return value;
}
