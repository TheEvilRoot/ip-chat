//
// Created by theevilroot on 2/26/22.
//

#ifndef J_TIME_WRAPPER_H
#define J_TIME_WRAPPER_H

#include <chrono>
#include <sstream>
#include <sstream>
#include <ctime>

struct Time {

    typedef std::chrono::system_clock Clock;
    typedef Clock::time_point TimePoint;

    const TimePoint point;
    const std::string str;

    explicit Time(): point(Clock::now()), str{Time::time(point)}
    {
    }

    static std::string time(Time::TimePoint point)
    {
        const auto t = Clock::to_time_t(point);
        char buffer[16]{};
        std::strftime(buffer, sizeof(buffer), "%H:%M", localtime(&t));
        return std::string{buffer};
    }
};

#endif //J_TIME_WRAPPER_H
