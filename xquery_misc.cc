#include "xquery_misc.h"

std::string operator"" _red(const char* str, size_t n)
{
    return "\033[1;31m" + std::string(str, n) + "\033[0m";
}

std::string operator"" _yellow(const char* str, size_t n)
{
    return "\033[1;33m" + std::string(str, n) + "\033[0m";
}

std::string operator"" _green(const char* str, size_t n)
{
    return "\033[1;32m" + std::string(str, n) + "\033[0m";
}

