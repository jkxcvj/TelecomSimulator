#pragma once

#include <filesystem>
#include <fstream>
#include <optional>
#include <string>

#include "Serialization.h"

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

struct LoadNetworkFailure
{
    LoadNetworkError error;
    std::size_t lineNumber{0};

    std::optional<DeserializeUserError> userError;
    std::optional<DeserializeCallError> callError;
};

using LoadNetworkResult = std::variant<std::monostate, LoadNetworkFailure>;

bool ensureDirectoryExists(const std::filesystem::path &path);

bool saveText(const std::filesystem::path &path, const std::string &text);

std::optional<std::string> loadText(const std::filesystem::path &path);

std::string loadTextOrThrow(const std::filesystem::path &path);

std::string loadNetworkErrorToString(LoadNetworkError error);
std::string loadNetworkFailureToString(const LoadNetworkFailure &failure);