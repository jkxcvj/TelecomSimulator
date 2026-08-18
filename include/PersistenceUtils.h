#pragma once

#include <filesystem>
#include <fstream>
#include <string>
#include <unordered_set>

#include "Call.h"
#include "User.h"

bool ensureDirectoryExists(const std::filesystem::path &path)
{
    if (std::filesystem::exists(path))
    {
        return true;
    }
    return std::filesystem::create_directories(path);
}

bool saveText(const std::filesystem::path &path, const std::string &text)
{
    std::ofstream file(path);

    if (file.is_open() == false)
    {
        return false;
    }

    file << text;

    return file.good();
}

std::optional<std::string> loadText(const std::filesystem::path &path)
{
    std::ifstream file(path);

    if (file.is_open() == false)
    {
        return std::nullopt;
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();

    if (file.bad())
    {
        return std::nullopt;
    }

    return buffer.str();
}

std::string serializeUser(const User &user)
{
    std::ostringstream stream;
    stream << "USER|" << user.getId().value << "|" << user.getName() << "|" << user.getPhoneNumber();
    return stream.str();
}

std::optional<User> deserializeUser(const std::string &text)
{
    std::istringstream stream(text);

    std::string type;
    std::string id;
    std::string name;
    std::string phone;

    if (!std::getline(stream, type, '|') || !std::getline(stream, id, '|') || !std::getline(stream, name, '|') || !std::getline(stream, phone, '|'))
    {
        return std::nullopt;
    }

    std::string extra;

    if (std::getline(stream, extra, '|'))
    {
        return std::nullopt;
    }

    if (type != "USER")
    {
        return std::nullopt;
    }

    try
    {
        const int intId = std::stoi(id);
        return User(UserId{intId}, name, phone);
    }
    catch (const std::exception &)
    {
        return std::nullopt;
    }
}

std::string callStatusToString(CallStatus status)
{
    switch (status)
    {
    case CallStatus::Active:
        return "Active";
    case CallStatus::Created:
        return "Created";
    case CallStatus::Ended:
        return "Ended";
    }
    return "ERROR";
}

std::optional<CallStatus> callStatusFromString(const std::string &text)
{
    if (text == "Active")
    {
        return CallStatus::Active;
    }
    else if (text == "Created")
    {
        return CallStatus::Created;
    }
    else if (text == "Ended")
    {
        return CallStatus::Ended;
    }
    else
    {
        return std::nullopt;
    }
}

std::string serializeCall(const Call &call)
{
    std::ostringstream stream;
    stream << "CALL|" << call.getId().value << "|" << call.getCallerId().value << "|" << call.getReceiverId().value << "|"
           << callStatusToString(call.getStatusId());
    return stream.str();
}

std::optional<Call> deserializeCall(const std::string &text)
{
    std::istringstream stream(text);

    std::string type;
    std::string id;
    std::string callerId;
    std::string receiverId;
    std::string callstatus;

    if (!std::getline(stream, type, '|') || !std::getline(stream, id, '|') || !std::getline(stream, callerId, '|') ||
        !std::getline(stream, receiverId, '|') || !std::getline(stream, callstatus, '|'))
    {
        return std::nullopt;
    }

    std::string extra;

    if (std::getline(stream, extra, '|'))
    {
        return std::nullopt;
    }

    if (type != "CALL")
    {
        return std::nullopt;
    }
    try
    {
        const int intId = std::stoi(id);
        const int intreceiverId = std::stoi(receiverId);
        const int intcallerId = std::stoi(callerId);
        const auto status = callStatusFromString(callstatus);

        if (!status.has_value())
        {
            return std::nullopt;
        }

        CallStatus callSTS = status.value();
        return Call(CallParameters{CallId{intId}, UserId{intcallerId}, UserId{intreceiverId}}, callSTS);
    }
    catch (const std::exception &)
    {
        return std::nullopt;
    }
}

bool saveNetwork(const Network &network, const std::filesystem::path &path)
{
    std::ostringstream output;

    for (const UserId id : network.getUserIdsSortedByName())
    {
        const User *user = network.getUser(id);

        if (user == nullptr)
        {
            return false;
        }

        output << serializeUser(*user) << '\n';
    }

    for (const CallId id : network.getCallIds())
    {
        const Call *call = network.getCall(id);

        if (call == nullptr)
        {
            return false;
        }

        output << serializeCall(*call) << '\n';
    }

    return saveText(path, output.str());
}

bool loadNetwork(Network &network, const std::filesystem::path &path)
{
    std::ifstream file(path);

    std::vector<User> users;
    std::vector<Call> calls;

    if (file.is_open() == false)
    {
        return false;
    }
    std::string line;

    while (std::getline(file, line))
    {
        std::istringstream stream(line);
        std::string type;
        if (!std::getline(stream, type, '|'))
        {
            return false;
        }
        if (type == "CALL")
        {
            auto call = deserializeCall(line);
            if (!call.has_value())
            {
                return false;
            }
            calls.push_back(call.value());
        }
        else if (type == "USER")
        {
            auto user = deserializeUser(line);
            if (!user.has_value())
            {
                return false;
            }
            users.push_back(user.value());
        }
        else
        {
            return false;
        }
    }
    if (file.bad())
    {
        return false;
    }
    std::unordered_set<UserId, UserIdHash> userIds;
    std::unordered_set<CallId, CallIdHash> callIds;

    for (const auto &user : users)
    {
        const UserId id = user.getId();

        if (network.getUser(id) != nullptr)
        {
            return false;
        }

        if (userIds.insert(id).second == false)
        {
            return false;
        }
    }
    for (const auto &call : calls)
    {
        const CallId id = call.getId();

        if (network.getCall(id) != nullptr)
        {
            return false;
        }

        if (callIds.insert(id).second == false)
        {
            return false;
        }
        const UserId callerId = call.getCallerId();
        const UserId receiverId = call.getReceiverId();
        const bool callerExists = network.getUser(callerId) != nullptr || userIds.contains(callerId);
        const bool receiverExists = network.getUser(receiverId) != nullptr || userIds.contains(receiverId);
        if (!callerExists || !receiverExists)
        {
            return false;
        }
    }
    for (const auto &user : users)
    {
        if (!network.addUser(user))
        {
            return false;
        }
    }

    for (const auto &call : calls)
    {
        if (!network.restoreCall(call))
        {
            return false;
        }
    }

    return true;
}