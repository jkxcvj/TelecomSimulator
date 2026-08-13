#pragma once

#include <cstddef>
#include <functional>
#include <unordered_map>

template <typename Id, typename Value, typename Hash = std::hash<Id>> class Registry
{
  public:
    bool add(Id id, Value value) { return mMap.emplace(id, value).second; }

    Value *find(Id id)
    {
        auto it = mMap.find(id);

        if (it == mMap.end())
        {
            return nullptr;
        }

        return &it->second;
    }

    const Value *find(Id id) const
    {
        auto it = mMap.find(id);

        if (it == mMap.end())
        {
            return nullptr;
        }

        return &it->second;
    }

    bool contains(Id id) const { return mMap.contains(id); }

    std::size_t size() const { return mMap.size(); }

  private:
    std::unordered_map<Id, Value, Hash> mMap;
};