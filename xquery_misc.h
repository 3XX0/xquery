#pragma once

#include <memory>

struct NonCopyable
{
    NonCopyable() = default;
    ~NonCopyable() = default;
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
};

struct NonMoveable
{
    NonMoveable() = default;
    ~NonMoveable() = default;
    NonMoveable(NonMoveable&&) = delete;
    NonMoveable& operator=(NonMoveable&&) = delete;
};

std::string operator"" _red(const char* str, size_t n);
std::string operator"" _yellow(const char* str, size_t n);
std::string operator"" _green(const char* str, size_t n);
