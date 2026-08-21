#include "FileUtils.h"

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
