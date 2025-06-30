#include "common/precompiled.h"  // 预编译头文件
#include "meta_utils.h"          // 工具函数声明

static int parse_flag = 0;       // 静态解析标记（未使用）

namespace Utils
{
    // 转换CXString到std::string并释放资源
    void toString(const CXString& str, std::string& output)
    {
        auto cstr = clang_getCString(str);  // 获取C风格字符串
        output = cstr;                      // 赋值给输出
        clang_disposeString(str);           // 释放CXString资源
    }

    // 获取类型的全限定名（包含命名空间）
    std::string getQualifiedName(const CursorType& type)
    {
        return type.GetDisplayName();
    }

    // 获取不带命名空间的类型名
    std::string getTypeNameWithoutNamespace(const CursorType& type)
    {
        std::string&& type_name = type.GetDisplayName();  // 实际与全限定名相同（需改进）
        return type_name;
    }

    // 构造带命名空间的完整名称
    std::string getQualifiedName(const std::string& display_name, const Namespace& current_namespace)
    {
        std::string name = "";
        // 拼接命名空间
        for (auto& iter_string : current_namespace)
        {
            name += (iter_string + "::");
        }
        name += display_name;  // 添加类型名
        return name;
    }

    // 重载：从游标获取全限定名
    std::string getQualifiedName(const Cursor& cursor, const Namespace& current_namespace)
    {
        return getQualifiedName(cursor.getSpelling(), current_namespace);
    }

    // 格式化全限定名（替换特殊字符）
    std::string formatQualifiedName(std::string& source_string)
    {
        // 替换规则：
        Utils::replace(source_string, '<', 'L');  // < → L (左)
        Utils::replace(source_string, ':', 'S');  // : → S (作用域)
        Utils::replace(source_string, '>', 'R');  // > → R (右)
        Utils::replace(source_string, '*', 'P');  // * → P (指针)
        return source_string;
    }

    // 生成相对路径（自动处理'.'和'..'）
    fs::path makeRelativePath(const fs::path& from, const fs::path& to)
    {
        // 规范化输入路径（移除冗余部分）
        std::string form_complete_string;
        std::string to_complete_string;
        (void)formatPathString(from.string(), form_complete_string);
        (void)formatPathString(to.string(), to_complete_string);

        // 转换为路径对象
        fs::path form_complete = form_complete_string;
        fs::path to_complete   = to_complete_string;

        // 查找共同路径前缀
        auto iter_from = form_complete.begin();
        auto iter_to   = to_complete.begin();
        while (iter_from != form_complete.end() && iter_to != to_complete.end() && (*iter_to) == (*iter_from))
        {
            ++iter_to;
            ++iter_from;
        }

        // 构建相对路径
        fs::path final_path;
        // 回溯源路径差异部分
        while (iter_from != form_complete.end())
        {
            final_path /= "..";
            ++iter_from;
        }

        // 添加目标路径剩余部分
        while (iter_to != to_complete.end())
        {
            final_path /= *iter_to;
            ++iter_to;
        }

        return final_path;
    }

    // 报告致命错误并退出程序
    void fatalError(const std::string& error)
    {
        std::cerr << "Error: " << error << std::endl;
        exit(EXIT_FAILURE);  // 立即退出
    }

    // 分割字符串（按指定分隔符）
    std::vector<std::string> split(std::string input, std::string pat)
    {
        std::vector<std::string> ret_list;
        while (true)
        {
            size_t      index    = input.find(pat);         // 查找分隔符
            std::string sub_list = input.substr(0, index);  // 获取子串

            if (!sub_list.empty())
            {
                ret_list.push_back(sub_list);  // 非空部分加入结果
            }
            input.erase(0, index + pat.size());

            if (index == -1)
            {
                break;
            }
        }
        return ret_list;
    }

    // 从路径中提取文件名
    std::string getFileName(std::string path)
    {
        // 分割路径并取最后部分
        if (path.size() < 1)
        {
            return std::string();
        }

        std::vector<std::string> result = split(path, "/");

        if (result.size() < 1)
        {
            return std::string();
        }
        return result[result.size() - 1];
    }

    // 移除成员变量前缀 "m_" 
    std::string getNameWithoutFirstM(std::string& name)
    {
        std::string result = name;
        // 检查是否以"m_"开头
        if (name.size() > 2 && name[0] == 'm' && name[1] == '_')
        {
            result = name.substr(2);
        }
        return result;
    }

    // 提取容器模板中的类型
    std::string getNameWithoutContainer(std::string name)
    {
        // 查找模板的<和>位置
        size_t left  = name.find_first_of('<') + 1;
        size_t right = name.find_last_of('>');

        // 验证有效性并提取内容
        if (left > 0 && right < name.size() && left < right)
        {
            return name.substr(left, right - left);
        }
        else
        {
            return nullptr;
        }
    }

    // 移除字符串两端的引号
    std::string getStringWithoutQuot(std::string input)
    {
        size_t left  = input.find_first_of('\"') + 1;
        size_t right = input.find_last_of('\"');

        if (left > 0 && right < input.size() && left < right)
        {
            return input.substr(left, right - left);
        }
        else
        {
            return input;
        }
    }

    // 修剪字符串两端的空格
    std::string trim(const std::string& context)
    {

        std::string ret_string = context;
        // 移除左侧空格
        ret_string = ret_string.erase(0, ret_string.find_first_not_of(" "));
        // 移除右侧空格
        ret_string = ret_string.erase(ret_string.find_last_not_of(" ") + 1);
        return ret_string;
    }

    // 替换子字符串
    std::string replace(std::string& source_string, std::string sub_string, const std::string new_string)
    {
        std::string::size_type pos = 0;
        while ((pos = source_string.find(sub_string)) != std::string::npos)
        {
            source_string.replace(pos, sub_string.length(), new_string);
        }
        return source_string;
    }

    // 替换字符
    std::string replace(std::string& source_string, char taget_char, const char new_char)
    {
        std::replace(source_string.begin(), source_string.end(), taget_char, new_char);
        return source_string;
    }

    // 转换为大写
    std::string toUpper(std::string& source_string)
    {
        transform(source_string.begin(), source_string.end(), source_string.begin(), ::toupper);
        return source_string;
    }

    // 拼接字符串列表
    std::string join(std::vector<std::string> context_list, std::string separator)
    {
        std::string ret_string;
        if (context_list.size() == 0)
        {
            return ret_string;
        }
        ret_string = context_list[0];
        for (int index = 1; index < context_list.size(); ++index)
        {
            ret_string += separator + context_list[index];
        }

        return ret_string;
    }

    // 高级修剪（指定要移除的字符集）
    std::string trim(std::string& source_string, const std::string trim_chars)
    {
        // 找到第一个非移除字符
        size_t left_pos = source_string.find_first_not_of(trim_chars);
        if (left_pos == std::string::npos)
        {
            source_string = std::string();
        }
        else
        {
            // 找到最后一个非移除字符
            size_t right_pos = source_string.find_last_not_of(trim_chars);
            source_string    = source_string.substr(left_pos, right_pos - left_pos + 1);
        }
        return source_string;
    }

    // 加载文件内容到字符串
    std::string loadFile(std::string path)
    {
        std::ifstream      iFile(path);
        std::string        line_string;
        std::ostringstream template_stream;
        if (false == iFile.is_open())
        {
            iFile.close();
            return "";
        }
        while (std::getline(iFile, line_string))
        {
            template_stream << line_string << std::endl;
        }
        iFile.close();

        return template_stream.str();
    }

    // 保存字符串到文件（自动创建目录）
    void saveFile(const std::string& outpu_string, const std::string& output_file)
    {
        fs::path out_path(output_file);

        // 创建父目录（如果需要）
        if (!fs::exists(out_path.parent_path()))
        {
            fs::create_directories(out_path.parent_path());
        }
        // 写入文件
        std::fstream output_file_stream(output_file, std::ios_base::out);

        output_file_stream << outpu_string << std::endl;
        output_file_stream.flush();
        output_file_stream.close();
        return;
    }

    // 全局替换所有匹配项
    void replaceAll(std::string& resource_str, std::string sub_str, std::string new_str)
    {
        std::string::size_type pos = 0;
        while ((pos = resource_str.find(sub_str)) != std::string::npos)
        {
            resource_str = resource_str.replace(pos, sub_str.length(), new_str);
        }
        return;
    }

    // 规范化路径字符串（处理相对路径和特殊字符）
    unsigned long formatPathString(const std::string& path_string, std::string& out_string)
    {
        unsigned int ulRet             = 0;
        auto         local_path_string = path_string;
        fs::path     local_path;

        local_path = local_path_string;
        if (local_path.is_relative())
        {
            local_path_string = fs::current_path().string() + "/" + local_path_string;
        }

        replaceAll(local_path_string, "\\", "/");
        std::vector<std::string> subString = split(local_path_string, "/");
        std::vector<std::string> out_sub_string;
        for (auto p : subString)
        {
            if (p == "..")
            {
                out_sub_string.pop_back();
            }
            else if (p != ".")
            {
                out_sub_string.push_back(p);
            }
        }
        for (int i = 0; i < out_sub_string.size() - 1; i++)
        {
            out_string.append(out_sub_string[i] + "/");
        }
        out_string.append(out_sub_string[out_sub_string.size() - 1]);
        return 0;
    }

    // 转换为大驼峰命名法（UpperCamelCase）
    std::string convertNameToUpperCamelCase(const std::string& name, std::string pat)
    {
        std::string ret_string;
        auto&& name_spilts = split(name, pat);
        for (auto& split_string : name_spilts)
        {
            split_string[0] = toupper(split_string[0]);
            ret_string.append(split_string);
        }
        return ret_string;
    }
}