#pragma once

#include <optional>
#include <variant>

#include "Call.h"
#include "User.h"

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

using DeserializeUserResult = std::variant<User, DeserializeUserError>;
using DeserializeCallResult = std::variant<Call, DeserializeCallError>;

std::string serializeUser(const User &user);
DeserializeUserResult deserializeUser(const std::string &text);

std::string serializeCall(const Call &call);
DeserializeCallResult deserializeCall(const std::string &text);

std::string callStatusToString(CallStatus status);
std::optional<CallStatus> callStatusFromString(const std::string &text);

std::string deserializeUserErrorToString(DeserializeUserError error);
std::string deserializeCallErrorToString(DeserializeCallError error);