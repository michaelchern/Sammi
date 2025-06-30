#include "common/precompiled.h"  // 预编译头文件
#include "parser/parser.h"       // 包含元数据解析器

// 解析函数声明
int parse(std::string project_file_name,
          std::string source_include_file_name,
          std::string include_path,
          std::string sys_include,
          std::string module_name,
          std::string show_errors);

// 程序主入口
int main(int argc, char* argv[])
{
    // 记录开始时间（用于性能统计）
    auto start_time = std::chrono::system_clock::now();
    int  result     = 0;  // 返回结果初始化

    // 检查命令行参数数量（需要6个参数）
    if (argv[1] != nullptr && argv[2] != nullptr && argv[3] != nullptr && argv[4] != nullptr && argv[5] != nullptr && argv[6] != nullptr)
    {
        // 元数据解析器准备工作（目前为空实现）
        MetaParser::prepare();

        // 调用解析函数
        result = parse(argv[1], argv[2], argv[3], argv[4], argv[5], argv[6]);

        // 计算并输出耗时
        auto duration_time = std::chrono::system_clock::now() - start_time;
        std::cout << "Completed in "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(duration_time).count()
                  << "ms" << std::endl;

        return result;
    }
    else
    {
        // 参数不足时显示错误信息和使用说明
        std::cerr << "Arguments parse error!" << std::endl
                  << "Please call the tool like this:" << std::endl
                  << "meta_parser  project_file_name  include_file_name_to_generate  project_base_directory "
                     "sys_include_directory module_name showErrors(0 or 1)"
                  << std::endl
                  << std::endl;
        return -1;  // 返回错误码
    }

    return 0;
}

// 实际解析函数
int parse(std::string project_input_file_name,
          std::string source_include_file_name,
          std::string include_path,
          std::string sys_include,
          std::string module_name,
          std::string show_errors)
{
    std::cout << std::endl;
    std::cout << "Parsing meta data for target \"" << module_name << "\"" << std::endl;
    std::fstream input_file;

    // 转换错误显示标志
    bool is_show_errors = "0" != show_errors;  // "1" -> true, "0" -> false

    // 创建元数据解析器实例
    MetaParser parser(project_input_file_name,   // 项目输入文件
                      source_include_file_name,  // 生成的包含文件名
                      include_path,              // 包含路径
                      sys_include,               // 系统包含路径
                      module_name,               // 模块名称
                      is_show_errors);           // 是否显示错误

    // 显示解析路径信息
    std::cout << "Parsing in " << include_path << std::endl;

    // 执行解析过程
    int result = parser.parse();
    if (0 != result)  // 检查解析结果
    {
        return result;  // 解析失败时返回错误码
    }

    // 生成代码文件
    parser.generateFiles();

    return 0;  // 成功返回
}