#pragma once

#include <cstddef>
#include <functional>

struct UserId
{
    int value;

    bool operator==(const UserId &other) const = default;

    bool operator<(const UserId &other) const noexcept { return value < other.value; }
};

struct UserIdHash
{
    std::size_t operator()(UserId id) const noexcept { return std::hash<int>{}(id.value); }
};

struct CallId
{
    int value;

    bool operator==(const CallId &other) const = default;

    bool operator<(const CallId &other) const noexcept { return value < other.value; }
};

struct CallIdHash
{
    std::size_t operator()(CallId id) const noexcept { return std::hash<int>{}(id.value); }
};