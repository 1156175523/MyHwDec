#ifndef __ACL_UTILS_H__
#define __ACL_UTILS_H__

#include <chrono>
#include <ctime>
#include <ostream>
#include <sstream>
#include <string>
#include <cstring>
#ifdef _WIN32
#include <windows.h>
#else
#include <libgen.h>
#include <unistd.h>
#include <sys/syscall.h>
#endif

// 获取当前时间
std::string TIME_NOW();

// 获取线程id
std::string THREAD_ID();

// 获取文件名不带路径
#ifdef _WIN32
#define __B_FILENAME__ (strrchr(__FILE__, '\\') ? (strrchr(__FILE__, '\\') + 1):__FILE__)
#else
#define __B_FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1):__FILE__)
#endif

//日志输出
#define PREFIX "[%s] -%s- [%s][%s|%d]"   //带时间戳
#define MLOG_INFO(fmt, ...) fprintf(stdout, PREFIX "[INFO]  " fmt "\n", TIME_NOW().c_str(), THREAD_ID().c_str(), __B_FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define MLOG_WARN(fmt, ...) fprintf(stdout, PREFIX "[WARN]  " fmt "\n", TIME_NOW().c_str(), THREAD_ID().c_str(), __B_FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define MLOG_ERROR(fmt, ...) fprintf(stdout, PREFIX "[ERROR] " fmt "\n", TIME_NOW().c_str(), THREAD_ID().c_str(), __B_FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#endif