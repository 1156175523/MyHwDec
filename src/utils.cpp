#include "utils.h"

std::string THREAD_ID()
{
    std::string thdId;
#ifdef _WIN32
    //std::ostringstream os;
    //os << std::this_thread::get_id();
    //thdId = os.str();
    thdId = std::to_string((unsigned long)GetCurrentThreadId());
#else
    thdId = std::to_string((unsigned long)syscall(__NR_gettid));
#endif
    return thdId;
}

std::string TIME_NOW()
{
    std::string retStr;
    auto duration_since_epoch = std::chrono::system_clock::now().time_since_epoch();
    auto microseconds_since_epoch = std::chrono::duration_cast<std::chrono::microseconds>(duration_since_epoch).count();
    time_t seconds_since_epoch = static_cast<time_t>(microseconds_since_epoch / 1000000);
    std::tm current_time;
#ifdef _WIN32
    localtime_s(&current_time, &seconds_since_epoch);
    auto tm_microsec = microseconds_since_epoch % 1000000;
    char time_str[30] = {0};
    snprintf(time_str, sizeof(time_str), "%04d-%02d-%02d %02d:%02d:%02d.%06ld",
             current_time.tm_year + 1900, current_time.tm_mon + 1, current_time.tm_mday,
             current_time.tm_hour, current_time.tm_min, current_time.tm_sec, (long)tm_microsec);
    retStr = time_str;
#else 
    localtime_r(&seconds_since_epoch, &current_time);
    auto tm_microsec = microseconds_since_epoch % 1000000;
    char time_str[30] = {0};
    snprintf(time_str, sizeof(time_str), "%04d-%02d-%02d %02d:%02d:%02d.%06ld",
             current_time.tm_year + 1900, current_time.tm_mon + 1, current_time.tm_mday,
             current_time.tm_hour, current_time.tm_min, current_time.tm_sec, (long)tm_microsec);
    retStr = time_str;
#endif
    return retStr;
}