#pragma once

#include <cstddef>
#include <functional>
#include <unordered_map>
#include <utility>

template <typename Id, typename Value, std::size_t MaxSize, typename Hash = std::hash<Id>>
class LimitedRegistry
{
  public:
    bool add(Id id, Value value)
    {
        if (mMap.size() >= MaxSize)
        {
            return false;
        }

        return mMap.emplace(std::move(id), std::move(value)).second;
    }

    bool contains(Id id) const { return mMap.contains(id); }

    std::size_t size() const { return mMap.size(); }

  private:
    std::unordered_map<Id, Value, Hash> mMap;
};
