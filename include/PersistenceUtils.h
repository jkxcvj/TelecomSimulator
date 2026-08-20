#pragma once

#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <variant>

#include "Call.h"
#include "User.h"

class Network;

enum class LoadNetworkError
{
    FileOpenFailed,
    InvalidRecordType,
    InvalidUser,
    InvalidCall,
    DuplicateUser,
    DuplicateCall,
    MissingReferencedUser,
    FileReadError,
    AddUsersError,
    AddCallsError
};

enum class DeserializeUserError
{
    InvalidFieldCount,
    InvalidPrefix,
    InvalidId
};

enum class DeserializeCallError
{
    InvalidFieldCount,
    InvalidPrefix,
    InvalidId,
    InvalidStatus
};

struct LoadNetworkFailure
{
    LoadNetworkError error;
    std::size_t lineNumber{0};

    std::optional<DeserializeUserError> userError;
    std::optional<DeserializeCallError> callError;
};

using DeserializeUserResult = std::variant<User, DeserializeUserError>;
using DeserializeCallResult = std::variant<Call, DeserializeCallError>;
using LoadNetworkResult = std::variant<std::monostate, LoadNetworkFailure>;

std::string deserializeUserErrorToString(DeserializeUserError error);
std::string deserializeCallErrorToString(DeserializeCallError error);
std::string loadNetworkErrorToString(LoadNetworkError error);
std::string loadNetworkFailureToString(const LoadNetworkFailure &failure);

bool ensureDirectoryExists(const std::filesystem::path &path);
bool saveText(const std::filesystem::path &path, const std::string &text);
std::optional<std::string> loadText(const std::filesystem::path &path);

std::string serializeUser(const User &user);
DeserializeUserResult deserializeUser(const std::string &text);

std::string callStatusToString(CallStatus status);
std::optional<CallStatus> callStatusFromString(const std::string &text);

std::string serializeCall(const Call &call);
DeserializeCallResult deserializeCall(const std::string &text);

bool saveNetwork(const Network &network, const std::filesystem::path &path);
LoadNetworkResult loadNetwork(Network &network, const std::filesystem::path &path);

std::string loadTextOrThrow(const std::filesystem::path &path);
