#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <unordered_map>

namespace redis_clone
{

class Store
{
public:
    void set(std::string key, std::string value);

    [[nodiscard]] std::optional<std::string> get(const std::string& key) const;

    [[nodiscard]] bool erase(const std::string& key);

    [[nodiscard]] bool contains(const std::string& key) const;

    [[nodiscard]] std::size_t size() const noexcept;

    [[nodiscard]] bool empty() const noexcept;

private:
    std::unordered_map<std::string, std::string> entries_;
};

} // namespace redis_clone
