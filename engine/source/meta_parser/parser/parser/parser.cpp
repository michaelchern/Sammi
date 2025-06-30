#include "common/precompiled.h"
#include "language_types/class.h"
#include "generator/reflection_generator.h"  // 反射代码生成器
#include "generator/serializer_generator.h"  // 序列化代码生成器
#include "parser.h"

// 宏：递归处理命名空间
// 当遇到命名空间时，将其加入命名空间栈并递归处理子节点
#define RECURSE_NAMESPACES(kind, cursor, method, namespaces) \
    { \
        if (kind == CXCursor_Namespace) \
        { \
            auto display_name = cursor.getDisplayName(); \
            if (!display_name.empty()) \
            { \
                namespaces.emplace_back(display_name); \
                method(cursor, namespaces); \
                namespaces.pop_back(); \
            } \
        } \
    }

// 宏：尝试添加语言类型到对应模块
// 如果类型需要编译，则添加到所属模块的类型容器中，并记录类型名-文件映射
#define TRY_ADD_LANGUAGE_TYPE(handle, container) \
    { \
        if (handle->shouldCompile()) \
        { \
            auto file = handle->getSourceFile(); \
            m_schema_modules[file].container.emplace_back(handle); \
            m_type_table[handle->m_display_name] = file; \
        } \
    }

// 静态初始化（空实现）
void MetaParser::prepare(void) {}

// 获取类型所属的包含文件路径
std::string MetaParser::getIncludeFile(std::string name)
{
    auto iter = m_type_table.find(name);
    return iter == m_type_table.end() ? std::string() : iter->second;
}

// 构造函数
MetaParser::MetaParser(const std::string project_input_file,
                       const std::string include_file_path,
                       const std::string include_path,
                       const std::string sys_include,
                       const std::string module_name,
                       bool              is_show_errors)
                     : m_project_input_file(project_input_file),       // 项目输入文件
                       m_source_include_file_name(include_file_path),  // 包含文件路径
                       m_index(nullptr),                               // Clang索引初始化
                       m_translation_unit(nullptr),                    // 翻译单元初始化
                       m_sys_include(sys_include),                     // 系统包含路径
                       m_module_name(module_name),                     // 模块名称
                       m_is_show_errors(is_show_errors)                // 错误显示标志
{
    // 分割包含路径
    m_work_paths = Utils::split(include_path, ";");

    // 创建序列化生成器
    m_generators.emplace_back(new Generator::SerializerGenerator(m_work_paths[0], std::bind(&MetaParser::getIncludeFile, this, std::placeholders::_1)));

    // 创建反射生成器
    m_generators.emplace_back(new Generator::ReflectionGenerator(m_work_paths[0], std::bind(&MetaParser::getIncludeFile, this, std::placeholders::_1)));
}

// 析构函数（资源清理）
MetaParser::~MetaParser(void)
{
    // 释放所有生成器
    for (auto item : m_generators)
    {
        delete item;
    }
    m_generators.clear();

    // 释放Clang资源
    if (m_translation_unit)
        clang_disposeTranslationUnit(m_translation_unit);

    if (m_index)
        clang_disposeIndex(m_index);
}

// 完成处理（调用生成器的清理工作）
void MetaParser::finish(void)
{
    for (auto generator_iter : m_generators)
    {
        generator_iter->finish();
    }
}

// 解析项目文件
bool MetaParser::parseProject()
{
    bool result = true;
    std::cout << "Parsing project file: " << m_project_input_file << std::endl;

    // 打开项目文件
    std::fstream include_txt_file(m_project_input_file, std::ios::in);
    if (include_txt_file.fail())
    {
        std::cout << "Could not load file: " << m_project_input_file << std::endl;
        return false;
    }

    // 读取文件内容
    std::stringstream buffer;
    buffer << include_txt_file.rdbuf();
    std::string context = buffer.str();

    // 分割包含文件列表
    auto inlcude_files = Utils::split(context, ";");

    // 创建源包含文件
    std::fstream include_file;
    include_file.open(m_source_include_file_name, std::ios::out);
    if (!include_file.is_open())
    {
        std::cout << "Could not open the Source Include file: " << m_source_include_file_name << std::endl;
        return false;
    }

    std::cout << "Generating the Source Include file: " << m_source_include_file_name << std::endl;

    // 生成头文件保护符
    std::string output_filename = Utils::getFileName(m_source_include_file_name);
    if (output_filename.empty())
    {
        output_filename = "META_INPUT_HEADER_H";
    }
    else
    {
        Utils::replace(output_filename, ".", "_");
        Utils::replace(output_filename, " ", "_");
        Utils::toUpper(output_filename);
    }

    // 写入头文件保护
    include_file << "#ifndef __" << output_filename << "__" << std::endl;
    include_file << "#define __" << output_filename << "__" << std::endl;

    // 写入所有包含文件
    for (auto include_item : inlcude_files)
    {
        std::string temp_string(include_item);
        Utils::replace(temp_string, '\\', '/');
        include_file << "#include  \"" << temp_string << "\"" << std::endl;
    }

    // 结束头文件
    include_file << "#endif" << std::endl;
    include_file.close();
    return result;
}

// 主解析过程
int MetaParser::parse(void)
{
    // 解析项目文件
    bool parse_include_ = parseProject();
    if (!parse_include_)
    {
        std::cerr << "Parsing project file error! " << std::endl;
        return -1;
    }

    std::cerr << "Parsing the whole project..." << std::endl;

    // 创建Clang索引
    int is_show_errors = m_is_show_errors ? 1 : 0;
    m_index = clang_createIndex(true, is_show_errors);

    // 准备包含路径参数
    std::string pre_include = "-I";
    std::string sys_include_temp;
    if (!(m_sys_include == "*"))
    {
        sys_include_temp = pre_include + m_sys_include;
        arguments.emplace_back(sys_include_temp.c_str());
    }

    // 添加工作路径
    auto paths = m_work_paths;
    for (int index = 0; index < paths.size(); ++index)
    {
        paths[index] = pre_include + paths[index];
        arguments.emplace_back(paths[index].c_str());  // 添加工作包含路径
    }

    // 检查源包含文件是否存在
    fs::path input_path(m_source_include_file_name);
    if (!fs::exists(input_path))
    {
        std::cerr << input_path << " is not exist" << std::endl;
        return -2;
    }

    // 创建翻译单元（AST核心数据结构）
    m_translation_unit = clang_createTranslationUnitFromSourceFile(m_index,
                                                                   m_source_include_file_name.c_str(),
                                                                   static_cast<int>(arguments.size()),
                                                                   arguments.data(),
                                                                   0,
                                                                   nullptr);

    // 获取根游标
    auto cursor = clang_getTranslationUnitCursor(m_translation_unit);

    // 准备命名空间栈
    Namespace temp_namespace;

    // 构建类AST
    buildClassAST(cursor, temp_namespace);

    // 清理命名空间栈
    temp_namespace.clear();

    return 0;  // 成功
}

// 生成所有文件
void MetaParser::generateFiles(void)
{
    std::cerr << "Start generate runtime schemas(" << m_schema_modules.size() << ")..." << std::endl;

    // 遍历所有模块
    for (auto& schema : m_schema_modules)
    {
        // 调用每个生成器处理模块
        for (auto& generator_iter : m_generators)
        {
            generator_iter->generate(schema.first, schema.second);
        }
    }

    // 完成处理
    finish();
}

// 递归构建类AST
void MetaParser::buildClassAST(const Cursor& cursor, Namespace& current_namespace)
{
    // 遍历所有子节点
    for (auto& child : cursor.getChildren())
    {
        auto kind = child.getKind();

        // 处理类/结构体定义
        if (child.isDefinition() && (kind == CXCursor_ClassDecl || kind == CXCursor_StructDecl))
        {
            // 创建类对象
            auto class_ptr = std::make_shared<Class>(child, current_namespace);

            // 添加到模块
            TRY_ADD_LANGUAGE_TYPE(class_ptr, classes);
        }
        else
        {
            // 递归处理命名空间
            RECURSE_NAMESPACES(kind, child, buildClassAST, current_namespace);
        }
    }
}