#pragma once

#include "runtime/core/log/log_system.h"  // 日志系统实现
#include "runtime/function/global/global_context.h"  // 全局上下文访问

#include <chrono>   // 时间库
#include <thread>   // 线程库
#include <cassert>  // 断言支持（虽然条件编译中使用，但需要显式包含）

/////////////////////////////
// 日志记录宏系统
/////////////////////////////

// 日志辅助宏（实际执行日志记录）
// 参数:
//   LOG_LEVEL - 日志级别
//   ... - 可变参数，日志内容
// 功能:
//   1. 添加当前函数名作为日志前缀
//   2. 调用全局日志系统记录日志
#define LOG_HELPER(LOG_LEVEL, ...) \
    g_runtime_global_context.m_logger_system->log(LOG_LEVEL, "[" + std::string(__FUNCTION__) + "] " + __VA_ARGS__);

// 各级别日志记录宏
#define LOG_DEBUG(...) LOG_HELPER(LogSystem::LogLevel::debug, __VA_ARGS__);  // 调试信息
#define LOG_INFO(...) LOG_HELPER(LogSystem::LogLevel::info, __VA_ARGS__);    // 一般信息
#define LOG_WARN(...) LOG_HELPER(LogSystem::LogLevel::warn, __VA_ARGS__);    // 警告信息
#define LOG_ERROR(...) LOG_HELPER(LogSystem::LogLevel::error, __VA_ARGS__);  // 错误信息
#define LOG_FATAL(...) LOG_HELPER(LogSystem::LogLevel::fatal, __VA_ARGS__);  // 致命错误

/////////////////////////////
// 实用工具宏
/////////////////////////////

// 线程休眠宏（单位：毫秒）
// 示例: PolitSleep(100) 休眠100ms
#define SammiSleep(_ms) std::this_thread::sleep_for(std::chrono::milliseconds(_ms));

// 变量名转字符串宏
// 示例: 
//   int myVar;
//   std::cout << PolitNameOf(myVar);  // 输出 "myVar"
#define SammiNameOf(name) #name

/////////////////////////////
// 断言调试系统
/////////////////////////////

// 条件编译的断言宏
// 在发布版本中(定义NDEBUG时)禁用断言
// 在调试版本中启用标准断言
#ifdef NDEBUG
#define ASSERT(statement)
#else
#define ASSERT(statement) assert(statement)
#endif