//
// Created by theevilroot on 2/26/22.
//

#ifndef J_TIME_WRAPPER_H
#define J_TIME_WRAPPER_H

#include <chrono>
#include <sstream>

struct Time {
    const std::chrono::system_clock::time_point point;
    const std::string str;

    explicit Time(): point(std::chrono::system_clock::now()), str{"time"}
    {
    }
};

#endif //J_TIME_WRAPPER_H
