#include "storage/store.h"

#include <utility>

namespace redis_clone
{

void Store::set(std::string key, std::string value)
{
    entries_.insert_or_assign(std::move(key), std::move(value));
}

std::optional<std::string> Store::get(const std::string& key) const
{
    const auto it = entries_.find(key);
    if (it == entries_.end())
    {
        return std::nullopt;
    }

    return it->second;
}

bool Store::erase(const std::string& key)
{
    return entries_.erase(key) > 0;
}

bool Store::contains(const std::string& key) const
{
    return entries_.find(key) != entries_.end();
}

std::size_t Store::size() const noexcept
{
    return entries_.size();
}

bool Store::empty() const noexcept
{
    return entries_.empty();
}

} // namespace redis_clone
