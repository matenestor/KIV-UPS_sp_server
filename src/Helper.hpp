#ifndef HELPER_HPP
#define HELPER_HPP

#include <ctime>
#include <iomanip>
#include <string>

inline std::string getDateTime() {
    std::stringstream str;

    // get time
    std::time_t t = std::time(nullptr);
    std::tm tm = *std::localtime(&t);

    // year-month-day_hours-minutes-seconds
    str << std::put_time(&tm, "[%d.%m.%y %H:%M:%S] ");

    return str.str();
}

#endif
