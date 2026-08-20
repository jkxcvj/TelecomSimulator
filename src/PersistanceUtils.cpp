#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_set>
#include <variant>
#include <vector>

#include "Call.h"
#include "Network.h"
#include "PersistenceUtils.h"
#include "User.h"

std::string deserializeUserErrorToString(DeserializeUserError error)
{
    switch (error)
    {
    case DeserializeUserError::InvalidFieldCount:
        return "Invalid user field count";
    case DeserializeUserError::InvalidPrefix:
        return "Invalid user prefix";
    case DeserializeUserError::InvalidId:
        return "Invalid user ID";
    }
    return "Unknown user deserialization error";
}

std::string deserializeCallErrorToString(DeserializeCallError error)
{
    switch (error)
    {
    case DeserializeCallError::InvalidFieldCount:
        return "Invalid call field count";
    case DeserializeCallError::InvalidPrefix:
        return "Invalid call prefix";
    case DeserializeCallError::InvalidId:
        return "Invalid call ID";
    case DeserializeCallError::InvalidStatus:
        return "Invalid call status";
    }
    return "Unknown call deserialization error";
}

std::string loadNetworkErrorToString(LoadNetworkError error)
{
    switch (error)
    {
    case LoadNetworkError::FileOpenFailed:
        return "Failed to open network file";
    case LoadNetworkError::InvalidRecordType:
        return "Invalid network record type";
    case LoadNetworkError::InvalidUser:
        return "Invalid user record";
    case LoadNetworkError::InvalidCall:
        return "Invalid call record";
    case LoadNetworkError::DuplicateUser:
        return "Duplicate user";
    case LoadNetworkError::DuplicateCall:
        return "Duplicate call";
    case LoadNetworkError::MissingReferencedUser:
        return "Call references a missing user";
    case LoadNetworkError::FileReadError:
        return "Failed to read network file";
    case LoadNetworkError::AddUsersError:
        return "Failed to add users to the network";
    case LoadNetworkError::AddCallsError:
        return "Failed to add calls to the network";
    }
    return "Unknown network loading error";
}

std::string loadNetworkFailureToString(const LoadNetworkFailure &failure)
{
    std::string message = loadNetworkErrorToString(failure.error);

    if (failure.lineNumber != 0)
    {
        message += " at line " + std::to_string(failure.lineNumber);
    }

    if (failure.userError.has_value())
    {
        message += ": " + deserializeUserErrorToString(failure.userError.value());
    }

    if (failure.callError.has_value())
    {
        message += ": " + deserializeCallErrorToString(failure.callError.value());
    }

    return message;
}

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

DeserializeUserResult deserializeUser(const std::string &text)
{
    std::istringstream stream(text);

    std::string type;
    std::string id;
    std::string name;
    std::string phone;

    if (!std::getline(stream, type, '|') || !std::getline(stream, id, '|') || !std::getline(stream, name, '|') || !std::getline(stream, phone, '|'))
    {
        return DeserializeUserError::InvalidFieldCount;
    }

    std::string extra;

    if (std::getline(stream, extra, '|'))
    {
        return DeserializeUserError::InvalidFieldCount;
    }

    if (type != "USER")
    {
        return DeserializeUserError::InvalidPrefix;
    }

    try
    {
        const int intId = std::stoi(id);
        return User(UserId{intId}, name, phone);
    }
    catch (const std::exception &)
    {
        return DeserializeUserError::InvalidId;
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

DeserializeCallResult deserializeCall(const std::string &text)
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
        return DeserializeCallError::InvalidFieldCount;
    }

    std::string extra;

    if (std::getline(stream, extra, '|'))
    {
        return DeserializeCallError::InvalidFieldCount;
    }

    if (type != "CALL")
    {
        return DeserializeCallError::InvalidPrefix;
    }
    try
    {
        const int intId = std::stoi(id);
        const int intreceiverId = std::stoi(receiverId);
        const int intcallerId = std::stoi(callerId);
        const auto status = callStatusFromString(callstatus);

        if (!status.has_value())
        {
            return DeserializeCallError::InvalidStatus;
        }

        CallStatus callSTS = status.value();
        return Call(CallParameters{CallId{intId}, UserId{intcallerId}, UserId{intreceiverId}}, callSTS);
    }
    catch (const std::exception &)
    {
        return DeserializeCallError::InvalidId;
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

LoadNetworkResult loadNetwork(Network &network, const std::filesystem::path &path)
{
    std::ifstream file(path);

    std::vector<User> users;
    std::vector<Call> calls;
    std::size_t lineNumber = 0;
    if (file.is_open() == false)
    {
        return LoadNetworkFailure{LoadNetworkError::FileOpenFailed, 0, std::nullopt, std::nullopt};
    }
    std::string line;

    while (std::getline(file, line))
    {
        ++lineNumber;
        std::istringstream stream(line);
        std::string type;
        if (!std::getline(stream, type, '|'))
        {
            return LoadNetworkFailure{LoadNetworkError::InvalidRecordType, lineNumber, std::nullopt, std::nullopt};
        }
        if (type == "CALL")
        {
            auto result = deserializeCall(line);
            if (!std::holds_alternative<Call>(result))
            {
                return LoadNetworkFailure{LoadNetworkError::InvalidCall, lineNumber, std::nullopt, std::get<DeserializeCallError>(result)};
            }
            calls.push_back(std::get<Call>(result));
        }
        else if (type == "USER")
        {
            auto result = deserializeUser(line);
            if (!std::holds_alternative<User>(result))
            {
                return LoadNetworkFailure{LoadNetworkError::InvalidUser, lineNumber, std::get<DeserializeUserError>(result), std::nullopt};
            }
            users.push_back(std::get<User>(result));
        }
        else
        {
            return LoadNetworkFailure{LoadNetworkError::InvalidRecordType, lineNumber, std::nullopt, std::nullopt};
        }
    }

    if (file.bad())
    {
        return LoadNetworkFailure{LoadNetworkError::FileReadError, 0, std::nullopt, std::nullopt};
    }

    std::unordered_set<UserId, UserIdHash> userIds;
    std::unordered_set<CallId, CallIdHash> callIds;

    for (const auto &user : users)
    {
        const UserId id = user.getId();

        if ((network.getUser(id) != nullptr) || (userIds.insert(id).second == false))
        {
            return LoadNetworkFailure{LoadNetworkError::DuplicateUser, 0, std::nullopt, std::nullopt};
        }
    }

    for (const auto &call : calls)
    {
        const CallId id = call.getId();

        if ((network.getCall(id) != nullptr) || (callIds.insert(id).second == false))
        {
            return LoadNetworkFailure{LoadNetworkError::DuplicateCall, 0, std::nullopt, std::nullopt};
        }

        const UserId callerId = call.getCallerId();
        const UserId receiverId = call.getReceiverId();
        const bool callerExists = network.getUser(callerId) != nullptr || userIds.contains(callerId);
        const bool receiverExists = network.getUser(receiverId) != nullptr || userIds.contains(receiverId);
        if (!callerExists || !receiverExists)
        {
            return LoadNetworkFailure{LoadNetworkError::MissingReferencedUser, 0, std::nullopt, std::nullopt};
        }
    }

    for (const auto &user : users)
    {
        if (!network.addUser(user))
        {
            return LoadNetworkFailure{LoadNetworkError::AddUsersError, 0, std::nullopt, std::nullopt};
        }
    }

    for (const auto &call : calls)
    {
        if (!network.restoreCall(call))
        {
            return LoadNetworkFailure{LoadNetworkError::AddCallsError, 0, std::nullopt, std::nullopt};
        }
    }

    return std::monostate{};
}

std::string loadTextOrThrow(const std::filesystem::path &path)
{
    std::ifstream file(path);
    std::ostringstream buffer;
    if (!file.is_open())
    {
        throw std::runtime_error("File not working");
    }
    else
    {
        buffer << file.rdbuf();
    }
    if (file.bad())
    {
        throw std::runtime_error("Error during reading");
    }
    return buffer.str();
}

void handleSave(const Network &network, std::istream &input, std::ostream &output)
{
    output << "File path: ";

    std::string path;
    std::getline(input >> std::ws, path);

    if (saveNetwork(network, path))
    {
        output << "Network saved\n";
    }
    else
    {
        output << "Failed to save network\n";
    }
}

void handleLoad(Network &network, std::istream &input, std::ostream &output)
{
    output << "File path: ";

    std::string path;
    std::getline(input >> std::ws, path);

    const auto result = loadNetwork(network, path);

    if (std::holds_alternative<std::monostate>(result))
    {
        output << "Network loaded\n";
        return;
    }

    const auto &failure = std::get<LoadNetworkFailure>(result);

    output << "Failed to load network: " << loadNetworkFailureToString(failure) << '\n';
}