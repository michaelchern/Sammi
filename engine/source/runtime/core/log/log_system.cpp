// 包含自定义日志系统头文件
#include "runtime/core/log/log_system.h"

// 包含spdlog异步日志库相关组件
#include <spdlog/async.h>                     // 异步日志支持
#include <spdlog/sinks/basic_file_sink.h>     // 文件输出（未使用）
#include <spdlog/sinks/stdout_color_sinks.h>  // 彩色控制台输出
#include <spdlog/spdlog.h>                    // spdlog核心功能

namespace Sammi
{
    // 日志系统构造函数
    LogSystem::LogSystem()
    {
        // 创建彩色控制台输出器（多线程安全版本）
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::trace);  // 设置接收所有级别日志
        console_sink->set_pattern("[%^%l%$] %v");       // 日志格式：彩色级别 + 消息内容

        // 准备接收器列表（目前只包含控制台输出器）
        const spdlog::sinks_init_list sink_list = {console_sink};

        // 初始化异步线程池（8192个消息/队列，1个工作线程）
        spdlog::init_thread_pool(8192, 1);

        // 创建异步日志记录器
        m_logger = std::make_shared<spdlog::async_logger>("sammi_logger",                         // 记录器名称
                                                          sink_list.begin(),                      // 接收器列表起始
                                                          sink_list.end(),                        // 接收器列表结束
                                                          spdlog::thread_pool(),                  // 使用全局线程池
                                                          spdlog::async_overflow_policy::block);  // 队列满时阻塞策略
        m_logger->set_level(spdlog::level::trace);  // 记录所有级别日志

        // 注册日志记录器到全局注册表
        spdlog::register_logger(m_logger);
    }

    // 日志系统析构函数
    LogSystem::~LogSystem()
    {
        // 确保所有日志写入完成
        m_logger->flush();   // 刷新日志缓存
        spdlog::drop_all();  // 卸载所有记录器（关闭日志系统）
    }
}