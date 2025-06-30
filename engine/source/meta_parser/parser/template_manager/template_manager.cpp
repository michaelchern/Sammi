
#include "template_manager.h"

// 加载模板文件到内存池
// path: 基础模板文件路径
// template_name: 模板标识名（用于后续引用）
void TemplateManager::loadTemplates(std::string path, std::string template_name)
{
    // 构造完整模板路径：基础路径 + 相对路径 + 模板名 + 扩展名
    std::string full_path = path + "/../template/" + template_name + ".mustache";

    // 插入或替换模板池中的内容
    // 若模板名已存在则更新，否则新增条目
    m_template_pool.insert_or_assign(template_name,                // 模板标识名
                                     Utils::loadFile(full_path));  // 加载模板文件内容
}

// 渲染指定模板
// template_name: 已加载的模板标识名
// template_data: Mustache格式的数据对象
// return: 渲染后的字符串结果
std::string TemplateManager::renderByTemplate(std::string template_name, Mustache::data& template_data)
{
    // 检查模板是否存在
    if (m_template_pool.end() == m_template_pool.find(template_name))
    {
        return "";  // 未找到模板时返回空字符串
    }

    // 获取模板内容
    std::string& template_content = m_template_pool[template_name];

    // 创建Mustache渲染器
    Mustache::mustache tmpl(template_content);

    // 渲染模板并返回结果
    return tmpl.render(template_data);
}