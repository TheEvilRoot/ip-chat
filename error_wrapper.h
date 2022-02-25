//
// Created by theevilroot on 2/22/22.
//

#ifndef J_ERROR_WRAPPER_H
#define J_ERROR_WRAPPER_H

#include <cerrno>
#include <cstring>
#include <string>
#include <utility>

struct Error
{
    errno_t code;
    std::string place;
    std::string description{strerror(code)};

    Error(): code{0}, place{}
    {
    }

    explicit Error(std::string place): code{errno}, place{std::move(place)}
    {
    }

    explicit Error(std::string place, std::string description): code{errno}, place{std::move(place)}, description{std::move(description)}
    {
    }

    operator bool() const noexcept
    {
        return code != 0;
    }
};

#endif //J_ERROR_WRAPPER_H
