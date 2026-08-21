#include "NetworkPersistence.h"

#include <unordered_set>

#include "Network.h"

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
