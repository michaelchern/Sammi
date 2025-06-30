#pragma once

#include "common/precompiled.h"  // 预编译头文件（包含基础库）

// 模板管理器类（单例模式）
// 负责：
//   - 加载和管理Mustache模板文件
//   - 提供模板渲染服务
class TemplateManager
{
public:
    // 获取单例实例（线程不安全）
    static TemplateManager* getInstance()
    {
        static TemplateManager* m_pInstance;  // 静态局部变量保证唯一性
        if (nullptr == m_pInstance)
            m_pInstance = new TemplateManager();  // 延迟初始化
        return m_pInstance;
    }

    // 从文件加载模板
    // path: 模板文件路径
    // template_name: 模板标识名（用于后续引用）
    void loadTemplates(std::string path, std::string template_name);

    // 使用Mustache渲染模板
    // template_name: 已加载的模板名
    // template_data: Mustache格式的数据对象
    // return: 渲染后的字符串结果
    std::string renderByTemplate(std::string template_name, Mustache::data& template_data);

private:
    // 私有构造函数（强制单例模式）
    TemplateManager() {}

    // 禁用拷贝构造函数
    TemplateManager(const TemplateManager&);

    // 禁用赋值运算符
    TemplateManager& operator=(const TemplateManager&);

    // 模板池（存储所有已加载的模板）
    // key: 模板名, value: 模板内容
    std::unordered_map<std::string, std::string> m_template_pool;
};