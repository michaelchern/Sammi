#pragma once

#include <spdlog/spdlog.h>  // spdlog核心功能
#include <cstdint>          // 固定大小整数类型
#include <stdexcept>        // 标准异常类

namespace Sammi
{
    // 日志系统类（不可继承）
    class LogSystem final
    {
    public:
        // 日志级别枚举（显式指定uint8_t底层类型）
        enum class LogLevel : uint8_t
        {
            debug,  // 调试信息（最低级别）
            info,   // 一般信息
            warn,   // 警告信息
            error,  // 错误信息
            fatal   // 致命错误（最高级别）
        };

    public:
        LogSystem();   // 构造函数
        ~LogSystem();  // 析构函数

        // 泛型日志记录方法（支持任意数量和类型的参数）
        // 参数：
        //   level - 日志级别
        //   args... - 日志内容参数（支持格式化字符串和变量）
        template<typename... TARGS>
        void log(LogLevel level, TARGS&&... args)
        {
            switch (level)
            {
                case LogLevel::debug:
                    m_logger->debug(std::forward<TARGS>(args)...);  // 转发参数到spdlog
                    break;
                case LogLevel::info:
                    m_logger->info(std::forward<TARGS>(args)...);
                    break;
                case LogLevel::warn:
                    m_logger->warn(std::forward<TARGS>(args)...);
                    break;
                case LogLevel::error:
                    m_logger->error(std::forward<TARGS>(args)...);
                    break;
                case LogLevel::fatal:
                    m_logger->critical(std::forward<TARGS>(args)...);  // spdlog的critical级别
                    fatalCallback(std::forward<TARGS>(args)...);       // 执行致命错误回调
                    break;
                default:
                    break;  // 未知级别忽略（理论上不会执行）
            }
        }

        // 致命错误回调方法（抛出异常）
        // 参数：
        //   args... - 错误描述参数（支持格式化）
        template<typename... TARGS>
        void fatalCallback(TARGS&&... args)
        {
            // 1. 格式化错误信息
            const std::string format_str = fmt::format(std::forward<TARGS>(args)...);

            // 2. 抛出运行时异常（程序通常在此终止）
            throw std::runtime_error(format_str);
        }

    private:
        std::shared_ptr<spdlog::logger> m_logger;  // spdlog日志记录器实例
    };
}