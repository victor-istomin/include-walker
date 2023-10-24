#pragma once
#include <stdexcept>
#include <string>

struct Error : std::runtime_error
{
    Error(const std::string& message) : std::runtime_error(message) {}
    Error(const Error&) = default;
    Error& operator=(const Error&) = default;
    Error(Error&&) = default;
    Error& operator=(Error&&) = default;

    template <typename... Args>
        requires (std::constructible_from<std::string, Args> && ...)
    Error(Args&&... parts) : std::runtime_error((std::string(std::forward<Args>(parts)) + ...)) {}
};

