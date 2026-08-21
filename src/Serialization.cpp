#include "Serialization.h"

#include <sstream>

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
